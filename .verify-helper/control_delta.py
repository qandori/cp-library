"""Bounded PR24 execution receipts, not a successful-result cache."""

import json
import os
import re

from affected import GROUPS, SourceTree, canonical, checked_source, digest, git, require
import check_context
import control_full
import tree_baseline as proof


VERSION = 1
ANCHOR_REF = "refs/heads/codex/ci-tree-anchor-20261018"
REF = "refs/heads/codex/ci-tree-reuse-20261018"
ANCHOR_BASE = "3dd3db7c749ba3c00d432fc82b085146f86c4b0e"
ANCHOR_MARKER = "Verify-Mode: control-anchor"
MARKER = "Verify-Mode: control-delta"
ANCHOR_MODE = "control-anchor-no-cache-v1"
MODE = "control-delta-no-cache-v1"
INPUT = ".verify-helper/control-receipts.json"
RECEIPT_CONTROLS = (INPUT, ".verify-helper/control_delta.py", ".verify-helper/control_delta_test.py",
                    ".verify-helper/check_context.py", ".verify-helper/check_context_test.py")
# Reviewed upstream test-only update: expect the added wavelet-matrix unit.
# This is not check-profile equivalence; changed helper bytes still run fresh.
REVIEWED_UPSTREAM_TEST = {
    ".verify-helper/ci_test.py": (
        ("100644", "blob", "26f4fd4e3496dd37e7c87be47269856e4095c43c"),
        ("100644", "blob", "95e0567599e15b021061b311b75764310a6f3b1d")),
}
MAX_RECEIPTS = 8
LOG_LINE_LIMIT = 8192
TARGET_CHUNK = 24
SELECTOR = "Select exact-head control evidence scope without cache"
PLATFORMS = {".github/workflows/verify.yml": ("linux", "verify", GROUPS),
             ".github/workflows/macos-portability.yml": ("mac", "portable", ("headers", "unit"))}
CODE = (".verify-helper/affected.py", ".verify-helper/ci.py", ".verify-helper/no_cache_route.py",
        ".verify-helper/control_full.py", ".verify-helper/control_delta.py",
        ".verify-helper/check_context.py", ".verify-helper/tree_baseline.py")
SETTINGS = {
    "headers": ("test/verify.py",), "unit": ("test/verify.py",), "sanitize": ("test/verify.py",),
    "verification": (".verify-helper/config.toml", "test/requirements.txt", ".verify-helper/verification.py"),
    "bundles": (".verify-helper/check-docs.py", ".verify-helper/documentation.py", ".verify-helper/verification.py",
                ".verify-helper/config.toml", "test/requirements.txt"),
}


def marked(root, head, marker):
    message = git(root, "show", "-s", "--format=%B", head).decode().rstrip()
    return bool(message) and message.splitlines()[-1] == marker


def changes(root, base, head):
    original, old, new = SourceTree(root, ANCHOR_BASE), SourceTree(root, base), SourceTree(root, head)
    upstream_controls = {p for p in original.entries.keys() | old.entries.keys()
                         if p.startswith((".github/workflows/", ".verify-helper/"))
                         or p in ("test/verify.py", "test/requirements.txt")}
    require(all(original.entries.get(p) == old.entries.get(p)
                or (original.entries.get(p), old.entries.get(p)) == REVIEWED_UPSTREAM_TEST.get(p)
                for p in upstream_controls),
            "Upstream CI/helper inputs changed; renewed review required")
    changed = {path for path in old.entries.keys() | new.entries.keys()
               if old.entries.get(path) != new.entries.get(path)}
    require(bool(changed) and changed <= control_full.PATHS, "Control evidence changed non-control source")
    git(root, "merge-base", "--is-ancestor", ANCHOR_BASE, base)
    for name in ("verify.yml", "macos-portability.yml"):
        path = ".github/workflows/" + name
        source = original.content(path).decode()
        require(new.content(path) == control_full.expected_workflow(source, name).encode(),
                "Control evidence changed unapproved workflow execution")
    return new


def select_event(root, event, context, *, clean=True):
    import no_cache_route as route
    head, source, source_ref = route.event_source(root, event, context)
    kind, ref = context.get("GITHUB_EVENT_NAME"), context.get("GITHUB_REF")
    require(context.get("GITHUB_REPOSITORY") == route.REPOSITORY, "Wrong control evidence repository")
    anchor = source_ref == ANCHOR_REF or marked(root, source, ANCHOR_MARKER)
    if anchor:
        require(kind == "push" and ref == ANCHOR_REF and source == head
                and marked(root, source, ANCHOR_MARKER), "Anchor accepts only its dedicated source push")
        base, mode = ANCHOR_BASE, "control-anchor"
    else:
        if kind == "push" and ref == route.MAIN_REF:
            pair = route.parents(root, head)
            require(len(pair) == 2 and marked(root, head, MARKER), "Delta main requires a marked direct merge")
            base, source = pair
            require(event.get("before") == base and route.same_tree(root, head, source), "Delta main tree/base differs")
        else:
            pair = route.parents(root, source)
            require(len(pair) == 2, "Delta source requires its integration parents")
            base = pair[1]
        require(len(route.parents(root, source)) == 2 and route.parents(root, source)[1] == base
                and marked(root, source, MARKER), "Delta source lacks its exact integration base/marker")
        if kind == "pull_request":
            pull = event.get("pull_request", {})
            require(source_ref == REF and pull.get("head", {}).get("ref") == REF.removeprefix("refs/heads/")
                    and pull.get("head", {}).get("repo", {}).get("full_name") == route.REPOSITORY
                    and pull.get("base", {}).get("sha") == base and pull.get("base", {}).get("ref") == "main"
                    and pull.get("base", {}).get("repo", {}).get("full_name") == route.REPOSITORY,
                    "Delta PR integration base/source differs")
            require(type(event.get("number")) is int and ref == "refs/pull/" + str(event["number"]) + "/merge"
                    and route.parents(root, head) == [base, source] and route.same_tree(root, head, source),
                    "Delta PR merge differs")
        else:
            require(kind == "push" and ref in (REF, route.MAIN_REF), "Wrong delta event/ref")
        mode = "control-delta"
    if kind == "push":
        require(event.get("ref") == ref and event.get("after") == head and event.get("deleted") is False,
                "Control evidence push differs from checkout")
    if clean:
        checked_source(root, base, head)
    else:
        git(root, "merge-base", "--is-ancestor", base, head)
    changes(root, base, head)
    return {"mode": mode, "base": base, "head": head, "source_head": source,
            "ref": ANCHOR_REF if anchor else REF, "event": kind}


def inputs(tree):
    raw = tree.content(INPUT)
    require(raw is not None, "Missing explicit receipt inputs")
    value = json.loads(raw)
    require(raw == canonical(value) + b"\n" and isinstance(value, dict) and set(value) in ({"version", "receipts"}, {"version", "receipts", "fresh_groups"})
            and type(value["version"]) is int and value["version"] == VERSION, "Invalid receipt inputs")
    fresh = value.get("fresh_groups", [])
    require(isinstance(fresh, list) and all(isinstance(g, str) and g in GROUPS for g in fresh)
            and fresh == sorted(set(fresh)), "Invalid explicitly fresh groups")
    rows = value["receipts"]
    require(isinstance(rows, list) and len(rows) <= MAX_RECEIPTS, "Receipt input bound exceeded")
    seen = set()
    for row in rows:
        require(isinstance(row, dict) and set(row) == {"kind", "head", "workflow", "id", "attempt"}
                and row["kind"] in ("anchor", "delta") and isinstance(row["workflow"], str) and row["workflow"] in PLATFORMS
                and isinstance(row["head"], str) and re.fullmatch("[0-9a-f]{40}", row["head"])
                and type(row["id"]) is int and row["id"] > 0
                and type(row["attempt"]) is int and 0 < row["attempt"] <= proof.MAX_ATTEMPTS,
                "Invalid direct receipt input")
        identity = (row["id"], row["attempt"])
        require(identity not in seen, "Duplicate direct receipt input")
        seen.add(identity)
    return rows


def group_checks(tree, workflow):
    # Entire trusted helper code and workflow bind selection/check dispatch too.
    return {group: digest(canonical([[path, tree.entries.get(path), digest(tree.content(path))]
                                     for path in (*CODE, workflow, *SETTINGS[group])])) for group in GROUPS}


def runtime_valid(value):
    require(isinstance(value, dict) and set(value) == {"version", "profile", "groups", "ambient", "system_packages_sha256"}
            and type(value.get("version")) is int and value.get("version") == 1
            and value.get("profile") == check_context.PROFILE and isinstance(value.get("groups"), dict)
            and set(value["groups"]) == set(GROUPS),
            "Missing complete runtime profile")
    require(all(isinstance(value["groups"][g], dict) and set(value["groups"][g]) == {"runtime_sha256"}
                and isinstance(value["groups"][g]["runtime_sha256"], str)
                and re.fullmatch("[0-9a-f]{64}", value["groups"][g]["runtime_sha256"]) for g in GROUPS),
            "Invalid runtime context hashes")
    require(isinstance(value.get("ambient"), dict) and set(value["ambient"]) == set(check_context.AMBIENT_KEYS), "Missing ambient context presence")
    for entry in value["ambient"].values():
        require(isinstance(entry, dict) and set(entry) == {"present", "sha256"} and type(entry["present"]) is bool
                and ((entry["present"] and isinstance(entry["sha256"], str) and re.fullmatch("[0-9a-f]{64}", entry["sha256"]))
                     or (not entry["present"] and entry["sha256"] is None)), "Invalid hashed ambient context")
    require(isinstance(value.get("system_packages_sha256"), str)
            and re.fullmatch("[0-9a-f]{64}", value["system_packages_sha256"]), "Missing package identity")


def make_plan(root, route, workflow, runtime, donors, *, producer=None):
    runtime_valid(runtime)
    tree = SourceTree(root, route["head"])
    input_rows = inputs(tree)
    fresh = json.loads(tree.content(INPUT)).get("fresh_groups", [])
    targets, sources = tree.targets(), control_full.source_coverage(tree)
    checks = group_checks(tree, workflow)
    platform, _, groups = PLATFORMS[workflow]
    inherited = {g: {} for g in GROUPS}
    anchor = route["mode"] == "control-anchor"
    require(not anchor or not donors, "Anchor cannot inherit receipts")
    require(not anchor or not input_rows, "Anchor cannot declare inherited inputs")
    for donor in donors:
        require(donor["workflow"] == workflow, "Receipt platform differs")
        for group in groups:
            if group in fresh or donor["checks"][group] != checks[group] or donor["runtime"]["groups"][group] != runtime["groups"][group]:
                continue
            for target in donor["executed"].get(group, []):
                if (target in sources[group] and donor["sources"][group].get(target) == sources[group][target]
                        and not tree.closure(target) & tree.uncertain):
                    inherited[group][target] = {key: donor[key] for key in ("head", "id", "attempt", "job")}
    selected = {g: [t for t in targets[g] if t not in inherited[g]] if g in groups else [] for g in GROUPS}
    plan = {**selected, "version": VERSION, "event": "control_evidence_plan", "mode": ANCHOR_MODE if anchor else MODE,
            "head": route["head"], "head_tree": git(root, "rev-parse", route["head"] + "^{tree}").decode().strip(),
            "route": route, "workflow": workflow, "platform": platform, "runtime": runtime, "producer": producer,
            "checks": checks, "sources": sources, "inventory": targets, "inherited": inherited,
            "direct_receipts": donors, "receipt_inputs_sha256": digest(tree.content(INPUT)),
            "full": all(selected[g] == targets[g] for g in groups),
            "reason": "Immutable full anchor" if anchor else "Direct executed receipt coverage plus fresh delta",
            "cache_reads": False, "cache_writes": False, "artifact_writes": False,
            "state": {"version": "affected-inventory-only-v1", "targets": sources}}
    plan["plan_sha256"] = digest(canonical(plan))
    return plan


def completed(plan, groups):
    allowed = PLATFORMS[plan["workflow"]][2]
    require(set(groups) <= set(allowed), "Unsupported control evidence completed group")
    return {"event": "control_evidence_checks_completed", "version": VERSION, "mode": plan["mode"],
            "head": plan["head"], "plan_sha256": plan["plan_sha256"],
            "executed_now": {g: {"count": len(plan[g]), "targets_sha256": digest(canonical(plan[g]))} for g in groups}, "inherited_checks": "not_executed_now",
            "cache_written": False, "artifact_written": False, "intrinsic_checks": "not_run_control_only"}


def log_records(plan):
    # Git reconstructs inventories/fingerprints. Only bounded context and target
    # lists cross the log transport; no single giant JSON plan line.
    records = [{"event": "control_evidence_plan", "head": plan["head"], "workflow": plan["workflow"],
                "route": plan["route"], "runtime": plan["runtime"], "producer": plan["producer"], "plan_sha256": plan["plan_sha256"],
                "donors": len(plan["direct_receipts"])}]
    for index, donor in enumerate(plan["direct_receipts"]):
        records.append({"event": "control_evidence_donor", "index": index,
                        **{k: v for k, v in donor.items() if k not in ("sources", "checks", "executed")}})
        for group, targets in donor["executed"].items():
            for offset in range(0, max(1, len(targets)), TARGET_CHUNK):
                records.append({"event": "control_evidence_targets", "donor": index, "group": group,
                                "offset": offset, "targets": targets[offset:offset + TARGET_CHUNK]})
    for group in PLATFORMS[plan["workflow"]][2]:
        records.append({"event": "control_evidence_coverage", "group": group,
                        "inventory_sha256": digest(canonical(plan["inventory"][group])),
                        "source_fingerprints_sha256": digest(canonical(plan["sources"][group])),
                        "executed_now_count": len(plan[group]), "inherited_count": len(plan["inherited"][group])})
        for disposition, targets in (("selected", plan[group]), ("inherited", sorted(plan["inherited"][group]))):
            for offset in range(0, max(1, len(targets)), TARGET_CHUNK):
                records.append({"event": "control_evidence_targets", "disposition": disposition,
                                "group": group, "offset": offset, "targets": targets[offset:offset + TARGET_CHUNK]})
    require(all(len(json.dumps(row, sort_keys=True).encode()) <= LOG_LINE_LIMIT for row in records), "Control log record exceeds bound")
    return records


def logged_plan(root, lines, head, workflow):
    rows = proof.records(lines)
    records = [row for row in rows if str(row.get("event", "")).startswith("control_evidence_")
               and row.get("event") != "control_evidence_checks_completed"]
    require(all(len(json.dumps(row, sort_keys=True).encode()) <= LOG_LINE_LIMIT for row in records), "Control log record exceeds bound")
    header = proof.unique(records, lambda x: x.get("event") == "control_evidence_plan", "Missing unique complete-context plan")
    require(header.get("head") == head and header.get("workflow") == workflow
            and type(header.get("donors")) is int and 0 <= header["donors"] <= MAX_RECEIPTS,
            "Wrong control evidence plan identity")
    donors = []
    for index in range(header["donors"]):
        row = proof.unique(records, lambda x: x.get("event") == "control_evidence_donor" and x.get("index") == index,
                           "Missing direct donor record")
        donor = {k: v for k, v in row.items() if k not in ("event", "index")}
        tree = SourceTree(root, donor["head"])
        donor.update(sources=control_full.source_coverage(tree), checks=group_checks(tree, workflow), executed={})
        for group in PLATFORMS[workflow][2]:
            chunks = [r for r in records if r.get("event") == "control_evidence_targets"
                      and r.get("donor") == index and r.get("group") == group]
            chunks.sort(key=lambda r: r["offset"])
            targets = []
            for chunk in chunks:
                require(chunk["offset"] == len(targets) and isinstance(chunk["targets"], list)
                        and len(chunk["targets"]) <= TARGET_CHUNK, "Invalid direct target chunks")
                targets.extend(chunk["targets"])
            require(bool(chunks) and targets == sorted(set(targets)), "Missing or duplicate direct targets")
            donor["executed"][group] = targets
        donors.append(donor)
    plan = make_plan(root, header["route"], workflow, header["runtime"], donors, producer=header.get("producer"))
    require(plan["head"] == head and records == log_records(plan), "Control evidence plan/source/coverage differs")
    groups = PLATFORMS[workflow][2]
    executed = {}
    for item in (x for x in rows if x.get("event") == "control_evidence_checks_completed"):
        require(item == completed(plan, item.get("executed_now", {}).keys()), "Control completion differs")
        require(not set(executed) & set(item["executed_now"]), "Duplicate completed evidence group")
        executed.update({g: plan[g] for g in item["executed_now"]})
    require(set(executed) == set(groups), "Incomplete executed evidence groups")
    return plan, executed


def read_receipt(root, row, token, request, read_log, *, main=False):
    import no_cache_route as route
    workflow = row["workflow"]
    _, name, _ = PLATFORMS[workflow]
    prefix = "/repos/" + route.REPOSITORY + "/actions/runs/" + str(row["id"])
    branch = "main" if main else (ANCHOR_REF if row["kind"] == "anchor" else REF).removeprefix("refs/heads/")
    for run in (request(prefix, token), request(prefix + "/attempts/" + str(row["attempt"]), token)):
        proof.run_identity(run, row["id"], row["attempt"], row["head"], branch, workflow)
        require(run.get("event") == "push" and run.get("status") == "completed" and run.get("conclusion") == "success",
                "Direct execution receipt is not successful")
    for earlier in range(1, row["attempt"]):
        past = request(prefix + "/attempts/" + str(earlier), token)
        proof.run_identity(past, row["id"], earlier, row["head"], branch, workflow)
        require(past.get("event") == "push" and past.get("status") == "completed"
                and past.get("conclusion") == "success", "Direct receipt has an incomplete or unsuccessful earlier attempt")
        past_jobs = proof.complete_jobs(request(prefix + "/attempts/" + str(earlier) + "/jobs?per_page=100", token))
        prior_job = proof.unique(past_jobs, lambda j: j.get("name") == name, "Missing earlier required job")
        require(prior_job.get("conclusion") == "success", "Earlier required job did not succeed")
        for job in past_jobs:
            steps = job.get("steps")
            require(job.get("status") == "completed" and job.get("conclusion") in ("success", "skipped")
                    and isinstance(steps, list) and bool(steps) == (job["conclusion"] == "success")
                    and all(isinstance(s, dict) and s.get("status") == "completed"
                            and s.get("conclusion") in ("success", "skipped") for s in steps),
                    "Direct receipt has incomplete or unsuccessful earlier execution")
    jobs = proof.complete_jobs(request(prefix + "/attempts/" + str(row["attempt"]) + "/jobs?per_page=100", token))
    require(all(j.get("status") == "completed" and j.get("conclusion") in ("success", "skipped") for j in jobs),
            "Direct receipt has unsuccessful jobs")
    job = proof.unique(jobs, lambda j: j.get("name") == name, "Missing direct receipt job")
    require(job.get("run_id") == row["id"] and job.get("run_attempt") == row["attempt"]
            and job.get("head_sha") == row["head"] and job.get("head_branch") == branch
            and job.get("status") == "completed" and job.get("conclusion") == "success", "Wrong direct receipt job")
    steps = tuple(SELECTOR if s == "Select exact-head affected scope without cache" else s
                  for s in route.BASELINE_JOBS[workflow][1])
    steps += ("Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event")
    if name == "verify":
        steps += ("Install dependencies without cache", "Install Boost")
        for disposition in ("intrinsic", "deploy"):
            skipped = proof.unique(jobs, lambda j: j.get("name") == disposition, "Missing non-executed disposition")
            require(skipped.get("conclusion") == "skipped" and not skipped.get("steps"), "Unexpected intrinsic/deploy execution")
    stored = ("Restore successful verification results", "Cache judge test cases and checkers",
              "Save completed verification results", "Upload library website", "Install dependencies",
              "Select affected libraries and tests") if name == "verify" else (
                  "Run actions/cache/restore@v4", "Run actions/cache/save@v4", "Select affected headers and unit tests")
    stored += ("Select exact-head affected scope without cache", "Select exact-head full scope without cache")
    setup = [s for s in job.get("steps", []) if s.get("name") == "Run actions/setup-python@v5"]
    require(len(setup) == (2 if name == "verify" else 1)
            and sum(s.get("status") == "completed" and s.get("conclusion") == "success" for s in setup) == 1
            and all(s.get("conclusion") in ("success", "skipped") for s in setup), "Python setup did not execute as reviewed")
    for name in stored:
        step = proof.unique(job.get("steps", []), lambda s: s.get("name") == name, "Missing no-storage disposition")
        require(step.get("conclusion") == "skipped", "Unexpected cache/artifact execution")
    for name in steps:
        step = proof.unique(job.get("steps", []), lambda s: s.get("name") == name, "Missing direct executed step")
        require(step.get("status") == "completed" and step.get("conclusion") == "success", "Direct step was not executed")
    log = read_log(job["id"], token)
    require(len(log.encode()) <= proof.LOG_LIMIT, "Direct receipt log exceeds bound")
    lines = proof.log_lines(log)
    plan, executed = logged_plan(root, lines, row["head"], workflow)
    require(plan["mode"] == (ANCHOR_MODE if row["kind"] == "anchor" else MODE), "Direct receipt mode differs")
    ref = route.MAIN_REF if main else (ANCHOR_REF if row["kind"] == "anchor" else REF)
    event = {"ref": ref, "after": row["head"], "deleted": False,
             "before": route.parents(root, row["head"])[0] if main else plan["route"]["base"]}
    context = {"GITHUB_EVENT_NAME": "push", "GITHUB_REF": ref, "GITHUB_SHA": row["head"],
               "GITHUB_REPOSITORY": route.REPOSITORY}
    require(plan["route"] == select_event(root, event, context, clean=False), "Historical receipt route differs")
    require(isinstance(plan["producer"], dict)
            and type(plan["producer"].get("run_id")) is int and type(plan["producer"].get("run_attempt")) is int
            and plan["producer"] == {"run_id": row["id"], "run_attempt": row["attempt"], "job": PLATFORMS[workflow][1]},
            "Receipt producer differs from authoritative run/job")
    proof.runtime(lines, PLATFORMS[workflow][1])
    return {**row, "job": job["id"], "log_sha256": digest(log.encode()), "runtime": plan["runtime"],
            "checks": plan["checks"], "sources": plan["sources"], "executed": executed}, plan


def donor_receipts(root, tree, workflow, token, request, read_log):
    rows = inputs(tree)
    anchors = [r for r in rows if r["kind"] == "anchor"]
    require(len(anchors) == 2 and {r["workflow"] for r in anchors} == set(PLATFORMS)
            and len({r["head"] for r in anchors}) == 1, "Delta requires one complete two-platform anchor")
    result = []
    for row in rows:
        git(root, "merge-base", "--is-ancestor", row["head"], tree.commit)
        if row["workflow"] == workflow:
            donor, _ = read_receipt(root, row, token, request, read_log)
            result.append(donor)
    return result


def build_plan(root, head, *, request=None, read_log=None):
    import no_cache_route as route
    event = json.loads(open(os.environ["GITHUB_EVENT_PATH"]).read())
    selected = select_event(root, event, os.environ)
    require(selected["head"] == head, "Control evidence checkout differs")
    workflow = ".github/workflows/verify.yml" if os.environ.get("RUNNER_OS") == "Linux" else ".github/workflows/macos-portability.yml"
    require(os.environ.get("RUNNER_OS") in ("Linux", "macOS"), "Unsupported control runner")
    tree = SourceTree(root, head)
    if selected["mode"] == "control-anchor":
        require(not inputs(tree), "Immutable anchor must not inherit inputs")
        donors = []
    else:
        require(bool(os.environ.get("GH_TOKEN")), "Missing read-only receipt token")
        proof.check_main(root, selected["base"], os.environ["GH_TOKEN"], request or route.api_get)
        donors = donor_receipts(root, tree, workflow, os.environ["GH_TOKEN"], request or route.api_get,
                                read_log or proof.job_log)
    producer = {"run_id": int(os.environ["GITHUB_RUN_ID"]), "run_attempt": int(os.environ["GITHUB_RUN_ATTEMPT"]),
                "job": os.environ["GITHUB_JOB"]}
    require(producer["run_id"] > 0 and 0 < producer["run_attempt"] <= proof.MAX_ATTEMPTS
            and producer["job"] == PLATFORMS[workflow][1], "Wrong control producer context")
    return make_plan(root, selected, workflow, check_context.collect_runtime(), donors, producer=producer)


def validate_plan(root, plan):
    route = plan["route"]
    checked_source(root, route["base"], route["head"])
    changes(root, route["base"], route["head"])
    require(plan == make_plan(root, route, plan["workflow"], check_context.collect_runtime(), plan["direct_receipts"], producer=plan["producer"]),
            "Control evidence selection/context changed")


def bootstrap(root, route, evidence, token, request, read_log):
    import no_cache_route as routing
    base = route["base"]
    pair = routing.parents(root, base)
    require(len(pair) == 2 and marked(root, base, MARKER) and marked(root, pair[1], MARKER)
            and routing.parents(root, pair[1])[1:] == [pair[0]] and routing.same_tree(root, base, pair[1]),
            "Wrong actual-main control-delta merge")
    tree = changes(root, pair[0], base)
    controls = proof.historical_controls(tree)
    require(set(RECEIPT_CONTROLS) <= set(controls), "Delta historical control inventory is incomplete")
    require(evidence["qualification"]["reviewed_source_control_sha256"]
            == routing.source_hash(tree, controls), "Delta baseline control source differs")
    proof.check_main(root, base, token, request)
    receipts = []
    for workflow, identity in evidence["runs"].items():
        row = {**identity, "kind": "delta", "head": base, "workflow": workflow}
        current, plan = read_receipt(root, row, token, request, read_log, main=True)
        donors = donor_receipts(root, tree, workflow, token, request, read_log)
        require(plan == make_plan(root, plan["route"], workflow, plan["runtime"], donors, producer=plan["producer"]),
                "Actual-main coverage union lacks direct successful evidence")
        receipts.append({**identity, "workflow": workflow, "job": current["job"], "head": base,
                         "executed_now": current["executed"], "inherited": plan["inherited"]})
    proof.check_main(root, base, token, request)
    return {"policy": proof.POLICY, "basis": "control-delta-bootstrap", "receipts": receipts,
            "ordinary_coverage": "direct-success-receipt-union", "intrinsic_checks": "not_run_control_only",
            "full_baseline_target_coverage": "not_independently_established", "unaffected_current_head_checks": "not_run"}
