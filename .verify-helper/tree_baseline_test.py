import copy
import io
import json
from pathlib import Path
import unittest
import urllib.error
from unittest.mock import patch

import affected_test
import intrinsic_gate
import no_cache_route as route
import tree_baseline as reuse
from affected import SourceTree, source_affected_plan


class TreeBaselineTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit

    def setUp(self):
        affected_test.AffectedVerification.setUp(self)
        self.ref = route.SOURCE_REF
        self.controls = route.CONTROL_PATHS_V2
        for path in self.controls:
            self.write(path, "# Source-only fixture.\n")
        self.write(".verify-helper/no_cache_route.py", "SOURCE_REF = " + repr(self.ref) + "\n"
                   + "CONTROL_PATHS = " + repr(route.CONTROL_PATHS) + "\n"
                   + "CONTROL_PATHS_V2 = CONTROL_PATHS + " + repr(self.controls[len(route.CONTROL_PATHS):]) + "\n")
        self.write("test/unit/dynamic_wavelet_core.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n')
        self.base = self.commit("fixture baseline", bind=False)
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing changed = int;\n")
        self.source = self.commit("fixture source\n\n" + route.TRAILER)
        self.git("checkout", "--quiet", "-B", "main", self.base)
        self.git("merge", "--quiet", "--no-ff", self.source, "-m", "fixture merge\n\n" + route.TRAILER)
        self.main = self.git("rev-parse", "HEAD").decode().strip()
        self.proof = {"runs": {workflow: {"id": index, "attempt": 1}
                               for index, workflow in enumerate(route.BASELINE_JOBS, 1)},
                      "reviewed_source_control_sha256": route.source_hash(SourceTree(self.root, self.source), self.controls)}
        self.plan = source_affected_plan(self.root, self.base, self.source, expected_ref=self.ref)
        self.logs, self.responses = {}, {}
        self.prepare_api()

    def encode_log(self, rows):
        return "\n".join("2026-01-01T00:00:00.000Z " + (json.dumps(row) if isinstance(row, dict) else row)
                         for row in rows) + "\n"

    def selected_rows(self, groups):
        plan = self.plan
        return [{"mode": "no-cache", "base": plan["base"], "head": plan["head"], "source_head": plan["head"],
                 "ref": self.ref, "event": "push", "source_digest": plan["binding"]["source_digest"]},
                {key: value for key, value in plan.items() if key != "state"},
                {"event": "selected_checks_completed", "mode": "affected-no-cache-v1", "head": plan["head"],
                 "plan_sha256": plan["plan_sha256"], "groups": {group: plan[group] for group in groups},
                 "unselected_checks": "not_run", "successful_cache_written": False}]

    def intrinsic_rows(self):
        plan, tree = self.plan, SourceTree(self.root, self.source)
        family = intrinsic_gate.scope_mode(plan)
        modes = intrinsic_gate.family_modes(family)
        count = 2 + len(modes) * (1 + 2 * (len(plan["headers"]) + len(plan["unit"])))
        rows = ["Image: ubuntu-26.04", {"event": "scope", "route": self.selected_rows(())[0], "family": family,
                 "headers": plan["headers"], "units": plan["unit"], "plan_sha256": plan["plan_sha256"],
                 "source_controls": {name: reuse.digest(tree.content(name)) for name in (
                     ".verify-helper/intrinsic_gate.py", ".verify-helper/intrinsic_gate_test.py", ".github/workflows/verify.yml")}},
                {"event": "compiler", "assertions": True, "lto": False,
                 "compiler": {"path": "/usr/bin/x86_64-linux-gnu-g++-15", "sha256": "a" * 64},
                 "cc1plus": {"sha256": "b" * 64}}]
        for index in range(1, count + 1):
            rows.extend([{"event": "command_start", "id": index,
                          "argv": ["/usr/bin/x86_64-linux-gnu-g++-15", "-dumpfullversion"] if index == 1 else ["fixture"]},
                         {"event": "command_end", "id": index, "returncode": 0, "timed_out": False,
                          "output_tail": "15.2.0\n" if index == 1 else ""}])
        for mode in modes:
            rows.extend([{"event": "macro_proof", "mode": mode, "family": family, "gcc": "15.2.0", "assertions": True,
                          "ONLINE_JUDGE": "1", "ATCODER": "1", "flags": intrinsic_gate.mode_flags(mode, family),
                          "isa": {"__AVX2__": "1", "__BMI2__": "1"} if mode == "native" else {}},
                         {"event": "mode_completed", "mode": mode, "header_count": len(plan["headers"]),
                          "unit_count": len(plan["unit"])}])
            for group in ("headers", "unit"):
                for target in plan[group]:
                    rows.append({"event": "binary", "mode": mode, "group": group, "target": target,
                                 "binary": {"sha256": "c" * 64}, "dependencies": {"project_dependencies": [
                                     {"project_path": target, "sha256": reuse.digest(tree.content(target))}]}})
        rows.append({"event": "gate_completed", "head": self.source, "source_head": self.source, "base": self.base,
                     "source_digest": plan["binding"]["source_digest"], "plan_sha256": plan["plan_sha256"],
                     "commands": count, "unaffected_checks": "not_run", "cache_written": False, "artifacts_uploaded": False})
        return rows

    def prepare_api(self):
        main_runs = []
        for workflow, proof in self.proof["runs"].items():
            number, name = proof["id"], route.BASELINE_JOBS[workflow][0]
            run = {"id": number, "run_attempt": 1, "head_sha": self.source,
                   "head_branch": self.ref.removeprefix("refs/heads/"), "event": "push", "path": workflow,
                   "status": "completed", "conclusion": "success", "repository": {"full_name": route.REPOSITORY},
                   "head_repository": {"full_name": route.REPOSITORY}}
            prefix = "/repos/" + route.REPOSITORY + "/actions/runs/" + str(number)
            self.responses[prefix] = run
            self.responses[prefix + "/attempts/1"] = copy.deepcopy(run)
            names = ("Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event") + route.BASELINE_JOBS[workflow][1]
            if name == "verify":
                names += ("Install dependencies without cache", "Install Boost")
            def job(job_name, job_id, steps):
                return {"id": job_id, "run_id": number, "run_attempt": 1, "head_sha": self.source,
                        "head_branch": run["head_branch"], "name": job_name, "status": "completed", "conclusion": "success",
                        "steps": [{"name": step, "status": "completed", "conclusion": "success"} for step in steps]}
            jobs = [job(name, number * 10, names)]
            rows = self.selected_rows(reuse.GROUPS if name == "verify" else ("headers", "unit"))
            if name == "verify":
                rows += ["Image: ubuntu-24.04", "Linux runner 6.0 x86_64 x86_64 x86_64 GNU/Linux",
                         "g++ (Ubuntu 13.3) 13.3.0", "x86_64-linux-gnu", "Ubuntu clang version 18.1.3", "Target: x86_64-pc-linux-gnu"]
                jobs.append(job("intrinsic", number * 10 + 1, ("Set up job", "Run actions/checkout@v4", reuse.INTRINSIC_STEP)))
                self.logs[number * 10 + 1] = self.encode_log(self.intrinsic_rows())
            else:
                rows += ["Image: macos-14-arm64", "Darwin runner 23.6 Darwin Kernel Version arm64",
                         "Apple clang version 15.0.0 (clang-1500.3.9.4)", "Target: arm64-apple-darwin23.6.0"]
            self.logs[number * 10] = self.encode_log(rows)
            self.responses[prefix + "/attempts/1/jobs?per_page=100"] = {"total_count": len(jobs), "jobs": jobs}
            main = {**run, "id": number + 100, "head_sha": self.main, "head_branch": "main", "status": "in_progress", "conclusion": None}
            main_runs.append(main)
            main_path = "/repos/" + route.REPOSITORY + "/actions/runs/" + str(number + 100) + "/attempts/1"
            self.responses[main_path] = copy.deepcopy(main)
            self.responses[main_path + "/jobs?per_page=100"] = {"total_count": 0, "jobs": []}
        self.main_path = "/repos/" + route.REPOSITORY + "/actions/runs?head_sha=" + self.main + "&branch=main&per_page=100"
        self.responses[self.main_path] = {"total_count": len(main_runs), "workflow_runs": main_runs}

    def request(self, path, token):
        self.assertEqual(token, "fixture-token")
        return copy.deepcopy(self.responses[path])

    def qualify(self):
        return reuse.qualify(self.root, {"base": self.main}, self.proof, "fixture-token", self.request,
                             lambda number, token: self.logs[number], route.BASELINE_JOBS)

    def test_direct_identical_tree_source_receipts_qualify_without_claiming_main_execution(self):
        proof = self.qualify()
        self.assertEqual(proof["basis"], "source-push-identical-tree")
        self.assertEqual(len(proof["receipts"]), 3)
        self.assertEqual(proof["main_checks"], "not_claimed_by_source_receipt")
        self.assertEqual(proof["unaffected_current_head_checks"], "not_run")
        self.assertEqual(proof["full_baseline_target_coverage"], "not_independently_established")

    def test_plan_hashes_differ_but_target_fingerprints_match(self):
        first, second = reuse.source_plans(self.root, self.main, self.proof)
        self.assertNotEqual(first["plan_sha256"], second["plan_sha256"])
        self.assertEqual(reuse.coverage(first), reuse.coverage(second))
        self.assertEqual(reuse.coverage(first)["headers"]["targets"], first["headers"])
        changed = copy.deepcopy(first)
        changed["state"]["targets"]["headers"][first["headers"][0]] = "a" * 64
        self.assertNotEqual(reuse.coverage(first), reuse.coverage(changed))

    def test_source_uses_historical_inventory_after_current_benchmark_rename(self):
        old = "test/benchmark/fps_scalar_return/bench.cpp"
        new = "test/benchmark/fps_right_shift/bench.cpp"
        (self.root / new).parent.mkdir(parents=True)
        self.git("mv", old, new)
        self.commit("later benchmark rename", bind=False)
        renamed = tuple(new if path == old else path for path in self.controls)
        with patch.object(route, "CONTROL_PATHS_V2", renamed):
            self.assertEqual(self.qualify()["basis"], "source-push-identical-tree")
        self.assertEqual(reuse.historical_controls(SourceTree(self.root, self.source)), self.controls)

    def test_historical_inventory_rejects_omitted_duplicate_invalid_and_missing_controls(self):
        for paths in (self.controls[1:], self.controls + (self.controls[0],), self.controls + ("../escape",),
                      self.controls + ("missing-control.py",), self.controls + ("/absolute",)):
            with self.subTest(paths=paths):
                self.write(".verify-helper/no_cache_route.py", "CONTROL_PATHS = " + repr(paths)
                           + "\nCONTROL_PATHS_V2 = CONTROL_PATHS + ()\n")
                head = self.commit("invalid historical inventory", bind=False)
                with self.assertRaises(ValueError):
                    reuse.historical_controls(SourceTree(self.root, head))

    def test_historical_inventory_is_ast_only_and_rejects_symlinks(self):
        source = "CONTROL_PATHS = " + repr(self.controls) + "\nCONTROL_PATHS_V2 = CONTROL_PATHS + "
        for extra in ("__import__('os').abort()", "[]"):
            self.write(".verify-helper/no_cache_route.py", source + extra + "\n")
            head = self.commit("unsupported historical declaration", bind=False)
            with self.assertRaises(ValueError):
                reuse.historical_controls(SourceTree(self.root, head))
        self.write(".verify-helper/no_cache_route.py", source + "()\n")
        path = self.root / self.controls[0]
        path.unlink()
        path.symlink_to("target")
        head = self.commit("nonregular historical control", bind=False)
        with self.assertRaisesRegex(ValueError, "regular files"):
            reuse.historical_controls(SourceTree(self.root, head))

    def test_real_chunk_boundary_bom_preserves_strict_timestamp_grammar(self):
        # Unmodified boundary line from PR24 push job 117635743372; raw log stays local.
        boundary = "\ufeff2026-10-18T09:52:32.7908921Z ok 5 tokens"
        self.assertEqual(reuse.log_lines("2026-10-18T09:52:32.0000000Z previous chunk\n" + boundary + "\n"),
                         ["previous chunk", "ok 5 tokens"])
        self.assertEqual(reuse.log_lines(boundary + "\n"), ["ok 5 tokens"])
        for invalid in ("\ufeff" + boundary, "\ufeffuntimestamped output", boundary.replace("2026-", "2026-\ufeff")):
            with self.subTest(invalid=invalid), self.assertRaisesRegex(ValueError, "log format"):
                reuse.log_lines(invalid + "\n")

    def test_squash_and_unreviewed_control_are_rejected_before_api(self):
        with self.assertRaisesRegex(ValueError, "two-parent"):
            reuse.source_plans(self.root, self.source, self.proof)
        with self.assertRaisesRegex(ValueError, "controls differ"):
            reuse.source_plans(self.root, self.main, {**self.proof, "reviewed_source_control_sha256": "f" * 64})

    def test_wrong_source_run_and_stale_attempt_fail(self):
        path = "/repos/" + route.REPOSITORY + "/actions/runs/1"
        for field, value in (("head_sha", self.main), ("head_branch", "main"), ("run_attempt", 2),
                             ("event", "pull_request"), ("status", "in_progress"), ("conclusion", "failure"),
                             ("repository", {"full_name": "fork/cp-library"}), ("path", "other")):
            original = self.responses[path]
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.responses[path] = {**original, field: value}
                self.qualify()
            self.responses[path] = original

    def test_required_steps_and_intrinsic_cannot_be_skipped(self):
        path = "/repos/" + route.REPOSITORY + "/actions/runs/1/attempts/1/jobs?per_page=100"
        original = copy.deepcopy(self.responses[path])
        for index in (0, 1):
            for mode in ("skipped", "failure", "missing", "duplicate"):
                self.responses[path] = copy.deepcopy(original)
                job = self.responses[path]["jobs"][index]
                if mode == "missing":
                    job["steps"].pop()
                elif mode == "duplicate":
                    job["steps"].append(copy.deepcopy(job["steps"][-1]))
                else:
                    job["steps"][-1]["conclusion"] = mode
                with self.subTest(job=index, mode=mode), self.assertRaises(ValueError):
                    self.qualify()
        self.responses[path] = original

    def test_missing_runtime_identity_or_ambiguous_completion_fails(self):
        original = self.logs[10]
        for replacement in (original.replace("g++ (Ubuntu 13.3) 13.3.0", "g++ --version"),
                            original.replace("Target: x86_64-pc-linux-gnu", "Target: aarch64-pc-linux-gnu"),
                            original + self.encode_log([self.selected_rows(reuse.GROUPS)[-1]])):
            self.logs[10] = replacement
            with self.assertRaises(ValueError):
                self.qualify()
        self.logs[10] = original

    def test_wrong_or_incomplete_intrinsic_records_fail(self):
        for event in ("compiler", "gate_completed", "mode_completed", "binary", "command_end", "macro_proof"):
            rows = self.intrinsic_rows()
            index = next(index for index, item in enumerate(rows) if isinstance(item, dict) and item.get("event") == event)
            rows.pop(index)
            self.logs[11] = self.encode_log(rows)
            with self.subTest(event=event), self.assertRaises(ValueError):
                self.qualify()

    def test_actual_main_failure_is_distinct_from_unavailable_source(self):
        self.responses[self.main_path]["workflow_runs"][0]["conclusion"] = "failure"
        with self.assertRaises(reuse.MainFailure):
            self.qualify()

    def test_in_progress_main_with_failed_step_is_a_blocker(self):
        path = "/repos/" + route.REPOSITORY + "/actions/runs/101/attempts/1/jobs?per_page=100"
        self.responses[path] = {"total_count": 1, "jobs": [{"id": 99, "conclusion": None,
            "steps": [{"conclusion": "failure"}]}]}
        with self.assertRaises(reuse.MainFailure):
            self.qualify()

    def test_older_main_attempt_failure_blocks_later_green_summary(self):
        listed = self.responses[self.main_path]["workflow_runs"][0]
        listed.update(run_attempt=2, status="completed", conclusion="success")
        path = "/repos/" + route.REPOSITORY + "/actions/runs/101/attempts/1"
        self.responses[path]["conclusion"] = "failure"
        with self.assertRaises(reuse.MainFailure):
            self.qualify()

    def test_visible_failure_survives_prior_malformed_conclusion(self):
        listed = self.responses[self.main_path]["workflow_runs"]
        listed[0]["conclusion"] = []
        listed[1]["conclusion"] = "failure"
        with self.assertRaises(reuse.MainFailure):
            self.qualify()

    def test_main_enumeration_and_job_pagination_fail_closed(self):
        self.responses[self.main_path]["total_count"] += 1
        with self.assertRaisesRegex(ValueError, "Incomplete main"):
            self.qualify()


class LogTransportTests(unittest.TestCase):
    def redirect(self, target):
        def opened(request, timeout):
            self.assertEqual(request.host, "api.github.com")
            self.assertEqual(request.get_header("Authorization"), "Bearer fixture-token")
            raise urllib.error.HTTPError(request.full_url, 302, "redirect", {"Location": target}, io.BytesIO())
        return opened

    def test_only_new_credential_free_request_follows_verified_log_host(self):
        target = "https://productionresultssa0.blob.core.windows.net/actions-results/fixture/log?sig=fixture"
        def public(request, timeout):
            self.assertEqual(request.full_url, target)
            self.assertIsNone(request.get_header("Authorization"))
            return io.BytesIO(b"fixture log")
        self.assertEqual(reuse.job_log(1, "fixture-token", api_open=self.redirect(target), public_open=public), "fixture log")

    def test_bad_log_destinations_are_not_requested(self):
        for target in ("http://productionresultssa0.blob.core.windows.net/actions-results/log",
                       "https://attacker.invalid/log", "https://productionresultssa0.blob.core.windows.net.evil.invalid/actions-results/log",
                       "https://user@productionresultssa0.blob.core.windows.net/actions-results/log",
                       "https://productionresultssa0.blob.core.windows.net:443/actions-results/log",
                       "https://productionresultssa0.blob.core.windows.net/unrelated/log"):
            with self.subTest(target=target), self.assertRaises(ValueError):
                reuse.job_log(1, "fixture-token", api_open=self.redirect(target),
                              public_open=lambda *args, **kwargs: self.fail("Unverified host was requested"))

    def test_size_limit_and_redirect_chains_fail(self):
        with patch.object(reuse, "LOG_LIMIT", 4), self.assertRaises(ValueError):
            reuse.job_log(1, "fixture-token", api_open=lambda *args, **kwargs: io.BytesIO(b"12345"))
        self.assertIsNone(reuse.NoRedirect().redirect_request(None, None, None, None, None, None))


class VersionTwoEvidenceTests(unittest.TestCase):
    import no_cache_route_test as fixture
    git = fixture.NoCacheRouteTests.git
    write = fixture.NoCacheRouteTests.write
    commit = fixture.NoCacheRouteTests.commit
    push = fixture.NoCacheRouteTests.push
    selected = fixture.NoCacheRouteTests.selected
    api = fixture.NoCacheRouteTests.api

    def setUp(self):
        self.fixture.NoCacheRouteTests.setUp(self)
        for name in set(route.CONTROL_PATHS_V2) - set(route.CONTROL_PATHS):
            self.write(name, "# Fixture control.\n")
        self.head = self.commit("fixture v2 controls\n\n" + route.TRAILER)
        evidence = json.loads((self.root / route.EVIDENCE_PATH).read_text())
        evidence.update(version=2, policy=reuse.POLICY,
                        reviewed_control_sha256=route.source_hash(SourceTree(self.root, self.head), route.CONTROL_PATHS_V2),
                        qualification={"kind": "source-push", "runs": copy.deepcopy(evidence["runs"]),
                                       "reviewed_source_control_sha256": "a" * 64})
        self.write(route.EVIDENCE_PATH, (reuse.canonical(evidence) + b"\n").decode())
        self.head = self.commit("fixture v2 evidence\n\n" + route.TRAILER)

    def test_unavailable_source_evidence_uses_original_exact_main_receipts(self):
        with patch.object(reuse, "check_main", return_value=[]), patch.object(reuse, "qualify", side_effect=ValueError("unavailable")):
            result = route.verify_baseline(self.root, self.selected(), "fixture-token", self.api)
        self.assertEqual(len(result["receipts"]), 2)
        self.assertNotIn("basis", result)
        self.assertEqual(result["full_baseline_target_coverage"], "not_independently_established")

    def test_known_main_failure_is_never_swallowed_by_fallback(self):
        with patch.object(reuse, "check_main", side_effect=reuse.MainFailure("known failure")), self.assertRaises(reuse.MainFailure):
            route.verify_baseline(self.root, self.selected(), "fixture-token", self.api)

    def test_visible_failure_in_incomplete_main_run_list_blocks_green_fallback(self):
        def api(path, token):
            if "/runs?" in path:
                return {"total_count": 101, "workflow_runs": [{"id": 999, "head_sha": self.base, "head_branch": "main",
                        "event": "push", "conclusion": "failure"}]}
            return self.api(path, token)
        with self.assertRaises(reuse.MainFailure):
            route.verify_baseline(self.root, self.selected(), "fixture-token", api)

    def test_visible_failure_in_incomplete_main_jobs_blocks_green_fallback(self):
        workflows = list(route.BASELINE_JOBS)
        runs = [{"id": 100 + index, "run_attempt": 1, "head_sha": self.base, "head_branch": "main", "event": "push",
                 "path": workflow, "status": "in_progress", "conclusion": None, "repository": {"full_name": route.REPOSITORY},
                 "head_repository": {"full_name": route.REPOSITORY}} for index, workflow in enumerate(workflows)]
        def api(path, token):
            if "/runs?" in path:
                return {"total_count": len(runs), "workflow_runs": runs}
            if "/runs/100/attempts/1" in path:
                if "/jobs?" in path:
                    return {"total_count": 101, "jobs": [{"id": 999, "steps": None},
                            {"id": 1000, "steps": [{"conclusion": "failure"}]}]}
                return runs[0]
            return self.api(path, token)
        with self.assertRaises(reuse.MainFailure):
            route.verify_baseline(self.root, self.selected(), "fixture-token", api)

    def test_extra_v2_control_change_fails_before_api(self):
        self.write(".verify-helper/tree_baseline.py", "changed qualification\n")
        self.head = self.commit("changed v2 control\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "CI control"):
            route.verify_baseline(self.root, self.selected(), "fixture-token", lambda *args: self.fail("API called"))


if __name__ == "__main__":
    unittest.main()
