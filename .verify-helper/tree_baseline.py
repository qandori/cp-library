"""Optional direct-merge proof from successful source-push checks, never a cache."""

import ast
import json
import re
import urllib.error
import urllib.parse
import urllib.request

from affected import (GROUPS, SourceTree, canonical, digest, git, read_binding, require,
                      source_affected_plan, validate_binding)


POLICY = "direct-tree-source-push-or-exact-baseline-v2"
LOG_LIMIT = 16 * 1024 * 1024
MAX_ATTEMPTS = 10
FAILURES = {"failure", "cancelled", "timed_out", "action_required", "startup_failure", "stale"}
INTRINSIC_STEP = "Check exact-head native and explicit scalar intrinsic paths"
MANDATORY_CONTROLS = (
    ".github/workflows/verify.yml", ".github/workflows/macos-portability.yml",
    ".verify-helper/affected.py", ".verify-helper/affected_test.py",
    ".verify-helper/ci.py", ".verify-helper/ci_test.py",
    ".verify-helper/no_cache_route.py", ".verify-helper/no_cache_route_test.py",
    ".verify-helper/intrinsic_gate.py", ".verify-helper/intrinsic_gate_test.py",
    ".verify-helper/tree_baseline.py", ".verify-helper/tree_baseline_test.py",
    ".verify-helper/control_full.py", ".verify-helper/control_full_test.py",
)


class MainFailure(ValueError):
    pass


def failed(value):
    return isinstance(value, str) and value in FAILURES


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *args, **kwargs):
        return None


def job_log(job_id, token, *, api_open=None, public_open=None):
    """Keep credentials on api.github.com, including during signed-log redirects."""
    require(type(job_id) is int and job_id > 0 and bool(token), "Invalid log request")
    opener = urllib.request.build_opener(NoRedirect())
    api_open = api_open or opener.open
    public_open = public_open or opener.open
    request = urllib.request.Request(
        "https://api.github.com/repos/qandori/cp-library/actions/jobs/" + str(job_id) + "/logs",
        headers={"Authorization": "Bearer " + token, "Accept": "application/vnd.github+json",
                 "X-GitHub-Api-Version": "2022-11-28", "User-Agent": "cp-library-no-cache-ci"})
    try:
        response = api_open(request, timeout=30)
    except urllib.error.HTTPError as error:
        if error.code != 302:
            raise ValueError("Actions log download unavailable") from None
        target = urllib.parse.urlsplit(error.headers.get("Location", ""))
        error.close()
        require(target.scheme == "https" and target.username is None and target.password is None
                and target.port is None and not target.fragment
                and re.fullmatch(r"productionresultssa[0-9]+\.blob\.core\.windows\.net", target.hostname or "")
                and target.path.startswith("/actions-results/"), "Unrecognized Actions log destination")
        # A new request has no authorization header; this opener also rejects redirects.
        response = public_open(urllib.request.Request(target.geturl()), timeout=30)
    with response:
        data = response.read(LOG_LIMIT + 1)
    require(len(data) <= LOG_LIMIT, "Actions log exceeds the reviewed limit")
    return data.decode("utf-8-sig", errors="strict")


def complete_jobs(payload):
    require(isinstance(payload, dict), "Malformed Actions job response")
    jobs = payload.get("jobs")
    require(isinstance(jobs, list) and type(payload.get("total_count")) is int
            and payload["total_count"] == len(jobs), "Incomplete Actions job list")
    require(all(isinstance(job, dict) and type(job.get("id")) is int and job["id"] > 0 for job in jobs)
            and len({job["id"] for job in jobs}) == len(jobs), "Duplicate or invalid job identities")
    return jobs


def run_identity(run, number, attempt, head, branch, workflow=None):
    require(isinstance(run, dict) and isinstance(run.get("repository"), dict)
            and isinstance(run.get("head_repository"), dict), "Malformed run identity")
    require(run.get("id") == number and run.get("run_attempt") == attempt
            and run.get("head_sha") == head and run.get("head_branch") == branch
            and run.get("repository", {}).get("full_name") == "qandori/cp-library"
            and run.get("head_repository", {}).get("full_name") == "qandori/cp-library"
            and (workflow is None or run.get("path") == workflow), "Run identity differs")


def check_main(root, base, token, request):
    prefix = "/repos/qandori/cp-library/actions"
    payload = request(prefix + "/runs?head_sha=" + base + "&branch=main&per_page=100", token)
    require(isinstance(payload, dict), "Malformed main run response")
    runs = payload.get("workflow_runs")
    require(isinstance(runs, list), "Incomplete main run list")
    for run in runs:
        if (isinstance(run, dict) and run.get("head_sha") == base and run.get("head_branch") == "main"
                and run.get("event") in ("push", "workflow_dispatch") and failed(run.get("conclusion"))):
            raise MainFailure("Actual-main CI has an unsuccessful run")
    require(type(payload.get("total_count")) is int and payload["total_count"] == len(runs), "Incomplete main run list")
    require(all(isinstance(run, dict) and type(run.get("id")) is int and run["id"] > 0 for run in runs)
            and len({run["id"] for run in runs}) == len(runs), "Duplicate or invalid main runs")
    # Notice an already reported failure before requesting additional evidence.
    actual = [run for run in runs if run.get("event") in ("push", "workflow_dispatch")]
    for run in actual:
        require(run.get("head_sha") == base and run.get("head_branch") == "main", "Main query identity differs")
        if failed(run.get("conclusion")):
            raise MainFailure("Actual-main CI has an unsuccessful run")
    require({".github/workflows/verify.yml", ".github/workflows/macos-portability.yml"}
            <= {run.get("path") for run in actual}, "Required actual-main workflows are not visible")
    for run in actual:
        number, attempts = run.get("id"), run.get("run_attempt")
        require(type(number) is int and number > 0 and type(attempts) is int
                and 0 < attempts <= MAX_ATTEMPTS, "Unsupported main attempt history")
        for attempt in range(1, attempts + 1):
            path = prefix + "/runs/" + str(number) + "/attempts/" + str(attempt)
            receipt = request(path, token)
            run_identity(receipt, number, attempt, base, "main")
            require(receipt.get("event") in ("push", "workflow_dispatch"), "Unexpected main event")
            if failed(receipt.get("conclusion")):
                raise MainFailure("Actual-main CI has an unsuccessful attempt")
            payload = request(path + "/jobs?per_page=100", token)
            require(isinstance(payload, dict), "Malformed main jobs response")
            jobs = payload.get("jobs", [])
            if isinstance(jobs, list):
                for job in jobs:
                    if not isinstance(job, dict):
                        continue
                    steps = job.get("steps")
                    steps = steps if isinstance(steps, list) else []
                    if failed(job.get("conclusion")) or any(
                            isinstance(step, dict) and failed(step.get("conclusion")) for step in steps):
                        raise MainFailure("Actual-main CI has unsuccessful execution")
            complete_jobs(payload)
    return [{"id": run["id"], "attempt": run["run_attempt"], "status": run.get("status")}
            for run in actual]


def coverage(plan):
    return {group: {"targets": plan[group], "fingerprints": {
        target: plan["state"]["targets"][group][target] for target in plan[group]}}
        for group in GROUPS}


def literal(source, name):
    matches = [node.value for node in ast.parse(source).body if isinstance(node, ast.Assign)
               and any(isinstance(target, ast.Name) and target.id == name for target in node.targets)]
    require(len(matches) == 1, "Ambiguous historical source constant")
    return ast.literal_eval(matches[0])


def historical_controls(tree):
    source = tree.content(".verify-helper/no_cache_route.py")
    controls = literal(source, "CONTROL_PATHS")
    additions = [node.value for node in ast.parse(source).body if isinstance(node, ast.Assign)
                 and any(isinstance(target, ast.Name) and target.id == "CONTROL_PATHS_V2" for target in node.targets)]
    require(len(additions) == 1, "Ambiguous historical v2 control inventory")
    value = additions[0]
    require(isinstance(value, ast.BinOp) and isinstance(value.op, ast.Add)
            and isinstance(value.left, ast.Name) and value.left.id == "CONTROL_PATHS",
            "Unsupported historical v2 control inventory")
    extra = ast.literal_eval(value.right)
    require(isinstance(controls, tuple) and isinstance(extra, tuple), "Invalid historical control inventory")
    paths = controls + extra
    require(all(isinstance(path, str) and re.fullmatch(r"[A-Za-z0-9_.-]+(?:/[A-Za-z0-9_.-]+)*", path)
                and all(part not in (".", "..") for part in path.split("/")) for path in paths)
            and len(set(paths)) == len(paths) and set(MANDATORY_CONTROLS) <= set(paths),
            "Invalid or incomplete historical control inventory")
    require(all(tree.entries.get(path, ())[:2] in (("100644", "blob"), ("100755", "blob")) for path in paths),
            "Historical controls must be present regular files")
    return paths


def source_plans(root, base, proof):
    import no_cache_route as route
    pair = route.parents(root, base)
    require(len(pair) == 2, "Baseline is not a direct two-parent merge")
    previous, source = pair
    git(root, "merge-base", "--is-ancestor", previous, source)
    require(route.same_tree(root, base, source), "Baseline tree differs from source")
    old, tree, merged = SourceTree(root, previous), SourceTree(root, source), SourceTree(root, base)
    binding = read_binding(tree)
    require(binding["base"] == previous, "Baseline parent order or binding differs")
    require(read_binding(merged) == binding, "Baseline merge binding differs")
    ref = literal(tree.content(".verify-helper/no_cache_route.py"), "SOURCE_REF")
    require(ref == binding["ref"], "Historical source branch differs")
    for commit in (source, base):
        message = git(root, "show", "-s", "--format=%B", commit).decode().rstrip()
        require(message and message.splitlines()[-1] == route.TRAILER, "Historical explicit route is absent")
    validate_binding(old, tree, ref)
    validate_binding(old, merged, ref)
    require(proof["reviewed_source_control_sha256"] == route.source_hash(tree, historical_controls(tree)),
            "Historical source controls differ")
    source_plan = source_affected_plan(root, previous, source, expected_ref=ref)
    merge_plan = source_affected_plan(root, previous, base, expected_ref=ref)
    require(coverage(source_plan) == coverage(merge_plan), "Source/merge targets or fingerprints differ")
    require(source_plan["scope_reasons"] == merge_plan["scope_reasons"], "Source/merge scope reasons differ")
    return source_plan, merge_plan


def log_lines(text):
    require(isinstance(text, str) and len(text.encode()) <= LOG_LIMIT, "Invalid Actions log")
    lines = []
    for line in text.removeprefix("\ufeff").splitlines():
        match = re.fullmatch(r"\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d\.\d+Z (.*)", line)
        require(match is not None, "Unrecognized Actions log format")
        lines.append(match[1])
    return lines


def records(lines):
    return [json.loads(line) for line in lines if line.startswith("{")]


def unique(items, predicate, message):
    matches = [item for item in items if predicate(item)]
    require(len(matches) == 1, message)
    return matches[0]


def selected_log(lines, plan, groups):
    items = records(lines)
    logged = unique(items, lambda item: item.get("mode") == "affected-no-cache-v1" and "head_tree" in item,
                    "Missing or ambiguous source plan")
    require(logged == {key: value for key, value in plan.items() if key != "state"}, "Logged source plan differs")
    completed = [item for item in items if item.get("event") == "selected_checks_completed"]
    seen = set()
    for item in completed:
        require(item.get("head") == plan["head"] and item.get("plan_sha256") == plan["plan_sha256"]
                and item.get("mode") == "affected-no-cache-v1" and item.get("unselected_checks") == "not_run"
                and item.get("successful_cache_written") is False and isinstance(item.get("groups"), dict),
                "Source completion provenance differs")
        for group, targets in item["groups"].items():
            require(group in groups and group not in seen and targets == plan[group], "Source completed targets differ")
            seen.add(group)
    require(seen == set(groups), "Source completed groups are incomplete")
    route = unique(items, lambda item: item.get("mode") == "no-cache" and item.get("event") == "push",
                   "Missing or ambiguous source route")
    require(route == {"mode": "no-cache", "base": plan["base"], "head": plan["head"],
                     "source_head": plan["head"], "ref": plan["expected_ref"], "event": "push",
                     "source_digest": plan["binding"]["source_digest"]}, "Logged source route differs")


def runtime(lines, job_name):
    if job_name == "verify":
        require("Image: ubuntu-24.04" in lines, "Linux runner evidence differs")
        uname = unique(lines, lambda line: line.startswith("Linux ") and " x86_64 " in line,
                       "Missing actual Linux platform evidence")
        gcc = unique(lines, lambda line: re.match(r"g\+\+ \(.*\) \d+\.\d+", line), "Missing actual GCC version")
        clang = unique(lines, lambda line: re.match(r"(?:Ubuntu |Debian )?clang version \d+\.", line),
                       "Missing actual Linux Clang version")
        target = unique(lines, lambda line: re.fullmatch(r"Target: x86_64-[a-z0-9_-]*linux[a-z0-9_-]*", line),
                        "Missing actual Linux Clang target")
        gcc_target = unique(lines, lambda line: re.fullmatch(r"x86_64-[a-z0-9_-]*linux[a-z0-9_-]*", line),
                            "Missing actual GCC target")
        return {"platform": uname, "gcc": gcc, "gcc_target": gcc_target, "clang": clang, "clang_target": target}
    require("Image: macos-14-arm64" in lines or "Image: macos-14" in lines, "Mac runner evidence differs")
    uname = unique(lines, lambda line: line.startswith("Darwin ") and line.endswith(" arm64"),
                   "Missing actual Mac platform evidence")
    clang = unique(lines, lambda line: re.match(r"Apple clang version \d+\.", line), "Missing Apple Clang version")
    target = unique(lines, lambda line: re.fullmatch(r"Target: arm64-apple-darwin[0-9.]+", line),
                    "Missing actual Apple Clang target")
    return {"platform": uname, "clang": clang, "clang_target": target}


def intrinsic_log(lines, plan, tree):
    import intrinsic_gate as gate
    require("Image: ubuntu-26.04" in lines, "Intrinsic runner evidence differs")
    items = records(lines)
    family = gate.scope_mode(plan)
    modes = gate.family_modes(family)
    expected_route = {"mode": "no-cache", "base": plan["base"], "head": plan["head"],
                      "source_head": plan["head"], "ref": plan["expected_ref"], "event": "push",
                      "source_digest": plan["binding"]["source_digest"]}
    scope = unique(items, lambda item: item.get("event") == "scope", "Missing intrinsic scope")
    require(scope.get("route") == expected_route and scope.get("family") == family and scope.get("headers") == plan["headers"]
            and scope.get("units") == plan["unit"] and scope.get("plan_sha256") == plan["plan_sha256"],
            "Intrinsic scope differs")
    require(scope.get("source_controls") == {name: digest(tree.content(name)) for name in (
        ".verify-helper/intrinsic_gate.py", ".verify-helper/intrinsic_gate_test.py", ".github/workflows/verify.yml")},
        "Intrinsic source controls differ")
    compiler = unique(items, lambda item: item.get("event") == "compiler", "Missing intrinsic compiler proof")
    require(compiler.get("assertions") is True and compiler.get("lto") is False
            and compiler.get("compiler", {}).get("path") == "/usr/bin/x86_64-linux-gnu-g++-15"
            and re.fullmatch("[0-9a-f]{64}", compiler.get("compiler", {}).get("sha256", ""))
            and re.fullmatch("[0-9a-f]{64}", compiler.get("cc1plus", {}).get("sha256", "")),
            "Intrinsic compiler proof differs")
    starts = [item for item in items if item.get("event") == "command_start"]
    ends = [item for item in items if item.get("event") == "command_end"]
    require([item.get("id") for item in starts] == list(range(1, len(starts) + 1))
            and [item.get("id") for item in ends] == list(range(1, len(starts) + 1))
            and all(item.get("returncode") == 0 and item.get("timed_out") is False for item in ends),
            "Intrinsic command execution is incomplete")
    require(starts and starts[0].get("argv") == [compiler["compiler"]["path"], "-dumpfullversion"]
            and ends[0].get("output_tail") == "15.2.0\n", "Intrinsic GCC version differs")
    require(len(starts) == 2 + len(modes) * (1 + 2 * (len(plan["headers"]) + len(plan["unit"]))),
            "Intrinsic command count differs from executed scope")
    binaries = [item for item in items if item.get("event") == "binary"]
    expected = {(mode, group, target) for mode in modes for group in ("headers", "unit") for target in plan[group]}
    require(len(binaries) == len(expected) and {(item.get("mode"), item.get("group"), item.get("target"))
            for item in binaries} == expected, "Intrinsic binary execution scope differs")
    for item in binaries:
        require(re.fullmatch("[0-9a-f]{64}", item.get("binary", {}).get("sha256", "")),
                "Missing intrinsic binary identity")
        for dependency in item.get("dependencies", {}).get("project_dependencies", []):
            name = dependency.get("project_path")
            require(isinstance(name, str) and name in tree.entries
                    and dependency.get("sha256") == digest(tree.content(name)), "Intrinsic dependency bytes differ")
    for mode in modes:
        macro = unique(items, lambda item: item.get("event") == "macro_proof" and item.get("mode") == mode,
                       "Missing intrinsic macro proof")
        require(macro.get("ONLINE_JUDGE") == "1" and macro.get("ATCODER") == "1"
                and isinstance(macro.get("isa"), dict)
                and (macro["isa"].get("__AVX2__") == "1" and macro["isa"].get("__BMI2__") == "1"
                     if mode == "native" else not macro["isa"]), "Intrinsic ISA/assertion evidence differs")
        require(macro.get("gcc") == "15.2.0" and macro.get("family") == family
                and macro.get("assertions") is True and macro.get("flags") == gate.mode_flags(mode, family),
                "Intrinsic macro/flag proof differs")
        done = unique(items, lambda item: item.get("event") == "mode_completed" and item.get("mode") == mode,
                      "Missing intrinsic mode completion")
        require(done.get("header_count") == len(plan["headers"]) and done.get("unit_count") == len(plan["unit"]),
                "Intrinsic completed targets differ")
    done = unique(items, lambda item: item.get("event") == "gate_completed", "Missing intrinsic completion")
    require(done.get("head") == plan["head"] and done.get("source_head") == plan["head"]
            and done.get("base") == plan["base"] and done.get("source_digest") == plan["binding"]["source_digest"]
            and done.get("plan_sha256") == plan["plan_sha256"] and done.get("commands") == len(starts)
            and done.get("unaffected_checks") == "not_run" and done.get("cache_written") is False
            and done.get("artifacts_uploaded") is False, "Intrinsic completion provenance differs")
    return {"gcc": "15.2.0", "modes": list(modes), "compiler_sha256": compiler["compiler"]["sha256"]}


def qualify(root, route, proof, token, request, read_log, baseline_jobs):
    plan, merged = source_plans(root, route["base"], proof)
    check_main(root, route["base"], token, request)
    receipts = []
    for workflow, identity in proof["runs"].items():
        number, attempt = identity["id"], identity["attempt"]
        prefix = "/repos/qandori/cp-library/actions/runs/" + str(number)
        branch = plan["expected_ref"].removeprefix("refs/heads/")
        latest = request(prefix, token)
        path = prefix + "/attempts/" + str(attempt)
        for run in (latest, request(path, token)):
            run_identity(run, number, attempt, plan["head"], branch, workflow)
            require(run.get("event") == "push" and run.get("status") == "completed"
                    and run.get("conclusion") == "success", "Source push attempt is not successful")
        jobs = complete_jobs(request(path + "/jobs?per_page=100", token))
        require(all(job.get("status") == "completed" and job.get("conclusion") in ("success", "skipped")
                    for job in jobs), "Source jobs are unsuccessful")
        job_name, steps = baseline_jobs[workflow]
        setup = ("Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event")
        required = {job_name: setup + steps + (("Install dependencies without cache", "Install Boost")
                                             if job_name == "verify" else ())}
        if job_name == "verify":
            required["intrinsic"] = ("Set up job", "Run actions/checkout@v4", INTRINSIC_STEP)
        for name, required_steps in required.items():
            job = unique(jobs, lambda item: item.get("name") == name, "Missing required source job")
            require(job.get("run_id") == number and job.get("run_attempt") == attempt
                    and job.get("head_sha") == plan["head"] and job.get("head_branch") == branch
                    and job.get("status") == "completed" and job.get("conclusion") == "success",
                    "Required source job did not execute successfully")
            for step_name in required_steps:
                step = unique(job.get("steps", []), lambda item: item.get("name") == step_name,
                              "Missing required executed source step")
                require(step.get("status") == "completed" and step.get("conclusion") == "success",
                        "Required source step did not execute successfully")
            lines = log_lines(read_log(job["id"], token))
            if name == "intrinsic":
                observed = intrinsic_log(lines, plan, SourceTree(root, plan["head"]))
            else:
                selected_log(lines, plan, GROUPS if name == "verify" else ("headers", "unit"))
                observed = runtime(lines, name)
            receipts.append({"workflow": workflow, **identity, "head": plan["head"], "job_id": job["id"],
                             "job": name, "observed_runtime": observed})
    # Recheck failures after potentially slow source log reads.
    main_runs = check_main(root, route["base"], token, request)
    return {"policy": POLICY, "basis": "source-push-identical-tree", "receipts": receipts,
            "equivalent_main": route["base"], "tree": merged["head_tree"],
            "selected_coverage_sha256": digest(canonical(coverage(plan))), "observed_main_runs": main_runs,
            "main_checks": "not_claimed_by_source_receipt", "full_baseline_target_coverage": "not_independently_established",
            "unaffected_current_head_checks": "not_run"}


def bootstrap(root, route, evidence, token, request, read_log, baseline_jobs):
    import control_full as control
    import no_cache_route as routing
    base = route["base"]
    pair = routing.parents(root, base)
    require(len(pair) == 2 and pair[0] == control.BASE and routing.same_tree(root, base, pair[1])
            and control.marked(root, base) and control.marked(root, pair[1]), "Wrong control-full bootstrap merge")
    tree = control.check_changes(root, base)
    require(literal(tree.content(".verify-helper/control_full.py"), "BASE") == control.BASE
            and literal(tree.content(".verify-helper/control_full.py"), "REF") == control.REF,
            "Unsupported control-full bootstrap source")
    require(evidence["qualification"]["reviewed_source_control_sha256"] == routing.source_hash(tree, historical_controls(tree)),
            "Bootstrap control source differs")
    check_main(root, base, token, request)
    expected = {**tree.targets(), "full": True, "reason": "Full verification requested", "mode": control.MODE,
                "head": base, "head_tree": git(root, "rev-parse", base + "^{tree}").decode().strip(),
                "source_coverage_sha256": digest(canonical(control.source_coverage(tree))),
                "cache_reads": False, "cache_writes": False}
    receipts = []
    for workflow, identity in evidence["runs"].items():
        number, attempt = identity["id"], identity["attempt"]
        prefix = "/repos/qandori/cp-library/actions/runs/" + str(number)
        path = prefix + "/attempts/" + str(attempt)
        for run in (request(prefix, token), request(path, token)):
            run_identity(run, number, attempt, base, "main", workflow)
            require(run.get("event") == "push" and run.get("status") == "completed"
                    and run.get("conclusion") == "success", "Bootstrap main attempt is not successful")
        jobs = complete_jobs(request(path + "/jobs?per_page=100", token))
        require(all(job.get("status") == "completed" and job.get("conclusion") in ("success", "skipped")
                    for job in jobs), "Bootstrap jobs are unsuccessful")
        name, steps = baseline_jobs[workflow]
        job = unique(jobs, lambda item: item.get("name") == name, "Missing bootstrap job")
        require(job.get("run_id") == number and job.get("run_attempt") == attempt and job.get("head_sha") == base
                and job.get("head_branch") == "main" and job.get("conclusion") == "success", "Wrong bootstrap job")
        steps = tuple("Select exact-head full scope without cache" if step == "Select exact-head affected scope without cache"
                      else step for step in steps)
        steps += ("Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event")
        if name == "verify":
            steps += ("Install dependencies without cache", "Install Boost")
            intrinsic = unique(jobs, lambda item: item.get("name") == "intrinsic", "Missing bootstrap intrinsic disposition")
            require(intrinsic.get("conclusion") == "skipped" and not intrinsic.get("steps"),
                    "Control bootstrap must not claim intrinsic execution")
        for step_name in steps:
            step = unique(job.get("steps", []), lambda item: item.get("name") == step_name, "Missing bootstrap step")
            require(step.get("status") == "completed" and step.get("conclusion") == "success", "Bootstrap step was not executed")
        lines = log_lines(read_log(job["id"], token))
        items = records(lines)
        plan = unique(items, lambda item: item.get("mode") == control.MODE and "full" in item, "Missing bootstrap full plan")
        require(plan == expected, "Bootstrap did not select the complete exact-source inventory")
        groups = GROUPS if name == "verify" else ("headers", "unit")
        seen = set()
        for item in (item for item in items if item.get("event") == "control_full_checks_completed"):
            require(all(item.get(key) == expected[key] for key in ("mode", "head", "head_tree", "source_coverage_sha256"))
                    and item.get("cache_written") is False and item.get("intrinsic_checks") == "not_run_control_only",
                    "Bootstrap completion provenance differs")
            for group, targets in item.get("groups", {}).items():
                require(group in groups and group not in seen and targets == expected[group], "Bootstrap completed scope differs")
                seen.add(group)
        require(seen == set(groups), "Bootstrap completed groups are incomplete")
        receipts.append({"workflow": workflow, **identity, "head": base, "job_id": job["id"],
                         "observed_runtime": runtime(lines, name)})
    check_main(root, base, token, request)
    return {"policy": POLICY, "basis": "control-full-bootstrap", "receipts": receipts,
            "ordinary_full_workflows": "executed", "intrinsic_checks": "not_run_control_only",
            "full_baseline_target_coverage": "not_independently_established", "unaffected_current_head_checks": "not_run"}
