import copy
import json
import unittest

import affected_test
from affected import BINDING_PATH, SourceTree, binding_source_digest, canonical, read_binding
import no_cache_route as route


class NoCacheRouteTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit

    def setUp(self):
        affected_test.AffectedVerification.setUp(self)
        self.ref = route.SOURCE_REF
        for path in set(route.BASELINE_INPUTS) | set(route.CONTROL_PATHS):
            if not (self.root / path).exists():
                self.write(path, "# Source-only fixture.\n")
        self.base = self.commit("baseline inputs", bind=False)
        tree = SourceTree(self.root, self.base)
        evidence = {
            "version": 1, "base": self.base,
            "baseline_source_sha256": route.source_hash(tree, route.BASELINE_INPUTS),
            "reviewed_control_sha256": route.source_hash(tree, route.CONTROL_PATHS),
            "policy": "exact-baseline-ci-and-reviewed-source-keys-v1",
            "runs": {name: {"id": number, "attempt": 1}
                     for number, name in enumerate(route.BASELINE_JOBS, 1)},
        }
        self.write(route.EVIDENCE_PATH, (canonical(evidence) + b"\n").decode())
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing changed = int;\n")
        self.head = self.commit("candidate\n\n" + route.TRAILER)

    def push(self, head=None, ref=None, before=None):
        head, ref = head or self.head, ref or self.ref
        return ({"ref": ref, "after": head, "before": before or self.base, "deleted": False},
                {"GITHUB_EVENT_NAME": "push", "GITHUB_REPOSITORY": route.REPOSITORY,
                 "GITHUB_REF": ref, "GITHUB_SHA": head})

    def selected(self):
        return route.select_event(self.root, *self.push())

    def merge(self, marked=True):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        message = "Merge candidate" + ("\n\n" + route.TRAILER if marked else "")
        self.git("merge", "--quiet", "--no-ff", self.head, "-m", message)
        return self.git("rev-parse", "HEAD").decode().strip()

    def pr(self, merged):
        event = {"number": 7, "pull_request": {
            "head": {"sha": self.head, "ref": self.ref.removeprefix("refs/heads/"),
                     "repo": {"full_name": route.REPOSITORY}},
            "base": {"sha": self.base, "ref": "main", "repo": {"full_name": route.REPOSITORY}},
        }}
        context = {"GITHUB_EVENT_NAME": "pull_request", "GITHUB_REPOSITORY": route.REPOSITORY,
                   "GITHUB_SHA": merged, "GITHUB_REF": "refs/pull/7/merge",
                   "GITHUB_HEAD_REF": self.ref.removeprefix("refs/heads/")}
        return event, context

    def amend_bound_merge(self):
        self.git("add", "--all")
        self.git("commit", "--quiet", "--amend", "--no-edit")
        head = self.git("rev-parse", "HEAD").decode().strip()
        current = SourceTree(self.root, head)
        binding = read_binding(current)
        binding["source_digest"] = binding_source_digest(SourceTree(self.root, self.base), current, self.ref)
        self.write(BINDING_PATH, (canonical(binding) + b"\n").decode())
        self.git("add", BINDING_PATH)
        self.git("commit", "--quiet", "--amend", "--no-edit")
        return self.git("rev-parse", "HEAD").decode().strip()

    def forbidden_api(self, path, token):
        self.fail("Invalid source evidence must fail before any API request")

    def api(self, path, token):
        self.assertEqual(token, "fixture-token")
        for number, (workflow, (job_name, steps)) in enumerate(route.BASELINE_JOBS.items(), 1):
            prefix = "/repos/" + route.REPOSITORY + "/actions/runs/" + str(number) + "/attempts/1"
            if path == prefix:
                return {"id": number, "run_attempt": 1, "head_sha": self.base, "event": "push",
                        "path": workflow, "status": "completed", "conclusion": "success"}
            if path == prefix + "/jobs?per_page=100":
                return {"total_count": 1, "jobs": [{"id": number * 10, "name": job_name,
                    "status": "completed", "conclusion": "success",
                    "steps": [{"name": name, "status": "completed", "conclusion": "success"} for name in steps]}]}
        self.fail("Unexpected API path: " + path)

    def test_exact_production_source_and_bounded_baseline_evidence(self):
        selected = self.selected()
        self.assertEqual(selected["head"], self.head)
        self.assertEqual(selected["source_head"], self.head)
        proof = route.verify_baseline(self.root, selected, "fixture-token", self.api)
        self.assertEqual(proof["full_baseline_target_coverage"], "not_independently_established")
        self.assertEqual(proof["unaffected_current_head_checks"], "not_run")
        self.assertEqual(len(proof["receipts"]), 2)

    def test_fps_source_ref_is_exact_and_old_bound_sources_fail_closed(self):
        self.assertEqual(route.SOURCE_REF, "refs/heads/codex/fps-scalar-return-20261018")
        self.assertEqual(self.selected()["ref"], route.SOURCE_REF)
        for ref in ("refs/heads/codex/dwm-neighbor-20261017",
                    "refs/heads/codex/matrix-backward-20261018",
                    "refs/heads/codex/edit-distance-trim-20261018"):
            with self.subTest(ref=ref):
                self.ref = ref
                self.head = self.commit("other eligible binding is not this route\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "reviewed production branch"):
                    self.selected()

    def test_edit_distance_control_files_are_bound_before_baseline_api(self):
        paths = (".github/workflows/edit-distance-trim-final.yml",
                 "test/benchmark/edit_distance_trim.cpp", "test/benchmark/edit_distance_trim_call.cpp",
                 "test/benchmark/edit_distance.py")
        original = self.head
        for path in paths:
            with self.subTest(path=path):
                self.assertEqual(route.CONTROL_PATHS.count(path), 1)
                self.git("checkout", "--quiet", original)
                self.write(path, "Changed comparison control.\n")
                self.head = self.commit("changed edit-distance control\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "CI control"):
                    route.verify_baseline(self.root, self.selected(), "fixture-token", self.forbidden_api)

    def test_fps_control_files_are_bound_before_baseline_api(self):
        paths = (".github/workflows/fps-scalar-return-final.yml",
                 "test/benchmark/fps_scalar_return/api.hpp", "test/benchmark/fps_scalar_return/api.cpp",
                 "test/benchmark/fps_scalar_return/bench.cpp", "test/benchmark/fps_scalar_return/run.py")
        original = self.head
        for path in paths:
            with self.subTest(path=path):
                self.assertEqual(route.CONTROL_PATHS.count(path), 1)
                self.git("checkout", "--quiet", original)
                self.write(path, "Changed FPS comparison control.\n")
                self.head = self.commit("changed FPS control\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "CI control"):
                    route.verify_baseline(self.root, self.selected(), "fixture-token", self.forbidden_api)

    def test_exact_pr_synthetic_merge_and_event_identity(self):
        merged = self.merge(marked=False)
        event, context = self.pr(merged)
        selected = route.select_event(self.root, event, context)
        self.assertEqual(selected["head"], merged)
        self.assertEqual(selected["source_head"], self.head)
        for field, value in (("sha", "f" * 40), ("ref", "other"), ("repo", {"full_name": "fork/library"})):
            bad = copy.deepcopy(event)
            bad["pull_request"]["base"][field] = value
            with self.assertRaises(ValueError):
                route.select_event(self.root, bad, context)
        with self.assertRaises(ValueError):
            route.select_event(self.root, event, {**context, "GITHUB_REF": "refs/pull/8/merge"})
        bad = copy.deepcopy(event)
        bad["pull_request"]["head"]["repo"]["full_name"] = "fork/library"
        with self.assertRaises(ValueError):
            route.select_event(self.root, bad, context)

    def test_exact_main_merge_uses_real_event_sha_not_candidate_sha(self):
        merged = self.merge()
        selected = route.select_event(self.root, *self.push(merged, route.MAIN_REF))
        self.assertNotEqual(merged, self.head)
        self.assertEqual(selected["head"], merged)
        self.assertEqual(selected["source_head"], self.head)
        with self.assertRaises(ValueError):
            route.select_event(self.root, *self.push(merged, route.MAIN_REF, "f" * 40))

    def test_normal_main_and_dispatch_keep_default_path(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        head = self.commit("ordinary main change", bind=False)
        event, context = self.push(head, route.MAIN_REF)
        self.assertEqual(route.select_event(self.root, event, context), {"mode": "normal"})
        self.assertEqual(route.select_event(self.root, event, {**context, "GITHUB_EVENT_NAME": "workflow_dispatch"}),
                         {"mode": "normal"})

    def test_dropped_main_merge_marker_never_enables_normal_storage(self):
        merged = self.merge(marked=False)
        event, context = self.push(merged, route.MAIN_REF)
        for kind in ("push", "workflow_dispatch"):
            with self.assertRaisesRegex(ValueError, "explicit no-cache trailer"):
                route.select_event(self.root, event, {**context, "GITHUB_EVENT_NAME": kind})

    def test_missing_or_misplaced_marker_fails_closed_on_production_branch(self):
        for message in ("candidate without marker", route.TRAILER + "\n\nLater text"):
            head = self.commit(message)
            with self.assertRaises(ValueError):
                route.select_event(self.root, *self.push(head))

    def test_marked_unsupported_event_and_ref_fail_closed(self):
        event, context = self.push()
        for replacement in ({"GITHUB_EVENT_NAME": "workflow_dispatch"}, {"GITHUB_REF": "refs/heads/other"},
                            {"GITHUB_REPOSITORY": "fork/library"}):
            with self.assertRaises(ValueError):
                route.select_event(self.root, event, {**context, **replacement})
        for replacement in ({"deleted": True}, {"after": "f" * 40}, {"ref": "refs/heads/other"}):
            with self.assertRaises(ValueError):
                route.select_event(self.root, {**event, **replacement}, context)

    def test_squash_or_rebase_main_is_rejected(self):
        with self.assertRaises(ValueError):
            route.select_event(self.root, *self.push(self.head, route.MAIN_REF))

    def test_changed_ci_controls_require_separate_proof_review(self):
        self.write(".github/workflows/verify.yml", "# Changed setup, compiler, or controls.\n")
        self.head = self.commit("changed controls\n\n" + route.TRAILER)
        selected = self.selected()
        with self.assertRaisesRegex(ValueError, "CI control"):
            route.verify_baseline(self.root, selected, "fixture-token", self.api)

    def test_missing_baseline_evidence_fails_before_api(self):
        (self.root / route.EVIDENCE_PATH).unlink()
        self.head = self.commit("missing baseline evidence\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "Missing reviewed exact-baseline CI evidence"):
            route.verify_baseline(self.root, self.selected(), "fixture-token", self.forbidden_api)

    def test_corrupt_baseline_evidence_fails_before_api(self):
        original = self.head
        good = json.loads((self.root / route.EVIDENCE_PATH).read_text())
        invalid_schema = {**good, "runs": {}}
        for raw in ("{broken JSON\n", (canonical(invalid_schema) + b"\n").decode()):
            with self.subTest(raw=raw):
                self.git("checkout", "--quiet", original)
                self.write(route.EVIDENCE_PATH, raw)
                self.head = self.commit("corrupt baseline evidence\n\n" + route.TRAILER)
                with self.assertRaises(ValueError):
                    route.verify_baseline(self.root, self.selected(), "fixture-token", self.forbidden_api)

    def test_noncanonical_baseline_evidence_fails_before_api(self):
        original = self.head
        good = json.loads((self.root / route.EVIDENCE_PATH).read_text())
        exact = canonical(good).decode()
        variants = (json.dumps(good, indent=2) + "\n", exact, exact[:-1] + ',"version":1}\n')
        for raw in variants:
            with self.subTest(raw=raw):
                self.git("checkout", "--quiet", original)
                self.write(route.EVIDENCE_PATH, raw)
                self.head = self.commit("noncanonical baseline evidence\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "Baseline evidence must be canonical JSON"):
                    route.verify_baseline(self.root, self.selected(), "fixture-token", self.forbidden_api)

    def test_main_merge_tree_drift_fails_even_after_valid_rebinding(self):
        self.merge()
        self.write("math/unrelated.hpp", "#pragma once\nusing merge_only = int;\n")
        drifted = self.amend_bound_merge()
        self.assertEqual(route.parents(self.root, drifted), [self.base, self.head])
        with self.assertRaisesRegex(ValueError, "Main merge changed the reviewed candidate tree"):
            route.select_event(self.root, *self.push(drifted, route.MAIN_REF))

    def test_pr_merge_tree_drift_fails_even_after_valid_rebinding(self):
        self.merge(marked=False)
        self.write("math/unrelated.hpp", "#pragma once\nusing merge_only = int;\n")
        drifted = self.amend_bound_merge()
        self.assertEqual(route.parents(self.root, drifted), [self.base, self.head])
        with self.assertRaisesRegex(ValueError, "PR merge checkout differs from exact reviewed source"):
            route.select_event(self.root, *self.pr(drifted))

    def test_stale_push_payload_and_checkout_are_rejected(self):
        previous = self.head
        self.head = self.commit("new candidate identity\n\n" + route.TRAILER)
        self.assertNotEqual(previous, self.head)
        old_event, old_context = self.push(previous)
        with self.assertRaisesRegex(ValueError, "Checkout is not the expected head"):
            route.select_event(self.root, old_event, old_context)
        _, current_context = self.push()
        with self.assertRaisesRegex(ValueError, "Push payload differs from event checkout"):
            route.select_event(self.root, old_event, current_context)

    def test_stale_pr_candidate_does_not_match_synthetic_merge(self):
        previous = self.head
        self.head = self.commit("new candidate identity\n\n" + route.TRAILER)
        self.assertNotEqual(previous, self.head)
        merged = self.merge(marked=False)
        event, context = self.pr(merged)
        event["pull_request"]["head"]["sha"] = previous
        with self.assertRaisesRegex(ValueError, "PR merge checkout differs from exact reviewed source"):
            route.select_event(self.root, event, context)

    def test_wrong_or_incomplete_baseline_run_fails(self):
        selected = self.selected()
        for key, value in (("head_sha", "f" * 40), ("conclusion", "failure"), ("conclusion", "cancelled"),
                           ("status", "in_progress"), ("run_attempt", 2), ("event", "pull_request"),
                           ("path", ".github/workflows/other.yml")):
            def bad(path, token, key=key, value=value):
                result = self.api(path, token)
                if "/jobs?" not in path:
                    result[key] = value
                return result
            with self.assertRaises(ValueError):
                route.verify_baseline(self.root, selected, "fixture-token", bad)

    def test_failed_skipped_missing_or_paginated_baseline_steps_fail(self):
        selected = self.selected()
        for mode in ("failed", "skipped", "missing", "duplicate", "pagination", "empty"):
            def bad(path, token):
                result = self.api(path, token)
                if "/jobs?" in path:
                    if mode in ("failed", "skipped"):
                        result["jobs"][0]["steps"][0]["conclusion"] = mode
                    elif mode == "missing":
                        result["jobs"][0]["steps"].pop()
                    elif mode == "duplicate":
                        result["jobs"].append(copy.deepcopy(result["jobs"][0]))
                        result["total_count"] = 2
                    elif mode == "pagination":
                        result["total_count"] = 2
                    else:
                        result = {"total_count": 0, "jobs": []}
                return result
            with self.assertRaises(ValueError):
                route.verify_baseline(self.root, selected, "fixture-token", bad)

    def test_api_error_propagates_without_fallback(self):
        def failed(path, token):
            raise TimeoutError("Fixture API timeout")
        with self.assertRaises(TimeoutError):
            route.verify_baseline(self.root, self.selected(), "fixture-token", failed)
        with self.assertRaises(ValueError):
            route.verify_baseline(self.root, self.selected(), "", self.api)


class NoCacheWorkflowTests(unittest.TestCase):
    def workflow(self, name):
        from pathlib import Path
        import yaml
        path = Path(__file__).resolve().parent.parent / ".github/workflows" / name
        return yaml.load(path.read_text(), Loader=yaml.BaseLoader)

    def test_storage_and_setup_are_gated_without_hiding_checks(self):
        for filename, job_name in (("verify.yml", "verify"), ("macos-portability.yml", "portable")):
            workflow = self.workflow(filename)
            self.assertEqual(workflow["permissions"], {"contents": "read", "actions": "read"})
            job = workflow["jobs"][job_name]
            steps = job["steps"]
            gate = next(index for index, step in enumerate(steps) if step.get("id") == "route")
            for index, step in enumerate(steps):
                action = step.get("uses", "")
                if action.startswith(("actions/cache", "actions/upload-pages-artifact")):
                    self.assertGreater(index, gate)
                    self.assertIn("steps.route.outputs.mode == 'normal'", step["if"])
                self.assertNotIn("continue-on-error", step)
                if action.startswith("actions/setup-python") and "cache" in step.get("with", {}):
                    self.assertEqual(step["if"], "steps.route.outputs.mode == 'normal'")
            self.assertEqual(job["runs-on"], "ubuntu-24.04" if job_name == "verify" else "macos-14")
            self.assertEqual(job["timeout-minutes"], "120" if job_name == "verify" else "30")
            self.assertEqual(next(step for step in steps if step.get("uses", "").startswith("actions/checkout"))["with"]["persist-credentials"], "false")
            self.assertTrue(any(step.get("name") == "Select exact-head affected scope without cache" for step in steps))
        workflow = self.workflow("verify.yml")
        steps = workflow["jobs"]["verify"]["steps"]
        for name in route.BASELINE_JOBS[".github/workflows/verify.yml"][1]:
            step = next(step for step in steps if step.get("name") == name)
            if name == "Select exact-head affected scope without cache":
                self.assertEqual(step["if"], "steps.route.outputs.mode == 'no-cache'")
            else:
                self.assertNotIn("if", step)
        normal_selector = next(step for step in steps if step.get("name") == "Select affected libraries and tests")
        self.assertEqual(normal_selector["if"], "steps.route.outputs.mode == 'normal'")
        self.assertIn("needs.verify.outputs.mode == 'normal'", workflow["jobs"]["deploy"]["if"])
        self.assertTrue(any("--no-cache-dir" in step.get("run", "") for step in steps))

    def test_centered_comparison_uses_existing_fixed_driver(self):
        workflow = self.workflow("matrix-centered-final.yml")
        self.assertEqual(workflow["permissions"], {"contents": "read"})
        self.assertEqual(set(workflow["on"]), {"push"})
        self.assertEqual(workflow["on"]["push"]["branches"], ["codex/matrix-centered-20261018"])
        self.assertEqual(set(workflow["on"]["push"]["paths"]), {
            "components/matrix_arithmetic.hpp", "math/linear_algebra/matrix.hpp", "math/mint.hpp",
            "test/benchmark/matrix.cpp", "test/benchmark/matrix.py",
            ".github/workflows/matrix-centered-final.yml",
        })
        self.assertEqual(set(workflow["jobs"]), {"compare"})
        job = workflow["jobs"]["compare"]
        self.assertEqual(job["runs-on"], "ubuntu-26.04")
        self.assertEqual(job["timeout-minutes"], "10")
        self.assertEqual(len(job["steps"]), 2)
        checkout, compare = job["steps"]
        self.assertEqual(checkout, {"uses": "actions/checkout@v4", "with": {
            "fetch-depth": "0", "persist-credentials": "false"}})
        self.assertEqual(compare["timeout-minutes"], "8")
        self.assertEqual(compare["run"].split(), [
            "python3", "-B", "test/benchmark/matrix.py", "--base-ref",
            "36e2dcc732f89d7b0b3c4e47cb95155f5a41893d", "--candidate-ref", '"$GITHUB_SHA"',
        ])
        self.assertNotIn("continue-on-error", compare)
        self.assertIn(".github/workflows/matrix-centered-final.yml", route.CONTROL_PATHS)

    def test_backward_comparison_uses_existing_fixed_driver(self):
        workflow = self.workflow("matrix-backward-final.yml")
        self.assertEqual(workflow["permissions"], {"contents": "read"})
        self.assertEqual(set(workflow["on"]), {"push"})
        self.assertEqual(workflow["on"]["push"]["branches"], ["codex/matrix-backward-20261018"])
        self.assertEqual(set(workflow["on"]["push"]["paths"]), {
            "components/matrix_arithmetic.hpp", "math/linear_algebra/matrix.hpp", "math/mint.hpp",
            "test/benchmark/matrix.cpp", "test/benchmark/matrix.py",
            ".github/workflows/matrix-backward-final.yml",
        })
        self.assertEqual(set(workflow["jobs"]), {"compare"})
        job = workflow["jobs"]["compare"]
        self.assertEqual(job["runs-on"], "ubuntu-26.04")
        self.assertEqual(job["timeout-minutes"], "10")
        self.assertEqual(len(job["steps"]), 2)
        checkout, compare = job["steps"]
        self.assertEqual(checkout, {"uses": "actions/checkout@v4", "with": {
            "fetch-depth": "0", "persist-credentials": "false"}})
        self.assertEqual(compare["timeout-minutes"], "8")
        self.assertEqual(compare["run"].split(), [
            "python3", "-B", "test/benchmark/matrix.py", "--base-ref",
            "8d0babd9e56d88ea1ff332ded0c6b60f76680444", "--candidate-ref", '"$GITHUB_SHA"', "--comparison-scope", "backward",
        ])
        self.assertNotIn("continue-on-error", compare)
        self.assertIn(".github/workflows/matrix-backward-final.yml", route.CONTROL_PATHS)
        import runpy
        from pathlib import Path
        driver = runpy.run_path(str(Path(__file__).resolve().parents[1] / "test/benchmark/matrix.py"))
        cases, pairs, _ = driver["COMPARISON_SCOPES"]["backward"]
        self.assertEqual(cases, [
            ('inverse', 'dense', 64, 0, 64), ('inverse', 'dense', 384, 0, 384),
            ('inverse', 'dense', 512, 0, 512), ('inverse', 'singular', 384, 0, 384),
            ('solve', 'dense', 384, 0, 384), ('det', 'dense', 512, 0, 512),
        ])
        self.assertEqual(pairs, 4)
        self.assertEqual(len(driver["COMPARISON_SCOPES"]["product"][0]), 12)
        self.assertEqual(driver["COMPARISON_SCOPES"]["product"][1], 6)


    def test_fps_comparison_uses_exact_read_only_fixed_source(self):
        workflow = self.workflow("fps-scalar-return-final.yml")
        self.assertEqual(workflow["permissions"], {"contents": "read"})
        self.assertEqual(set(workflow["on"]), {"push"})
        self.assertEqual(workflow["on"]["push"]["branches"], ["codex/fps-scalar-return-20261018"])
        self.assertEqual(set(workflow["on"]["push"]["paths"]), {
            "fps/fps.hpp", "components/fps_operations.hpp", "components/ntt.hpp", "math/mint.hpp",
            "test/benchmark/fps_scalar_return/**", ".github/workflows/fps-scalar-return-final.yml",
        })
        self.assertEqual(set(workflow["jobs"]), {"compare"})
        job = workflow["jobs"]["compare"]
        self.assertIn("github.run_attempt == 1", job["if"])
        self.assertIn("refs/heads/codex/fps-scalar-return-20261018", job["if"])
        self.assertEqual(job["runs-on"], "ubuntu-26.04")
        self.assertEqual(job["timeout-minutes"], "10")
        self.assertEqual(len(job["steps"]), 2)
        checkout, compare = job["steps"]
        self.assertEqual(checkout, {"uses": "actions/checkout@v4", "with": {
            "fetch-depth": "0", "persist-credentials": "false"}})
        self.assertEqual(compare["timeout-minutes"], "8")
        self.assertEqual(compare["run"].split(), [
            "python3", "-B", "test/benchmark/fps_scalar_return/run.py", "--base-ref",
            "1b8607587c164b013386423ada2298e05a9e4028", "--candidate-ref", '\"$GITHUB_SHA\"',
        ])
        self.assertNotIn("continue-on-error", compare)
        import hashlib
        import runpy
        from pathlib import Path
        root = Path(__file__).resolve().parents[1]
        driver = runpy.run_path(str(root / "test/benchmark/fps_scalar_return/run.py"))
        self.assertEqual(driver["BASE"], "1b8607587c164b013386423ada2298e05a9e4028")
        self.assertEqual(driver["REF"], route.SOURCE_REF)
        self.assertEqual(driver["CASES"], {"add": (32, 300000), "sub": (262144, 128),
                                          "mul": (65536, 512), "div": (4096, 4096)})
        self.assertEqual(driver["PAIRS"], ("AB", "BA", "AB", "BA"))
        self.assertEqual((driver["MAX_COMMANDS"], driver["JOB_SECONDS"], driver["MAX_LOG_BYTES"]),
                         (96, 420, 1024 * 1024))
        self.assertEqual(set(driver["MODES"]), {"generic", "native"})
        self.assertIn("-fno-lto", driver["COMMON"])
        self.assertNotIn("-DNDEBUG", driver["COMMON"])
        import intrinsic_gate
        self.assertEqual(driver["COMMON"], intrinsic_gate.COMMON + [
            "-Wall", "-Wextra", "-Werror", "-Wno-error=free-nonheap-object"])
        self.assertFalse(any(flag.startswith("-Wno-") for flag in intrinsic_gate.COMMON))
        for name, value in driver["FROZEN"].items():
            self.assertEqual(hashlib.sha256((root / driver["SOURCE"] / name).read_bytes()).hexdigest(), value)
            self.assertEqual(route.CONTROL_PATHS.count(driver["SOURCE"] + name), 1)

    def test_fps_comparison_rejects_changed_schedule_context_and_checksums(self):
        import runpy
        from pathlib import Path
        root = Path(__file__).resolve().parents[1]
        driver = runpy.run_path(str(root / "test/benchmark/fps_scalar_return/run.py"))
        checksums = {
            "add": ("13886490060049436654", "13882659635403116537", "13082089982011685829"),
            "sub": ("11207330850518936369", "11130598958178708230", "4508857342948613413"),
            "mul": ("4568921047891222776", "7306674426695528431", "11662797323877304613"),
            "div": ("1482039196620619456", "4719237234168918346", "16645246113738105637"),
        }
        self.assertEqual(driver["CHECKSUMS"], checksums)
        samples = {}
        for operation, (size, repeats) in driver["CASES"].items():
            source, result, aggregate = checksums[operation]
            row = {"type": "public", "operation": operation, "n": size, "repeats": repeats,
                   "total_ns": repeats * 100, "ns_per_call": 100,
                   "source_checksum": source, "result_checksum": result, "aggregate_checksum": aggregate}
            self.assertEqual(driver["validate_row"](json.dumps(row).encode(), operation), row)
            samples[operation] = row
        row = samples["add"]
        for key, value in (("n", 33), ("repeats", 300001), ("type", "internal"),
                           ("operation", "sub"), ("source_checksum", "0"),
                           ("result_checksum", "0"), ("aggregate_checksum", "0"),
                           ("total_ns", 0), ("ns_per_call", float("nan")),
                           ("ns_per_call", row["ns_per_call"] * 2)):
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                driver["validate_row"](json.dumps({**row, key: value}).encode(), "add")
        duplicate = (json.dumps(row)[:-1] + ', "n": 32}').encode()
        with self.assertRaises(ValueError):
            driver["validate_row"](duplicate, "add")
        candidate = "a" * 40
        context = {"GITHUB_EVENT_NAME": "push", "GITHUB_REPOSITORY": "qandori/cp-library",
                   "GITHUB_REF": driver["REF"], "GITHUB_SHA": candidate, "GITHUB_RUN_ATTEMPT": "1"}
        event = {"deleted": False, "ref": driver["REF"], "after": candidate,
                 "repository": {"full_name": "qandori/cp-library"}}
        driver["validate_context"](driver["BASE"], candidate, context, event)
        for key, value in (("GITHUB_EVENT_NAME", "workflow_dispatch"), ("GITHUB_REPOSITORY", "fork/repo"),
                           ("GITHUB_REF", driver["REF"] + "-other"), ("GITHUB_SHA", "b" * 40),
                           ("GITHUB_RUN_ATTEMPT", "2")):
            with self.subTest(key=key), self.assertRaises(ValueError):
                driver["validate_context"](driver["BASE"], candidate, {**context, key: value}, event)
        for changed in ({**event, "deleted": True}, {**event, "after": "b" * 40},
                        {**event, "repository": {"full_name": "fork/repo"}}):
            with self.assertRaises(ValueError):
                driver["validate_context"](driver["BASE"], candidate, context, changed)
        with self.assertRaises(ValueError):
            driver["validate_context"]("b" * 40, candidate, context, event)



if __name__ == "__main__":
    unittest.main()
