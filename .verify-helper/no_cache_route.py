"""Explicit, source-bound no-cache event routing. No cache or artifact APIs."""

import json
import os
from pathlib import Path
import re
import subprocess
import urllib.request

from affected import (BINDING_PATH, SourceTree, canonical, checked_source, digest, git,
                      read_binding, require, validate_binding)


REPOSITORY = "qandori/cp-library"
SOURCE_REF = "refs/heads/codex/fps-scalar-return-20261018"
MAIN_REF = "refs/heads/main"
TRAILER = "Verify-Mode: no-cache"
EVIDENCE_PATH = ".verify-helper/no-cache-baseline.json"
CONTROL_PATHS = (
    ".github/workflows/verify.yml", ".github/workflows/macos-portability.yml",
    ".verify-helper/affected.py", ".verify-helper/affected_test.py",
    ".verify-helper/ci.py", ".verify-helper/ci_test.py",
    ".verify-helper/no_cache_route.py", ".verify-helper/no_cache_route_test.py",
    ".verify-helper/intrinsic_gate.py", ".verify-helper/intrinsic_gate_test.py",
    '.github/workflows/dwm-avx-final.yml',
    'test/benchmark/dwm_avx_final/bench.cpp',
    'test/benchmark/dwm_avx_final/caller.cpp',
    'test/benchmark/dwm_avx_final/escape.cpp',
    'test/benchmark/dwm_avx_final/run.py',
    'test/benchmark/dwm_avx_final/vector_oracle.cpp',
    ".github/workflows/matrix-packed-final.yml",
    ".github/workflows/matrix-centered-final.yml",
    ".github/workflows/matrix-backward-final.yml",
    "test/benchmark/matrix.cpp", "test/benchmark/matrix.py",
    ".github/workflows/edit-distance-trim-final.yml",
    "test/benchmark/edit_distance_trim.cpp", "test/benchmark/edit_distance_trim_call.cpp",
    "test/benchmark/edit_distance.py",
    ".github/workflows/fps-scalar-return-final.yml",
    "test/benchmark/fps_scalar_return/api.hpp", "test/benchmark/fps_scalar_return/api.cpp",
    "test/benchmark/fps_scalar_return/bench.cpp", "test/benchmark/fps_scalar_return/run.py",
)
CONTROL_PATHS_V2 = CONTROL_PATHS + (
    ".verify-helper/tree_baseline.py", ".verify-helper/tree_baseline_test.py",
    ".verify-helper/control_full.py", ".verify-helper/control_full_test.py",
    ".verify-helper/control_delta.py", ".verify-helper/control_delta_test.py",
    ".verify-helper/check_context.py", ".verify-helper/check_context_test.py",
    ".verify-helper/control-receipts.json",
)
BASELINE_INPUTS = (
    ".github/workflows/verify.yml", ".github/workflows/macos-portability.yml",
    ".verify-helper/ci.py", ".verify-helper/ci_test.py", "test/verify.py",
    "test/requirements.txt", ".verify-helper/config.toml",
    ".verify-helper/check-docs.py", ".verify-helper/documentation.py",
    ".verify-helper/verification.py", ".verify-helper/verification_test.py",
)
BASELINE_JOBS = {
    ".github/workflows/verify.yml": ("verify", (
        "Test incremental verification", "Select exact-head affected scope without cache",
        "Run unit tests with GCC and Clang", "Run ASan and UBSan tests",
        "Run affected judge tests", "Generate documentation",
        "Check bundled documentation", "Build library website",
    )),
    ".github/workflows/macos-portability.yml": ("portable", (
        "Select exact-head affected scope without cache", "Check Apple Clang",
    )),
}


def source_hash(tree, paths):
    return digest(canonical([[path, tree.entries.get(path), digest(tree.content(path))]
                             for path in paths]))


def object_id(value, label):
    require(isinstance(value, str) and re.fullmatch("[0-9a-f]{40}", value),
            "Invalid " + label)
    return value


def parents(root, head):
    return git(root, "rev-list", "--parents", "-n", "1", head).decode().strip().split()[1:]


def same_tree(root, first, second):
    return git(root, "rev-parse", first + "^{tree}") == git(root, "rev-parse", second + "^{tree}")


def event_source(root, event, context):
    head = object_id(context.get("GITHUB_SHA"), "event head")
    kind, ref = context.get("GITHUB_EVENT_NAME"), context.get("GITHUB_REF")
    if kind == "pull_request":
        pull = event.get("pull_request", {})
        source = object_id(pull.get("head", {}).get("sha"), "PR head")
        return head, source, "refs/heads/" + context.get("GITHUB_HEAD_REF", "")
    return head, head, ref


def control_requested(root, head, source, source_ref, event, context):
    markers = ("Verify-Mode: control-full", "Verify-Mode: control-anchor", "Verify-Mode: control-delta")
    def marked(commit):
        message = git(root, "show", "-s", "--format=%B", commit).decode().rstrip()
        present = {marker for marker in markers if marker in message}
        final = message.splitlines()[-1] if message else None
        require(not present or present == {final}, "Reserved control marker must be the final nonempty line")
        return final in markers
    source_marked = marked(source)
    pair = parents(root, head) if source_ref == MAIN_REF else []
    requested = (source_ref in ("refs/heads/codex/ci-tree-reuse-20261018", "refs/heads/codex/ci-tree-anchor-20261018") or source_marked
                 or (len(pair) == 2 and marked(pair[1])))
    if pair and not requested:
        previous, current = SourceTree(root, pair[0]), SourceTree(root, head)
        control_paths = (".verify-helper/control_full.py", ".verify-helper/control_delta.py", ".verify-helper/check_context.py")
        control_path = control_paths[0]
        if context.get("GITHUB_EVENT_NAME") == "push" and current.entries.get(control_path) is not None:
            before = object_id(event.get("before"), "main control transition baseline")
            try:
                previous = SourceTree(root, before)
            except subprocess.CalledProcessError:
                raise ValueError("Cannot resolve the main control transition baseline") from None
        require(all(previous.entries.get(path) == current.entries.get(path) for path in control_paths),
                "Main control transition requires the explicit direct control route")
    return requested


def select_event(root, event, context):
    head, source, source_ref = event_source(root, event, context)
    if control_requested(root, head, source, source_ref, event, context):
        message = git(root, "show", "-s", "--format=%B", source).decode().rstrip()
        if (source_ref == "refs/heads/codex/ci-tree-anchor-20261018"
                or (message and message.splitlines()[-1] in ("Verify-Mode: control-anchor", "Verify-Mode: control-delta"))):
            from control_delta import select_event as select_control_event
        else:
            from control_full import select_event as select_control_event
        control = select_control_event(root, event, context)
        require(control is not None, "Missing explicit control route")
        return control
    message = git(root, "show", "-s", "--format=%B", source).decode().rstrip()
    marked = bool(message) and message.splitlines()[-1] == TRAILER
    if not marked:
        require(source_ref != SOURCE_REF, "Production branch requires the explicit no-cache trailer")
        require(TRAILER not in message, "No-cache trailer must be the final nonempty commit-message line")
        if source_ref == MAIN_REF:
            pair = parents(root, head)
            if pair:
                old, current = SourceTree(root, pair[0]), SourceTree(root, head)
                binding = current.entries.get(BINDING_PATH)
                require(binding is None or binding == old.entries.get(BINDING_PATH),
                        "Main binding transition requires the explicit no-cache trailer")
            if len(pair) == 2:
                candidate_message = git(root, "show", "-s", "--format=%B", pair[1]).decode().rstrip()
                require(not candidate_message or candidate_message.splitlines()[-1] != TRAILER,
                        "Main merge of an opted-in candidate requires the explicit no-cache trailer")
        return {"mode": "normal"}
    require(context.get("GITHUB_REPOSITORY") == REPOSITORY, "Wrong repository")
    current = SourceTree(root, head)
    binding = read_binding(current)
    require(binding["ref"] == SOURCE_REF, "Binding is not for the reviewed production branch")
    base = binding["base"]
    checked_source(root, base, head)
    validate_binding(SourceTree(root, base), current, SOURCE_REF)
    kind, ref = context.get("GITHUB_EVENT_NAME"), context.get("GITHUB_REF")
    if kind == "push":
        require(event.get("deleted") is False and event.get("after") == head
                and event.get("ref") == ref, "Push payload differs from event checkout")
        if ref == SOURCE_REF:
            require(source == head, "Production push source differs from checkout")
        else:
            require(ref == MAIN_REF, "No-cache push is restricted to the production branch or main")
            pair = parents(root, head)
            require(event.get("before") == base and len(pair) == 2 and pair[0] == base,
                    "Main no-cache event requires a two-parent merge onto the exact baseline")
            source = pair[1]
            require(same_tree(root, head, source), "Main merge changed the reviewed candidate tree")
            candidate_message = git(root, "show", "-s", "--format=%B", source).decode().rstrip()
            require(bool(candidate_message) and candidate_message.splitlines()[-1] == TRAILER,
                    "Merge source was not explicitly opted in")
    elif kind == "pull_request":
        pull = event.get("pull_request", {})
        require(source_ref == SOURCE_REF and pull.get("head", {}).get("ref") == SOURCE_REF.removeprefix("refs/heads/"),
                "Wrong PR source branch")
        require(pull.get("head", {}).get("repo", {}).get("full_name") == REPOSITORY,
                "No-cache PR must originate in this repository")
        require(pull.get("base", {}).get("sha") == base and pull.get("base", {}).get("ref") == "main"
                and pull.get("base", {}).get("repo", {}).get("full_name") == REPOSITORY,
                "PR base differs from the pinned main baseline")
        require(type(event.get("number")) is int and ref == "refs/pull/" + str(event["number"]) + "/merge",
                "Unexpected PR event ref")
        require(parents(root, head) == [base, source] and same_tree(root, head, source),
                "PR merge checkout differs from exact reviewed source")
    else:
        raise ValueError("No-cache mode supports push and pull_request only")
    require(read_binding(SourceTree(root, source)) == binding, "Candidate binding differs from merge binding")
    return {"mode": "no-cache", "base": base, "head": head, "ref": SOURCE_REF,
            "source_head": source, "source_digest": binding["source_digest"], "event": kind}


def read_evidence(tree):
    raw = tree.content(EVIDENCE_PATH)
    require(raw is not None, "Missing reviewed exact-baseline CI evidence")
    value = json.loads(raw)
    require(raw == canonical(value) + b"\n", "Baseline evidence must be canonical JSON")
    fields = {"version", "base", "baseline_source_sha256", "reviewed_control_sha256", "policy", "runs"}
    require(isinstance(value, dict) and type(value.get("version")) is int and value["version"] in (1, 2)
            and set(value) == fields | ({"qualification"} if value["version"] == 2 else set()),
            "Invalid baseline evidence schema")
    if value["version"] == 1:
        require(value["policy"] == "exact-baseline-ci-and-reviewed-source-keys-v1", "Unreviewed baseline proof policy")
    else:
        from tree_baseline import POLICY
        require(value["policy"] == POLICY, "Unreviewed baseline proof policy")
        proof = value["qualification"]
        require(isinstance(proof, dict) and proof.get("kind") in ("source-push", "control-full", "control-delta"), "Invalid qualification kind")
        require(set(proof) == {"kind", "reviewed_source_control_sha256"}
                | ({"runs"} if proof["kind"] == "source-push" else set()), "Invalid qualification schema")
        require(isinstance(proof["reviewed_source_control_sha256"], str)
                and re.fullmatch("[0-9a-f]{64}", proof["reviewed_source_control_sha256"]), "Invalid qualification source hash")
        if proof["kind"] == "source-push":
            validate_run_ids(proof["runs"])
    validate_run_ids(value["runs"])
    return value


def validate_run_ids(runs):
    require(isinstance(runs, dict) and set(runs) == set(BASELINE_JOBS), "Missing baseline workflows")
    for run in runs.values():
        require(isinstance(run, dict) and set(run) == {"id", "attempt"}
                and all(type(run[key]) is int and run[key] > 0 for key in run), "Invalid exact baseline run identity")


def api_get(path, token):
    require(path.startswith("/repos/" + REPOSITORY + "/actions/"), "Unexpected Actions API path")
    request = urllib.request.Request("https://api.github.com" + path, headers={
        "Authorization": "Bearer " + token, "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28", "User-Agent": "cp-library-no-cache-ci",
    })
    class NoRedirect(urllib.request.HTTPRedirectHandler):
        def redirect_request(self, *args, **kwargs):
            raise ValueError("Unexpected Actions API redirect")
    with urllib.request.build_opener(NoRedirect()).open(request, timeout=30) as response:
        return json.load(response)


def verify_baseline(root, route, token, request=api_get, read_log=None):
    require(bool(token), "Missing read-only Actions token")
    old, current = SourceTree(root, route["base"]), SourceTree(root, route["head"])
    evidence = read_evidence(current)
    require(evidence["base"] == route["base"], "Baseline evidence binds another source commit")
    require(evidence["baseline_source_sha256"] == source_hash(old, BASELINE_INPUTS), "Baseline selector/check sources differ")
    controls = CONTROL_PATHS if evidence["version"] == 1 else CONTROL_PATHS_V2
    require(evidence["reviewed_control_sha256"] == source_hash(current, controls),
            "CI control, compiler/setup, or workflow source changed; review scope again")
    if evidence["version"] == 2:
        import tree_baseline
        read_log = read_log or tree_baseline.job_log
        if evidence["qualification"]["kind"] == "control-delta":
            from control_delta import bootstrap
            return bootstrap(root, route, evidence, token, request, read_log)
        if evidence["qualification"]["kind"] == "control-full":
            return tree_baseline.bootstrap(root, route, evidence, token, request, read_log, BASELINE_JOBS)
        try:
            tree_baseline.check_main(root, route["base"], token, request)
            return tree_baseline.qualify(root, route, evidence["qualification"], token, request, read_log, BASELINE_JOBS)
        except tree_baseline.MainFailure:
            raise
        except (ValueError, OSError, KeyError, TypeError, subprocess.CalledProcessError):
            # Source evidence never substitutes for missing or failed exact-main evidence.
            pass
    receipts = []
    for workflow, proof in evidence["runs"].items():
        path = "/repos/" + REPOSITORY + "/actions/runs/" + str(proof["id"]) + "/attempts/" + str(proof["attempt"])
        run = request(path, token)
        require(run.get("id") == proof["id"] and run.get("run_attempt") == proof["attempt"]
                and run.get("head_sha") == route["base"] and run.get("event") in ("push", "workflow_dispatch")
                and run.get("path") == workflow and run.get("status") == "completed"
                and run.get("conclusion") == "success", "Baseline run is not successful at the exact bound source")
        payload = request(path + "/jobs?per_page=100", token)
        jobs = payload.get("jobs", [])
        require(type(payload.get("total_count")) is int and payload["total_count"] == len(jobs)
                and all(job.get("status") == "completed" and job.get("conclusion") in ("success", "skipped") for job in jobs),
                "Baseline job evidence is incomplete or unsuccessful")
        job_name, required_steps = BASELINE_JOBS[workflow]
        matches = [job for job in jobs if job.get("name") == job_name]
        require(len(matches) == 1 and matches[0].get("conclusion") == "success",
                "Baseline required job is missing, ambiguous, or unsuccessful")
        for name in required_steps:
            steps = [step for step in matches[0].get("steps", []) if step.get("name") == name]
            require(len(steps) == 1 and steps[0].get("status") == "completed" and steps[0].get("conclusion") == "success",
                    "Baseline required step did not succeed: " + name)
        receipts.append({"workflow": workflow, **proof, "head": route["base"], "job_id": matches[0]["id"]})
    return {"policy": evidence["policy"], "receipts": receipts,
            "full_baseline_target_coverage": "not_independently_established",
            "unaffected_current_head_checks": "not_run"}


def main():
    root = Path(__file__).resolve().parent.parent
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text())
    route = select_event(root, event, os.environ)
    if route["mode"] == "no-cache":
        proof = verify_baseline(root, route, os.environ.get("GH_TOKEN"))
        label = "source_push_baseline_evidence" if proof.get("basis") == "source-push-identical-tree" else "exact_baseline_evidence"
        print(json.dumps({"event": label, **proof}, sort_keys=True), flush=True)
        with open(os.environ["GITHUB_ENV"], "a") as stream:
            stream.write("VERIFY_NO_CACHE=true\nPIP_NO_CACHE_DIR=1\n")
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as stream:
            basis = ("Historical source-push checks qualify an identical-tree main baseline; this is not execution on main. "
                     if proof.get("basis") == "source-push-identical-tree" else
                     "Exact-main control-only full workflows succeeded; intrinsic execution is NOT claimed. "
                     if proof.get("basis") == "control-full-bootstrap" else
                     "Exact-baseline CI succeeded under reviewed source-key semantics; fresh full-baseline coverage is not claimed. ")
            stream.write("Explicit no-cache run at " + route["head"] + ".\n\n" + basis +
                         "Current-head unaffected checks are NOT RUN. Pages publication is omitted.\n")
    elif route["mode"] in ("control-anchor", "control-delta"):
        with open(os.environ["GITHUB_ENV"], "a") as stream:
            stream.write("VERIFY_NO_CACHE=true\nVERIFY_CONTROL_EVIDENCE=true\nPIP_NO_CACHE_DIR=1\n")
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as stream:
            stream.write("Control-only evidence validation at " + route["head"] + ". "
                         "Immutable anchor executes all ordinary checks; delta inherits only direct successful matching checks. "
                         "Inherited checks are NOT EXECUTED NOW. No cache/artifact/Pages writes; intrinsic NOT RUN.\n")
    elif route["mode"] == "control-full":
        with open(os.environ["GITHUB_ENV"], "a") as stream:
            stream.write("VERIFY_NO_CACHE=true\nVERIFY_CONTROL_FULL=true\nPIP_NO_CACHE_DIR=1\n")
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as stream:
            stream.write("Control-only full ordinary verification at " + route["head"] + ". "
                         "No successful-result cache reads/writes, artifacts, or Pages publication. "
                         "Intrinsic checks are NOT RUN in this control-only mode.\n")
    with open(os.environ["GITHUB_OUTPUT"], "a") as stream:
        for name, value in route.items():
            stream.write(name + "=" + value + "\n")
    print(json.dumps({"event": "verification_route", **route}, sort_keys=True), flush=True)


if __name__ == "__main__":
    main()
