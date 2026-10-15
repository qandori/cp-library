import copy
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from ci import ROOT, build_plan, read_json, verify_selected, write_json


class Marker:
    def __init__(self, verified=()):
        self.verified = set(verified)

    def mark_failed(self, path):
        self.verified.discard(path.as_posix())

    def mark_verified(self, path):
        self.verified.add(path.as_posix())

    def is_verified(self, path):
        return path.as_posix() in self.verified


class Summary:
    def __init__(self, success=True):
        self.success = success

    def show(self):
        pass

    def succeeded(self):
        return self.success


class IncrementalVerification(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        for name, source in {
            "math/mint.hpp": "#pragma once\n",
            "math/comb.hpp": '#include "mint.hpp"\n',
            "graph/lca.hpp": "#pragma once\n",
            "test/unit/comb.cpp": '#include "math/comb.hpp"\n',
            "test/unit/mint.cpp": '#include "math/mint.hpp"\n',
            "test/unit/lca.cpp": '#include "graph/lca.hpp"\n',
            "test/comb.test.cpp": '#include "../math/comb.hpp"\n',
            "test/lca.test.cpp": '#include "../graph/lca.hpp"\n',
            "test/requirements.txt": "online-judge-verify-helper==5.6.0\n",
        }.items():
            self.write(name, source)
        self.previous = self.plan()["state"]

    def write(self, name, source):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(source)

    def plan(self, previous=None, **kwargs):
        return build_plan(self.root, previous, environment=kwargs.pop("environment", "test"), **kwargs)

    def test_first_run_is_full(self):
        plan = self.plan()
        self.assertTrue(plan["full"])
        self.assertEqual(len(plan["headers"]), 3)
        self.assertEqual(len(plan["unit"]), 3)
        self.assertEqual(len(plan["verification"]), 2)

    def test_unchanged_and_documentation_only(self):
        self.write("docs/comb.md", "Updated documentation\n")
        self.write("test/benchmark/result.md", "Updated benchmark\n")
        self.write("math/mint.hpp", "#pragma once\n")
        plan = self.plan(self.previous)
        for group in ("headers", "unit", "verification"):
            self.assertEqual(plan[group], [])

    def test_transitive_header_change(self):
        self.write("math/mint.hpp", "#pragma once\nusing mint = int;\n")
        plan = self.plan(self.previous)
        self.assertEqual(plan["headers"], ["math/comb.hpp", "math/mint.hpp"])
        self.assertEqual(plan["unit"], ["test/unit/comb.cpp", "test/unit/mint.cpp"])
        self.assertEqual(plan["verification"], ["test/comb.test.cpp"])

    def test_driver_only_change(self):
        self.write("test/comb.test.cpp", '#include "../math/comb.hpp"\nint main() {}\n')
        plan = self.plan(self.previous)
        self.assertEqual(plan["headers"], [])
        self.assertEqual(plan["unit"], [])
        self.assertEqual(plan["verification"], ["test/comb.test.cpp"])

    def test_nested_verification_driver(self):
        self.write("test/nested/comb.test.cpp", '#include "../../math/comb.hpp"\n')
        plan = self.plan(self.previous)
        self.assertEqual(plan["verification"], ["test/nested/comb.test.cpp"])
        self.write("math/mint.hpp", "#pragma once\nusing mint = int;\n")
        self.assertIn("test/nested/comb.test.cpp", self.plan(plan["state"])["verification"])

    def test_unit_only_change(self):
        self.write("test/unit/comb.cpp", '#include "math/comb.hpp"\nint main() {}\n')
        plan = self.plan(self.previous)
        self.assertEqual(plan["headers"], [])
        self.assertEqual(plan["unit"], ["test/unit/comb.cpp"])
        self.assertEqual(plan["verification"], [])

    def test_new_and_deleted_header(self):
        self.write("math/new.hpp", '#include "mint.hpp"\n')
        self.assertEqual(self.plan(self.previous)["headers"], ["math/new.hpp"])
        (self.root / "math/mint.hpp").unlink()
        plan = self.plan(self.previous)
        self.assertIn("math/comb.hpp", plan["headers"])
        self.assertIn("test/comb.test.cpp", plan["verification"])

    def test_string_header_and_dependencies(self):
        self.write("string/lcs.hpp", "#pragma once\n")
        self.write("test/unit/lcs.cpp", '#include "string/lcs.hpp"\n')
        self.write("test/lcs.test.cpp", '#include "../string/lcs.hpp"\n')
        plan = self.plan(self.previous)
        self.assertEqual(plan["headers"], ["string/lcs.hpp"])
        self.assertEqual(plan["unit"], ["test/unit/lcs.cpp"])
        self.assertEqual(plan["verification"], ["test/lcs.test.cpp"])
        self.write("string/lcs.hpp", "#pragma once\nint lcs();\n")
        changed = self.plan(plan["state"])
        for group in ("headers", "unit", "verification"):
            self.assertEqual(changed[group], plan[group])

    def test_deleted_test_is_not_run(self):
        (self.root / "test/comb.test.cpp").unlink()
        plan = self.plan(self.previous)
        self.assertEqual(plan["verification"], [])
        self.assertNotIn("test/comb.test.cpp", plan["state"]["targets"]["verification"])

    def test_include_resolution_and_shadowing(self):
        self.write("math/mint.hpp", '#include "../math/detail/../small.hpp"\n')
        self.write("math/small.hpp", "#pragma once\n")
        previous = self.plan()["state"]
        self.write("math/small.hpp", "#pragma once\nusing number = long;\n")
        self.assertIn("test/comb.test.cpp", self.plan(previous)["verification"])
        self.write("test/unit/math/comb.hpp", "#pragma once\n")
        self.assertIn("test/unit/comb.cpp", self.plan(previous)["unit"])

    def test_angle_include_and_cycle(self):
        self.write("math/mint.hpp", '#include <math/comb.hpp>\n')
        previous = self.plan()["state"]
        self.write("math/comb.hpp", '#include "mint.hpp"\nusing value = long;\n')
        plan = self.plan(previous)
        self.assertEqual(plan["headers"], ["math/comb.hpp", "math/mint.hpp"])

    def test_new_local_file_shadowing_system_header(self):
        self.write("math/mint.hpp", "#include <vector>\n")
        previous = self.plan()["state"]
        self.write("vector", "#pragma once\n")
        self.assertIn("test/comb.test.cpp", self.plan(previous)["verification"])

    def test_common_configuration_and_environment_changes(self):
        self.assertTrue(self.plan(self.previous, environment="new compiler")["full"])
        self.write(".verify-helper/config.toml", "changed = true\n")
        self.assertTrue(self.plan(self.previous)["full"])

    def test_forced_or_unsupported_dependency_runs_everything(self):
        self.assertTrue(self.plan(self.previous, force=True)["full"])
        for source in ("#include HEADER\n", '#include_next "other.hpp"\n', '#if __has_include("other.hpp")\n#endif\n'):
            self.write("math/mint.hpp", source)
            self.assertTrue(self.plan(self.previous)["full"])

    def test_bad_cache_and_failed_previous_run(self):
        for previous in (None, [], {}, {"version": 999}):
            self.assertTrue(self.plan(previous)["full"])
        state_path = self.root / "state.json"
        write_json(state_path, self.previous)
        self.write("math/mint.hpp", "#pragma once\nusing mint = int;\n")
        first = self.plan(read_json(state_path))
        second = self.plan(read_json(state_path))
        self.assertEqual(first, second)
        self.assertEqual(read_json(state_path), self.previous)

    def test_successful_state_skips_already_verified_content(self):
        self.write("math/mint.hpp", "#pragma once\nusing mint = int;\n")
        successful = self.plan(self.previous)["state"]
        self.assertEqual(self.plan(successful)["verification"], [])

    def test_verification_never_skips_selected_or_runs_unselected(self):
        plan = self.plan(self.previous)
        plan["verification"] = ["test/comb.test.cpp"]
        marker = Marker(["test/comb.test.cpp"])

        def run(paths):
            self.assertEqual(paths, [Path("test/comb.test.cpp")])
            self.assertFalse(marker.is_verified(paths[0]))
            marker.mark_verified(paths[0])
            return Summary()

        verify_selected(plan, marker, run)
        self.assertEqual(marker.verified, {"test/comb.test.cpp", "test/lca.test.cpp"})
        plan["verification"] = []
        verify_selected(plan, marker, lambda paths: self.fail("Unchanged tests must not run"))

    def test_failure_and_timeout_are_not_successes(self):
        plan = copy.deepcopy(self.plan())
        with self.assertRaisesRegex(RuntimeError, "verification failed"):
            verify_selected(plan, Marker(), lambda paths: Summary(False))
        with self.assertRaisesRegex(RuntimeError, "did not finish"):
            verify_selected(plan, Marker(), lambda paths: Summary())

    def test_repository_wavelet_dependencies(self):
        previous = build_plan(ROOT, environment="test")["state"]
        original = Path.read_bytes

        def changed(path):
            data = original(path)
            return data + b"\n" if path == ROOT / "data_structure/wavelet_matrix.hpp" else data

        with patch.object(Path, "read_bytes", changed):
            plan = build_plan(ROOT, previous, environment="test")
        self.assertFalse(plan["full"])
        self.assertEqual(plan["headers"], ["data_structure/wavelet_matrix.hpp"])
        self.assertEqual(plan["unit"], ["test/unit/standalone.cpp", "test/unit/wavelet_matrix.cpp"])
        self.assertEqual(plan["verification"], ["test/wavelet_matrix_freq.test.cpp", "test/wavelet_matrix_kth.test.cpp", "test/wavelet_matrix_sum.test.cpp"])

    def test_repository_mint_dependencies(self):
        previous = build_plan(ROOT, environment="test")["state"]
        original = Path.read_bytes

        def changed(path):
            data = original(path)
            return data + b"\n" if path == ROOT / "math/mint.hpp" else data

        with patch.object(Path, "read_bytes", changed):
            plan = build_plan(ROOT, previous, environment="test")
        self.assertFalse(plan["full"])
        self.assertIn("math/fps.hpp", plan["headers"])
        self.assertIn("convolution/convolution.hpp", plan["headers"])
        self.assertIn("test/convolution.test.cpp", plan["verification"])
        self.assertIn("test/weighted_union_find.test.cpp", plan["verification"])
        self.assertNotIn("test/unit/lca.cpp", plan["unit"])


if __name__ == "__main__":
    unittest.main()
