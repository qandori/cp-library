import contextlib
import copy
import hashlib
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import ci
from affected import (BINDING_PATH, MODE, SourceTree, binding_source_digest, build_affected_plan,
                      canonical, read_binding, record_affected_success, validate_affected_plan,
                      verify_push_binding)


class Marker:
    def __init__(self):
        self.verified, self.failed_calls, self.verified_calls = set(), [], []

    def mark_failed(self, path):
        name = path.as_posix()
        self.failed_calls.append(name)
        self.verified.discard(name)

    def mark_verified(self, path):
        name = path.as_posix()
        self.verified_calls.append(name)
        self.verified.add(name)

    def is_verified(self, path):
        return path.as_posix() in self.verified


class Summary:
    def __init__(self, success=True):
        self.success = success

    def show(self):
        pass

    def succeeded(self):
        return self.success


class AffectedVerification(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix="affected-ci-test-")
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.addCleanup(os.chdir, Path.cwd())
        environment = {key: value for key, value in os.environ.items() if not key.startswith("GIT_")}
        environment.update(HOME=str(self.root), GIT_CONFIG_NOSYSTEM="1",
                           GIT_CONFIG_GLOBAL=os.devnull, GIT_CONFIG_SYSTEM=os.devnull,
                           GIT_TERMINAL_PROMPT="0", GIT_AUTHOR_NAME="Fixture",
                           GIT_AUTHOR_EMAIL="fixture@example.invalid", GIT_COMMITTER_NAME="Fixture",
                           GIT_COMMITTER_EMAIL="fixture@example.invalid",
                           GIT_AUTHOR_DATE="2020-01-01T00:00:00+00:00",
                           GIT_COMMITTER_DATE="2020-01-01T00:00:00+00:00")
        environment.pop("VERIFY_FULL", None)
        environment.pop("VERIFY_NO_CACHE", None)
        environment.pop("GITHUB_STEP_SUMMARY", None)
        environment.pop("GITHUB_REF", None)
        environment.pop("GITHUB_HEAD_REF", None)
        environment_patch = patch.dict(os.environ, environment, clear=True)
        environment_patch.start()
        self.addCleanup(environment_patch.stop)
        self.git("init", "--quiet")
        self.write(".gitignore", ".verify-helper/selection.json\n.verify-helper/verified-state.json\n.verify-helper/timestamps.*.json\n__pycache__/\n")
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\n")
        self.write("data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp", '#include "../../components/dynamic_bit_vector.hpp"\n')
        self.write("math/unrelated.hpp", "#pragma once\n")
        self.write("test/unit/dynamic_bit_vector.cpp", '#include "components/dynamic_bit_vector.hpp"\n')
        for name in ("dynamic_wavelet_count", "dynamic_wavelet_matrix", "standalone"):
            self.write("test/unit/" + name + ".cpp", '#include "data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"\n')
        self.write("test/unit/unrelated.cpp", '#include "math/unrelated.hpp"\n')
        for name in ("freq", "kth"):
            self.write("test/dynamic_wavelet_matrix_" + name + ".test.cpp",
                       '#include "../data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"\n')
        self.write("test/unrelated.test.cpp", '#include "../math/unrelated.hpp"\n')
        self.write("test/verify.py", "# Fixture runner, never executed.\n")
        self.ref = "refs/heads/codex/dwm-neighbor-20261017"
        self.base = self.commit("baseline")

    def git(self, *arguments):
        return subprocess.check_output(["git", "-c", "core.hooksPath=" + os.devnull,
                                        "-c", "maintenance.autoDetach=false", "-c", "gc.autoDetach=false",
                                        "-C", str(self.root), *arguments], stderr=subprocess.PIPE)

    def write(self, name, source):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source)

    def commit(self, message, bind=True):
        self.git("add", "--all")
        self.git("commit", "--quiet", "--allow-empty", "-m", message)
        head = self.git("rev-parse", "HEAD").decode().strip()
        if bind and hasattr(self, "base"):
            # Synthetic fixture authoring only. Production verification never writes a binding.
            binding = {"version": 1, "base": self.base, "ref": self.ref,
                       "source_digest": binding_source_digest(SourceTree(self.root, self.base),
                                                              SourceTree(self.root, head), self.ref)}
            self.write(BINDING_PATH, (canonical(binding) + b"\n").decode())
            self.git("add", BINDING_PATH)
            self.git("commit", "--quiet", "--amend", "--allow-empty", "--no-edit")
        return self.git("rev-parse", "HEAD").decode().strip()

    def change_bits(self):
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing value = int;\n")
        return self.commit("change bits")

    def plan(self, base=None, head=None, **kwargs):
        return build_affected_plan(self.root, base or self.base,
                                   head or self.git("rev-parse", "HEAD").decode().strip(),
                                   expected_ref=self.ref, **kwargs)

    def main(self, *arguments):
        output = io.StringIO()
        if "--affected-base" in arguments:
            arguments = (*arguments, "--expected-ref", self.ref)
        with patch.object(ci, "ROOT", self.root), patch.object(sys, "argv", ["ci.py", *arguments]), \
                patch.object(ci, "environment_key", return_value="fixture"), contextlib.redirect_stdout(output):
            ci.main()
        return output.getvalue()

    def test_fixture_git_waits_for_maintenance(self):
        for name in ("maintenance.autoDetach", "gc.autoDetach"):
            self.assertEqual(self.git("config", "--get", name), b"false\n")

    def test_sequence_header_scope(self):
        self.write("sequence/lis.hpp", "#pragma once\n")
        self.write("test/unit/lis.cpp", '#include "sequence/lis.hpp"\n')
        self.write("test/lis.test.cpp", '#include "../sequence/lis.hpp"\n')
        self.commit("add sequence library")
        plan = self.plan()
        self.assertEqual(plan["headers"], ["sequence/lis.hpp"])
        self.assertEqual(plan["unit"], ["test/unit/lis.cpp"])
        self.assertEqual(plan["verification"], ["test/lis.test.cpp"])

    def test_cold_dynamic_scope_and_incompatible_cache_inventory(self):
        self.change_bits()
        plan = self.plan()
        self.assertEqual(plan["headers"], ["components/dynamic_bit_vector.hpp", "data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"])
        self.assertEqual(plan["bundles"], plan["headers"])
        self.assertEqual(plan["unit"], ["test/unit/dynamic_bit_vector.cpp", "test/unit/dynamic_wavelet_count.cpp",
                                        "test/unit/dynamic_wavelet_matrix.cpp", "test/unit/standalone.cpp"])
        self.assertEqual(plan["sanitize"], plan["unit"])
        self.assertEqual(plan["verification"], ["test/dynamic_wavelet_matrix_freq.test.cpp", "test/dynamic_wavelet_matrix_kth.test.cpp"])
        self.assertIsNone(ci.cached_targets(plan["state"]))
        self.assertFalse((self.root / ".verify-helper/verified-state.json").exists())
        self.assertIn("test/unit/unrelated.cpp", plan["state"]["targets"]["unit"])

    def test_package_only_change_does_not_invent_source_scope(self):
        self.write("test/benchmark/package/candidate.json", '{"candidate":"derived-only"}\n')
        self.commit("package-only experiment")
        plan = self.plan()
        self.assertTrue(all(plan[group] == [] for group in ci.GROUPS))
        self.assertIn("test/benchmark/package/candidate.json", plan["reviewed_extra_paths"])
        self.assertEqual(len(plan["state"]["targets"]["unit"]), 5)

    def test_cumulative_selection_and_exact_binding_after_helper_only_commit(self):
        first = self.change_bits()
        self.write(".verify-helper/new-helper.py", "# Reviewed fixture infrastructure.\n")
        head = self.commit("helper only")
        plan = self.plan()
        self.assertEqual(len(plan["unit"]), 4)
        self.assertEqual(plan["parent"], [first])
        expected = binding_source_digest(SourceTree(self.root, self.base), SourceTree(self.root, head), self.ref)
        self.assertEqual(plan["binding"]["source_digest"], expected)
        self.assertIn(".verify-helper/new-helper.py", plan["reviewed_extra_paths"])
        with self.assertRaisesRegex(ValueError, "Binding baseline"):
            self.plan(base=first)
        self.write(".verify-helper/new-helper.py", "# Unreviewed altered infrastructure.\n")
        self.commit("unreviewed change", bind=False)
        with self.assertRaisesRegex(ValueError, "source digest mismatch"):
            self.plan()

    def test_wrong_missing_and_abbreviated_commit_ids(self):
        head = self.change_bits()
        with self.assertRaisesRegex(ValueError, "not the expected head"):
            self.plan(head=self.base)
        for base, expected in (("f" * 40, head), (self.base, "f" * 40), (self.base[:12], head), (self.base, head[:12])):
            with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                self.plan(base=base, head=expected)

    def test_nonancestor_base(self):
        head = self.change_bits()
        self.git("checkout", "--quiet", "--orphan", "unrelated-history")
        self.write("math/unrelated.hpp", "#pragma once\nusing unrelated = long;\n")
        other = self.commit("unrelated root")
        self.git("checkout", "--quiet", head)
        with self.assertRaises(subprocess.CalledProcessError):
            self.plan(base=other, head=head)

    def test_tracked_index_and_untracked_changes_rejected(self):
        self.change_bits()
        self.write("untracked.txt", "unexpected\n")
        with self.assertRaisesRegex(ValueError, "untracked"):
            self.plan()
        (self.root / "untracked.txt").unlink()
        self.write("math/unrelated.hpp", "dirty\n")
        with self.assertRaisesRegex(ValueError, "Tracked checkout"):
            self.plan()
        self.git("add", "math/unrelated.hpp")
        with self.assertRaisesRegex(ValueError, "Tracked checkout"):
            self.plan()

    def test_old_new_closures_and_transitive_changes(self):
        self.write("components/detail.hpp", "#pragma once\n")
        self.write("components/dynamic_bit_vector.hpp", '#include "detail.hpp"\n')
        self.base = self.commit("add transitive input")
        self.write("components/detail.hpp", "#pragma once\nusing detail = int;\n")
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\n")
        head = self.commit("remove old edge and change old input")
        plan = self.plan()
        why = plan["scope_reasons"]["unit"]["test/unit/dynamic_wavelet_matrix.cpp"]["changed_dependencies"]
        self.assertIn("components/detail.hpp", why)
        self.assertNotIn("components/detail.hpp", SourceTree(self.root, head).closure("test/unit/dynamic_wavelet_matrix.cpp"))
        self.assertEqual(len(plan["unit"]), 4)

    def test_new_local_shadow_is_selected(self):
        self.write("test/unit/data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp", "#pragma once\n")
        self.commit("introduce quoted include shadow")
        plan = self.plan()
        self.assertEqual(plan["headers"], [])
        self.assertEqual(plan["verification"], [])
        self.assertEqual(plan["unit"], ["test/unit/dynamic_wavelet_count.cpp", "test/unit/dynamic_wavelet_matrix.cpp",
                                        "test/unit/standalone.cpp"])

    def test_deleted_dependency_and_deleted_driver(self):
        self.write("data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp", '#include <components/dynamic_bit_vector.hpp>\n')
        self.base = self.commit("root-relative dependency")
        (self.root / "components/dynamic_bit_vector.hpp").unlink()
        (self.root / "test/dynamic_wavelet_matrix_freq.test.cpp").unlink()
        self.commit("remove dependency and driver")
        plan = self.plan()
        self.assertEqual(plan["headers"], ["data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"])
        self.assertEqual(plan["verification"], ["test/dynamic_wavelet_matrix_kth.test.cpp"])
        self.assertEqual(len(plan["unit"]), 4)
        self.assertIn("test/dynamic_wavelet_matrix_freq.test.cpp", plan["changed_paths"])

    def test_added_unit_and_judge_and_include_cycle(self):
        self.write("test/unit/added.cpp", '#include "components/dynamic_bit_vector.hpp"\n')
        self.write("test/added.test.cpp", '#include "../components/dynamic_bit_vector.hpp"\n')
        self.write("components/dynamic_bit_vector.hpp", '#include <data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp>\n')
        self.commit("add drivers and cycle")
        plan = self.plan()
        self.assertIn("test/unit/added.cpp", plan["unit"])
        self.assertIn("test/added.test.cpp", plan["verification"])
        self.assertEqual(len(plan["headers"]), 2)

    def test_uncertain_include_is_never_skipped(self):
        self.write("components/dynamic_bit_vector.hpp", "#include HEADER\n")
        self.base = self.commit("uncertain include")
        head = self.commit("rebind unchanged uncertain source")
        plan = self.plan(head=head)
        self.assertEqual(len(plan["unit"]), 4)
        self.assertTrue(plan["scope_reasons"]["headers"]["components/dynamic_bit_vector.hpp"]["uncertain"])

    def test_symlink_prefix_even_before_parent_traversal_is_rejected(self):
        self.write("components/detail/leaf.hpp", "#pragma once\n")
        (self.root / "components/link").symlink_to("detail", target_is_directory=True)
        self.write("components/dynamic_bit_vector.hpp", '#include "link/../detail/leaf.hpp"\n')
        self.commit("unsafe include path")
        with self.assertRaisesRegex(ValueError, "Non-directory include path component"):
            self.plan()

    def test_full_precedence_and_configuration_change(self):
        self.write("test/verify.py", "# Changed fixture runner, never executed.\n")
        head = self.commit("runner changed")
        with self.assertRaisesRegex(ValueError, "Check implementation/configuration changed"):
            self.plan()
        plan = self.plan(force=True)
        for group in ci.GROUPS:
            self.assertEqual(plan[group], sorted(plan["state"]["targets"][group]))
        for option in ("flag", "environment"):
            with patch.dict(os.environ, {"VERIFY_FULL": "true" if option == "environment" else "false"}):
                extra = ["--full"] if option == "flag" else []
                self.main("plan", "--affected-base", self.base, "--expected-head", head, *extra)
                result = json.loads((self.root / ".verify-helper/selection.json").read_text())
                self.assertTrue(result["full"])
                self.assertEqual(len(result["unit"]), 5)

    def test_matrix_prefix_keeps_exact_binding_validation(self):
        self.ref = "refs/heads/codex/matrix-centered-20261018"
        self.change_bits()
        self.assertEqual(self.plan()["expected_ref"], self.ref)
        binding = read_binding(SourceTree(self.root, self.git("rev-parse", "HEAD").decode().strip()))
        for ref in ("refs/heads/codex/other-task", "refs/heads/codex/matrix-",
                    "refs/heads/codex/matrix-a..b", "refs/heads/codex/matrix-a/b"):
            self.write(BINDING_PATH, (canonical({**binding, "ref": ref}) + b"\n").decode())
            head = self.commit("invalid binding prefix", bind=False)
            with self.assertRaises(ValueError):
                read_binding(SourceTree(self.root, head))

    def test_edit_distance_exact_ref_keeps_binding_validation_narrow(self):
        self.ref = "refs/heads/codex/edit-distance-trim-20261018"
        self.change_bits()
        self.assertEqual(self.plan()["expected_ref"], self.ref)
        binding = read_binding(SourceTree(self.root, self.git("rev-parse", "HEAD").decode().strip()))
        for ref in ("refs/heads/codex/edit-distance-trim-20261019",
                    "refs/heads/codex/edit-distance-other-20261018",
                    "refs/heads/codex/edit-distance-trim-20261018-extra",
                    "refs/heads/codex/edit-distance-trim-20261018/child",
                    "refs/heads/codex/edit-distance-trim-20261018..x",
                    "refs/tags/codex/edit-distance-trim-20261018"):
            with self.subTest(ref=ref):
                self.write(BINDING_PATH, (canonical({**binding, "ref": ref}) + b"\n").decode())
                head = self.commit("invalid exact edit-distance ref", bind=False)
                with self.assertRaisesRegex(ValueError, "exact temporary branch"):
                    read_binding(SourceTree(self.root, head))

    def test_fps_scalar_return_exact_ref_keeps_binding_validation_narrow(self):
        self.ref = "refs/heads/codex/fps-scalar-return-20261018"
        self.change_bits()
        self.assertEqual(self.plan()["expected_ref"], self.ref)
        binding = read_binding(SourceTree(self.root, self.git("rev-parse", "HEAD").decode().strip()))
        for ref in ("refs/heads/codex/fps-scalar-return-20261019",
                    "refs/heads/codex/fps-scalar-other-20261018",
                    "refs/heads/codex/fps-scalar-return-20261018-extra",
                    "refs/heads/codex/fps-scalar-return-20261018/child",
                    "refs/heads/codex/fps-scalar-return-20261018..x",
                    "refs/tags/codex/fps-scalar-return-20261018"):
            with self.subTest(ref=ref):
                self.write(BINDING_PATH, (canonical({**binding, "ref": ref}) + b"\n").decode())
                head = self.commit("invalid exact FPS ref", bind=False)
                with self.assertRaisesRegex(ValueError, "exact temporary branch"):
                    read_binding(SourceTree(self.root, head))

    def test_no_state_read_write_and_record_is_selected_only(self):
        head = self.change_bits()
        state = self.root / ".verify-helper/verified-state.json"
        self.write(state.relative_to(self.root).as_posix(), '{"sentinel":"unchanged"}\n')
        before = state.read_bytes()
        self.main("plan", "--affected-base", self.base, "--expected-head", head)
        plan = json.loads((self.root / ".verify-helper/selection.json").read_text())
        result = json.loads(self.main("record", "--groups", "headers", "unit"))
        self.assertEqual(state.read_bytes(), before)
        self.assertEqual(result["groups"], {"headers": plan["headers"], "unit": plan["unit"]})
        self.assertFalse(result["successful_cache_written"])
        self.assertEqual(result["unselected_checks"], "not_run")
        with self.assertRaises(ValueError):
            ci.record_success(None, plan, ["unit"])
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.main("save")
        self.assertEqual(state.read_bytes(), before)
        with self.assertRaises(ValueError):
            record_affected_success(plan, ["unknown"])

    def test_scoped_marker_only_selected_and_failure_paths(self):
        self.change_bits()
        plan, marker = self.plan(), Marker()
        expected = plan["verification"]
        def run(paths):
            self.assertEqual([path.as_posix() for path in paths], expected)
            for path in paths:
                self.assertFalse(marker.is_verified(path))
                marker.mark_verified(path)
            return Summary()
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            ci.verify_selected(plan, marker, run)
        self.assertEqual(marker.failed_calls, expected)
        self.assertEqual(marker.verified_calls, expected)
        self.assertEqual(marker.verified, set(expected))
        self.assertIn("unaffected/not run", output.getvalue())
        for callback in (lambda paths: Summary(False), lambda paths: Summary()):
            with self.assertRaises(RuntimeError):
                ci.verify_selected(plan, Marker(), callback)
        with self.assertRaises(TimeoutError):
            ci.verify_selected(plan, Marker(), lambda paths: (_ for _ in ()).throw(TimeoutError()))

    def test_modified_plan_or_later_head_is_rejected(self):
        self.change_bits()
        original = self.plan()
        validate_affected_plan(self.root, original)
        for key, value in (("unit", []), ("plan_sha256", "0" * 64), ("full", True), ("parent", [])):
            changed = copy.deepcopy(original)
            changed[key] = value
            with self.assertRaises(ValueError):
                validate_affected_plan(self.root, changed)
        changed = copy.deepcopy(original)
        changed["state"]["targets"]["unit"] = {}
        with self.assertRaises(ValueError):
            validate_affected_plan(self.root, changed)
        self.write("math/unrelated.hpp", "#pragma once\nusing extra = int;\n")
        self.commit("new head")
        with self.assertRaises(ValueError):
            validate_affected_plan(self.root, original)

    def test_mode_downgrade_cannot_route_affected_inventory_to_cache(self):
        self.change_bits()
        original = self.plan()
        for mode in (None, "legacy", "affected-unknown"):
            changed = copy.deepcopy(original)
            if mode is None:
                del changed["mode"]
            else:
                changed["mode"] = mode
            self.write(".verify-helper/selection.json", json.dumps(changed))
            with self.assertRaises(ValueError):
                ci.record_success(None, changed, ["unit"])
            with self.assertRaises(ValueError):
                ci.verify_selected(changed, Marker(), lambda paths: self.fail("Tampered judge must not run"))
            for command in ("judge", "record", "save", "docs"):
                with self.assertRaises(ValueError):
                    self.main(command)
            self.assertFalse((self.root / ".verify-helper/verified-state.json").exists())

    def test_temporary_context_refuses_complete_provenance_stripping(self):
        self.change_bits()
        normal = ci.build_plan(self.root, environment="fixture")
        self.write(".verify-helper/selection.json", json.dumps(normal))
        for context in ({"VERIFY_NO_CACHE": "true"},):
            with patch.dict(os.environ, context):
                for command in ("plan", "judge", "record", "save", "docs"):
                    with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                        self.main(command)
        self.assertFalse((self.root / ".verify-helper/verified-state.json").exists())

    def test_legacy_cache_plan_and_record_remain_unchanged(self):
        first = ci.build_plan(self.root, environment="fixture")
        self.assertTrue(first["full"])
        previous = ci.record_success(None, first, ci.GROUPS)
        self.assertEqual(previous["version"], ci.VERSION)
        same = ci.build_plan(self.root, previous, environment="fixture")
        self.assertTrue(all(same[group] == [] for group in ci.GROUPS))
        self.change_bits()
        old_mode = ci.build_plan(self.root, previous, environment="fixture")
        new_mode = self.plan()
        for group in ci.GROUPS:
            self.assertEqual(old_mode[group], new_mode[group])
        marker = Marker()
        legacy = copy.deepcopy(first)
        legacy["verification"] = []
        with contextlib.redirect_stdout(io.StringIO()):
            ci.verify_selected(legacy, marker, lambda paths: self.fail("No selected judges"))
        self.assertEqual(marker.verified, set(first["state"]["targets"]["verification"]))


    def push_context(self, head):
        event = {"ref": self.ref, "after": head, "deleted": False}
        context = {"GITHUB_EVENT_NAME": "push", "GITHUB_REPOSITORY": "qandori/cp-library",
                   "GITHUB_REF": self.ref, "GITHUB_SHA": head}
        return event, context

    def test_exact_push_binding_and_event_checks(self):
        head = self.change_bits()
        event, context = self.push_context(head)
        result = verify_push_binding(self.root, event, context, self.ref)
        self.assertEqual(result["base"], self.base)
        self.assertEqual(result["head"], head)
        for key, value in (("GITHUB_EVENT_NAME", "workflow_dispatch"), ("GITHUB_REPOSITORY", "someone/cp-library"),
                           ("GITHUB_REF", "refs/heads/main"), ("GITHUB_SHA", "f" * 40)):
            with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                verify_push_binding(self.root, event, {**context, key: value}, self.ref)
        for key, value in (("ref", "refs/heads/main"), ("after", "f" * 40), ("deleted", True)):
            with self.assertRaises(ValueError):
                verify_push_binding(self.root, {**event, key: value}, context, self.ref)
        with self.assertRaises(ValueError):
            verify_push_binding(self.root, event, context, "refs/heads/main")

    def test_binding_schema_digest_and_canonical_bytes(self):
        good_head = self.change_bits()
        good = read_binding(SourceTree(self.root, good_head))
        bad_values = [{**good, "unknown": "executable"}, {**good, "version": True},
                      {**good, "source_digest": "0" * 64}, {**good, "source_digest": ""},
                      {**good, "base": "0" * 40}, {**good, "ref": "refs/heads/main"},
                      {**good, "ref": self.ref + "\nINJECTED=1"}]
        for value in bad_values:
            self.git("checkout", "--quiet", good_head)
            self.write(BINDING_PATH, (canonical(value) + b"\n").decode())
            self.commit("invalid binding", bind=False)
            with self.assertRaises(ValueError):
                self.plan()
        for raw in (json.dumps(good, indent=2) + "\n", canonical(good).decode(),
                    canonical(good).decode()[:-1] + ',"version":1}\n'):
            self.git("checkout", "--quiet", good_head)
            self.write(BINDING_PATH, raw)
            self.commit("noncanonical binding", bind=False)
            with self.assertRaises(ValueError):
                self.plan()

    def test_binding_fixed_path_mode_and_no_broad_exclusion(self):
        good_head = self.change_bits()
        path = self.root / BINDING_PATH
        path.chmod(0o755)
        self.commit("executable binding", bind=False)
        with self.assertRaisesRegex(ValueError, "fixed regular data file"):
            self.plan()
        self.git("checkout", "--quiet", good_head)
        (self.root / BINDING_PATH).unlink()
        self.commit("missing binding", bind=False)
        with self.assertRaisesRegex(ValueError, "fixed regular data file"):
            self.plan()
        self.git("checkout", "--quiet", good_head)
        self.write("nested/.verify-helper/affected-binding.json", "{}\n")
        self.commit("similarly named file is substantive", bind=False)
        with self.assertRaisesRegex(ValueError, "source digest mismatch"):
            self.plan()

    def test_workflow_package_and_source_changes_all_need_rebinding(self):
        good_head = self.change_bits()
        for name in (".github/workflows/new.yml", "test/benchmark/package/manifest.json", "math/unrelated.hpp"):
            self.git("checkout", "--quiet", good_head)
            self.write(name, "changed substantive bytes\n")
            self.commit("unreviewed substantive change", bind=False)
            with self.assertRaisesRegex(ValueError, "source digest mismatch"):
                self.plan()



if __name__ == "__main__":
    unittest.main()
