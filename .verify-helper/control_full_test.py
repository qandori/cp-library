import contextlib
import copy
import io
import json
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

import affected_test
import ci
import control_full as control
import no_cache_route as route
import tree_baseline as reuse
from affected import SourceTree, canonical, git


class ControlFullTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit

    def setUp(self):
        affected_test.AffectedVerification.setUp(self)
        outer = Path(__file__).resolve().parent.parent
        self.workflows = (".github/workflows/verify.yml", ".github/workflows/macos-portability.yml")
        for path in self.workflows:
            self.write(path, git(outer, "show", control.BASE + ":" + path).decode())
        self.base = self.commit("control fixture baseline", bind=False)
        self.addCleanup(patch.stopall)
        patch.object(control, "BASE", self.base).start()
        patch.object(ci, "environment_key", lambda: "fixture environment").start()
        for path in self.workflows:
            self.write(path, (outer / path).read_text())
        self.write(".verify-helper/control_full.py", "BASE = " + repr(self.base) + "\nREF = " + repr(control.REF) + "\n")
        self.head = self.commit("control fixture\n\n" + control.TRAILER, bind=False)

    def push(self, head=None, ref=None, before=None):
        head, ref = head or self.head, ref or control.REF
        return ({"ref": ref, "after": head, "before": before or self.base, "deleted": False},
                {"GITHUB_EVENT_NAME": "push", "GITHUB_REPOSITORY": route.REPOSITORY, "GITHUB_SHA": head, "GITHUB_REF": ref})

    def merge(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        self.git("merge", "--quiet", "--no-ff", self.head, "-m", "control merge\n\n" + control.TRAILER)
        return self.git("rev-parse", "HEAD").decode().strip()

    def cli(self, *args):
        with patch.object(ci, "ROOT", self.root), patch.object(sys, "argv", ["ci.py", *args]), \
                patch.dict(os.environ, {"VERIFY_NO_CACHE": "true", "VERIFY_CONTROL_FULL": "true"}), \
                contextlib.redirect_stdout(io.StringIO()) as stream:
            ci.main()
            return stream.getvalue()

    def test_exact_control_source_forces_full_distinct_route(self):
        selected = route.select_event(self.root, *self.push())
        self.assertEqual(selected["mode"], "control-full")
        self.assertEqual(selected["base"], self.base)
        self.assertEqual(selected["ref"], control.REF)
        plan = control.build_plan(self.root, self.head)
        self.assertEqual({group: plan[group] for group in ci.GROUPS}, SourceTree(self.root, self.head).targets())
        self.assertTrue(plan["full"])
        self.assertFalse(plan["cache_reads"])
        self.assertFalse(plan["cache_writes"])

    def test_control_branch_without_marker_does_not_enter_normal_cache_route(self):
        self.head = self.commit("no marker", bind=False)
        with self.assertRaisesRegex(ValueError, "explicit marker"):
            route.select_event(self.root, *self.push())

    def test_product_unit_judge_intrinsic_binding_and_config_changes_are_rejected(self):
        original = self.head
        for name in ("math/unrelated.hpp", "test/unit/unrelated.cpp", "test/unrelated.test.cpp",
                     ".verify-helper/intrinsic_gate.py", ".verify-helper/affected-binding.json", ".verify-helper/config.toml"):
            self.git("checkout", "--quiet", original)
            self.write(name, "unexpected source\n")
            self.head = self.commit("changed source\n\n" + control.TRAILER, bind=False)
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "non-control"):
                route.select_event(self.root, *self.push())

    def test_check_flags_and_intrinsic_job_must_not_change(self):
        original = self.head
        for before, after in (("--sanitize --selection", "--sanitize --extra --selection"),
                              ("timeout-minutes: 20", "timeout-minutes: 21")):
            self.git("checkout", "--quiet", original)
            path = self.root / ".github/workflows/verify.yml"
            path.write_text(path.read_text().replace(before, after))
            self.head = self.commit("changed command\n\n" + control.TRAILER, bind=False)
            with self.assertRaises(ValueError):
                route.select_event(self.root, *self.push())

    def test_exact_main_and_pr_merge_are_required(self):
        merged = self.merge()
        selected = route.select_event(self.root, *self.push(merged, route.MAIN_REF))
        self.assertEqual(selected["source_head"], self.head)
        with self.assertRaises(ValueError):
            route.select_event(self.root, *self.push(merged, route.MAIN_REF, "f" * 40))
        event = {"number": 17, "pull_request": {
            "head": {"sha": self.head, "ref": control.REF.removeprefix("refs/heads/"), "repo": {"full_name": route.REPOSITORY}},
            "base": {"sha": self.base, "ref": "main", "repo": {"full_name": route.REPOSITORY}}}}
        context = {"GITHUB_EVENT_NAME": "pull_request", "GITHUB_REPOSITORY": route.REPOSITORY, "GITHUB_SHA": merged,
                   "GITHUB_REF": "refs/pull/17/merge", "GITHUB_HEAD_REF": control.REF.removeprefix("refs/heads/")}
        self.assertEqual(route.select_event(self.root, event, context)["mode"], "control-full")
        event["pull_request"]["head"]["repo"]["full_name"] = "fork/cp-library"
        with self.assertRaises(ValueError):
            route.select_event(self.root, event, context)

    def test_unmarked_control_squash_cannot_enable_normal_storage(self):
        tree = self.git("rev-parse", self.head + "^{tree}").decode().strip()
        self.head = self.git("commit-tree", tree, "-p", self.base, "-m", "unmarked control squash").decode().strip()
        self.git("reset", "--hard", self.head)
        with self.assertRaisesRegex(ValueError, "explicit direct control route"):
            route.select_event(self.root, *self.push(self.head, route.MAIN_REF))

    def test_unmarked_control_tip_cannot_fast_forward_main_into_normal_storage(self):
        self.head = self.commit("unmarked control tip", bind=False)
        with self.assertRaisesRegex(ValueError, "explicit direct control route"):
            route.select_event(self.root, *self.push(self.head, route.MAIN_REF))

    def test_unapproved_workflow_steps_env_or_storage_conditions_are_rejected(self):
        original = self.head
        for before, after in (("permissions:\n", "env:\n  CXXFLAGS: -DNDEBUG\npermissions:\n"),
                              ("if: steps.route.outputs.mode == 'normal'", "if: always()"),
                              ("          persist-credentials: false", "          persist-credentials: true")):
            self.git("checkout", "--quiet", original)
            path = self.root / ".github/workflows/verify.yml"
            path.write_text(path.read_text().replace(before, after, 1))
            self.head = self.commit("changed workflow execution\n\n" + control.TRAILER, bind=False)
            with self.assertRaisesRegex(ValueError, "unapproved workflow"):
                route.select_event(self.root, *self.push())

    def test_control_merge_tree_drift_and_swapped_parents_fail(self):
        merged = self.merge()
        self.write(".verify-helper/ci.py", "allowed control drift at merge only\n")
        self.git("add", "--all")
        self.git("commit", "--quiet", "--amend", "--no-edit")
        drifted = self.git("rev-parse", "HEAD").decode().strip()
        with self.assertRaisesRegex(ValueError, "exact direct merge"):
            route.select_event(self.root, *self.push(drifted, route.MAIN_REF))
        tree = self.git("rev-parse", merged + "^{tree}").decode().strip()
        swapped = self.git("commit-tree", tree, "-p", self.head, "-p", self.base,
                           "-m", "swapped merge\n\n" + control.TRAILER).decode().strip()
        self.git("reset", "--hard", swapped)
        with self.assertRaisesRegex(ValueError, "exact direct merge"):
            route.select_event(self.root, *self.push(swapped, route.MAIN_REF))

    def test_wrong_ref_event_dirty_checkout_and_untracked_source_fail(self):
        event, context = self.push()
        for change in ({"GITHUB_REF": "refs/heads/unrelated"}, {"GITHUB_EVENT_NAME": "workflow_dispatch"}):
            with self.assertRaises(ValueError):
                route.select_event(self.root, event, {**context, **change})
        self.write(".verify-helper/ci.py", "untracked source\n")
        with self.assertRaises(ValueError):
            route.select_event(self.root, event, context)

    def test_full_cli_ignores_and_never_writes_success_state(self):
        state = self.root / ".verify-helper/verified-state.json"
        state.write_text('{"sentinel":"not an input"}\n')
        before = state.read_bytes()
        text = self.cli("plan", "--control-full", "--expected-head", self.head)
        plan = json.loads((self.root / ".verify-helper/selection.json").read_text())
        self.assertIn('"mode": "control-full-no-cache-v1"', text)
        self.assertEqual(plan["headers"], SourceTree(self.root, self.head).targets()["headers"])
        record = json.loads(self.cli("record", "--groups", "headers", "unit"))
        self.assertEqual(record["event"], "control_full_checks_completed")
        self.assertEqual(record["intrinsic_checks"], "not_run_control_only")
        self.assertEqual(state.read_bytes(), before)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.cli("save")
        self.assertEqual(state.read_bytes(), before)

    def test_full_record_revalidates_selection_and_current_source(self):
        self.cli("plan", "--control-full", "--expected-head", self.head)
        path = self.root / ".verify-helper/selection.json"
        plan = json.loads(path.read_text())
        plan["headers"].pop()
        path.write_text(json.dumps(plan))
        with self.assertRaisesRegex(ValueError, "provenance changed"):
            self.cli("record", "--groups", "headers")

    def test_full_scope_does_not_reuse_judge_markers(self):
        plan = control.build_plan(self.root, self.head)
        marker = affected_test.Marker()
        marker.verified.update(plan["verification"])
        def run(paths):
            for path in paths:
                marker.mark_verified(path)
            return affected_test.Summary()
        ci.verify_selected(plan, marker, run)
        self.assertEqual(marker.failed_calls, plan["verification"])
        self.assertEqual(marker.verified_calls, plan["verification"])

    def test_storage_and_deployment_stay_normal_only(self):
        import yaml
        for path, name in ((self.workflows[0], "verify"), (self.workflows[1], "portable")):
            workflow = yaml.load((self.root / path).read_text(), Loader=yaml.BaseLoader)
            for step in workflow["jobs"][name]["steps"]:
                if step.get("uses", "").startswith(("actions/cache", "actions/upload-pages-artifact")):
                    self.assertIn("steps.route.outputs.mode == 'normal'", step["if"])
            selector = next(step for step in workflow["jobs"][name]["steps"]
                            if step.get("name") == "Select exact-head full scope without cache")
            self.assertEqual(selector["if"], "steps.route.outputs.mode == 'control-full'")
            self.assertIn("--control-full", selector["run"])
            if name == "verify":
                self.assertIn("needs.verify.outputs.mode == 'no-cache'", workflow["jobs"]["intrinsic"]["if"])
                self.assertIn("needs.verify.outputs.mode == 'normal'", workflow["jobs"]["deploy"]["if"])

    def bootstrap_data(self):
        merged = self.merge()
        plan = control.build_plan(self.root, merged)
        controls = (*self.workflows, ".verify-helper/control_full.py")
        evidence = {"runs": {}, "qualification": {"kind": "control-full", "reviewed_source_control_sha256":
                    route.source_hash(SourceTree(self.root, merged), controls)}}
        responses, logs, runs = {}, {}, []
        for number, (workflow, (name, step_names)) in enumerate(route.BASELINE_JOBS.items(), 1):
            evidence["runs"][workflow] = {"id": number, "attempt": 1}
            run = {"id": number, "run_attempt": 1, "head_sha": merged, "head_branch": "main", "event": "push", "path": workflow,
                   "status": "completed", "conclusion": "success", "repository": {"full_name": route.REPOSITORY},
                   "head_repository": {"full_name": route.REPOSITORY}}
            runs.append(run)
            prefix = "/repos/" + route.REPOSITORY + "/actions/runs/" + str(number)
            responses[prefix] = run
            responses[prefix + "/attempts/1"] = run
            steps = tuple("Select exact-head full scope without cache" if step == "Select exact-head affected scope without cache"
                          else step for step in step_names)
            steps += ("Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event")
            if name == "verify":
                steps += ("Install dependencies without cache", "Install Boost")
            job = {"id": number * 10, "run_id": number, "run_attempt": 1, "head_sha": merged, "head_branch": "main",
                   "name": name, "status": "completed", "conclusion": "success",
                   "steps": [{"name": step, "status": "completed", "conclusion": "success"} for step in steps]}
            jobs = [job]
            if name == "verify":
                jobs.append({"id": 11, "name": "intrinsic", "status": "completed", "conclusion": "skipped", "steps": []})
            responses[prefix + "/attempts/1/jobs?per_page=100"] = {"total_count": len(jobs), "jobs": jobs}
            rows = [{key: value for key, value in plan.items() if key != "state"},
                    control.completed(plan, ci.GROUPS if name == "verify" else ("headers", "unit"))]
            rows += (["Image: ubuntu-24.04", "Linux runner 6.0 x86_64 x86_64 x86_64 GNU/Linux", "g++ (Ubuntu 13.3) 13.3.0",
                      "x86_64-linux-gnu", "Ubuntu clang version 18.1.3", "Target: x86_64-pc-linux-gnu"] if name == "verify" else
                     ["Image: macos-14-arm64", "Darwin runner 23.6 Darwin Kernel Version arm64",
                      "Apple clang version 15.0.0 (clang-1500.3.9.4)", "Target: arm64-apple-darwin23.6.0"])
            logs[number * 10] = "\n".join("2026-01-01T00:00:00.000Z " + (json.dumps(row) if isinstance(row, dict) else row)
                                          for row in rows) + "\n"
        query = "/repos/" + route.REPOSITORY + "/actions/runs?head_sha=" + merged + "&branch=main&per_page=100"
        responses[query] = {"total_count": len(runs), "workflow_runs": runs}
        return merged, evidence, responses, logs, controls

    def test_actual_main_full_bootstrap_explicitly_does_not_claim_intrinsic(self):
        merged, evidence, responses, logs, controls = self.bootstrap_data()
        result = reuse.bootstrap(self.root, {"base": merged}, evidence, "fixture-token",
                                 lambda path, token: copy.deepcopy(responses[path]), lambda number, token: logs[number],
                                 controls, route.BASELINE_JOBS)
        self.assertEqual(result["basis"], "control-full-bootstrap")
        self.assertEqual(result["ordinary_full_workflows"], "executed")
        self.assertEqual(result["intrinsic_checks"], "not_run_control_only")

    def test_bootstrap_requires_every_completed_group(self):
        merged, evidence, responses, logs, controls = self.bootstrap_data()
        logs[10] = "\n".join(line for line in logs[10].splitlines() if "control_full_checks_completed" not in line)
        with self.assertRaisesRegex(ValueError, "completed groups"):
            reuse.bootstrap(self.root, {"base": merged}, evidence, "fixture-token",
                            lambda path, token: copy.deepcopy(responses[path]), lambda number, token: logs[number],
                            controls, route.BASELINE_JOBS)


if __name__ == "__main__":
    unittest.main()
