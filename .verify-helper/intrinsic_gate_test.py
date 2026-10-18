import copy
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import unittest
from unittest.mock import Mock, patch

import affected_test
from affected import SourceTree, canonical
import intrinsic_gate as gate
import no_cache_route as route
import no_cache_route_test


MACROS = (b"#define assert(expr) check(expr)\n#define ONLINE_JUDGE 1\n#define ATCODER 1\n"
          b"#define __GNUC__ 15\n#define __GNUC_MINOR__ 2\n#define __GNUC_PATCHLEVEL__ 0\n"
          b"#define __x86_64__ 1\n")
NATIVE_MACROS = MACROS + b"#define __AVX2__ 1\n#define __BMI2__ 1\n"


class IntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.write("test/unit/dynamic_wavelet_core.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n')
        self.head = self.commit("intrinsic fixture\n\n" + route.TRAILER)

    def test_exact_push_pr_and_main_select_all_five_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.scope_mode(plan), "dwm")
        self.assertEqual(set(plan["unit"]), gate.REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(set(plan["unit"]), gate.REQUIRED_UNITS)

    def test_normal_event_does_not_plan_or_execute_cpp(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        head = self.commit("ordinary event", bind=False)
        with patch.object(gate, "build_affected_plan", side_effect=AssertionError("unexpected planner")):
            self.assertEqual(gate.selected(self.root, *self.push(head, route.MAIN_REF)),
                             ({"mode": "normal"}, None))

    def test_missing_required_unit_and_changed_source_fail_closed(self):
        (self.root / "test/unit/dynamic_wavelet_core.cpp").unlink()
        self.head = self.commit("remove required unit\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "Required DWM units"):
            gate.selected(self.root, *self.push())
        self.write("components/dynamic_bit_vector.hpp", "changed but not committed\n")
        with self.assertRaisesRegex(ValueError, "checkout differs"):
            gate.selected(self.root, *self.push())

    def test_dependency_manifest_uses_exact_checkout_bytes(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/dynamic_wavelet_core.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="intrinsic-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            names = sorted({target} | gate.PRODUCTS)
            continuation = " " + chr(92) + "\n "
            depfile.write_text("gate-output: " + continuation.join(str(self.root / name) for name in names) + "\n")
            proof = gate.dependency_proof(self.root, tree, depfile, source, target)
            self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, set(names))
            self.write("components/dynamic_bit_vector.hpp", "mismatched baseline bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target)

    def test_external_baseline_and_missing_production_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/dynamic_wavelet_core.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="intrinsic-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "dynamic_bit_vector.hpp"
            old.write_bytes(tree.content("components/dynamic_bit_vector.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target)
            depfile.write_text(f"gate-output: {source}\n")
            with self.assertRaisesRegex(ValueError, "current production DWM headers"):
                gate.dependency_proof(self.root, tree, depfile, source, target)

    def test_macro_requirements_and_explicit_scalar_flags(self):
        base, native = MACROS, NATIVE_MACROS
        self.assertTrue(gate.macro_proof(native, "native")["assertions"])
        self.assertEqual(gate.macro_proof(base, "scalar")["isa"], {})
        for raw, mode in ((base, "native"), (native, "scalar"),
                          (native + b"#define NDEBUG 1\n", "native"),
                          (native.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "native")):
            with self.assertRaises(ValueError):
                gate.macro_proof(raw, mode)
        self.assertEqual(gate.MODES["scalar"], ["-march=x86-64", "-mtune=generic", "-mno-avx",
            "-mno-avx2", "-mno-avx512f", "-mno-bmi", "-mno-bmi2",
            "-fno-tree-loop-vectorize", "-fno-tree-slp-vectorize"])
        self.assertIn("-fno-lto", gate.COMMON)
        self.assertNotIn("-DNDEBUG", gate.COMMON)
        for mode in gate.MODES:
            self.assertEqual(gate.mode_flags(mode), gate.COMMON + gate.MODES[mode])
            self.assertNotIn("-DMATRIX_DISABLE_AVX2", gate.mode_flags(mode))

    def test_reduced_or_oversized_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        for group, change in (("unit", []), ("headers", []),
                              ("unit", plan["unit"] + ["test/unit/extra.cpp"] * 4)):
            altered = copy.deepcopy(plan)
            altered[group] = change
            with patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())


class MatrixIntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.git("checkout", "--quiet", self.base)
        self.write("components/matrix_arithmetic.hpp", "#pragma once\n")
        self.write("math/matrix.hpp", '#include "../components/matrix_arithmetic.hpp"\n')
        self.write("math/mint.hpp", "#pragma once\n")
        for name in ("matrix", "matrix_characteristic"):
            self.write("test/unit/" + name + ".cpp", '#include "math/matrix.hpp"\n#include "math/mint.hpp"\n')
        self.write("test/unit/standalone.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n'
                   '#include "math/matrix.hpp"\n')
        self.base = self.commit("matrix baseline", bind=False)
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
        self.write("components/matrix_arithmetic.hpp", "#pragma once\nusing matrix_changed = int;\n")
        self.head = self.commit("matrix candidate\n\n" + route.TRAILER)

    def test_exact_matrix_push_pr_and_main_select_transitive_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.scope_mode(plan), "matrix")
        self.assertEqual(set(plan["unit"]), gate.MATRIX_REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.MATRIX_PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(gate.scope_mode(plan), "matrix")
            self.assertEqual(set(plan["unit"]), gate.MATRIX_REQUIRED_UNITS)
            self.assertEqual(set(plan["headers"]), gate.MATRIX_PRODUCTS)

    def test_missing_matrix_units_or_headers_fail_closed(self):
        head = self.head
        for name in sorted(gate.MATRIX_REQUIRED_UNITS | gate.MATRIX_PRODUCTS):
            with self.subTest(name=name):
                self.git("checkout", "--quiet", head)
                (self.root / name).unlink()
                self.head = self.commit("remove matrix input\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "(Required MATRIX units|Production MATRIX headers|Include escapes source tree)"):
                    gate.selected(self.root, *self.push())

    def test_reduced_extra_or_mixed_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        variants = []
        for group in ("headers", "unit"):
            for target in plan[group]:
                altered = copy.deepcopy(plan)
                altered[group].remove(target)
                variants.append(altered)
            for extra in ([plan[group][0]], ["test/unit/extra.cpp" if group == "unit" else "math/extra.hpp"]):
                altered = copy.deepcopy(plan)
                altered[group].extend(extra)
                variants.append(altered)
        for field, values in (("changed_paths", []), ("changed_paths", ["math/unrelated.hpp"]),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.PRODUCTS))):
            altered = copy.deepcopy(plan)
            altered[field] = values
            variants.append(altered)
        for altered in variants:
            with self.subTest(plan=altered), patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())

    def test_actual_extra_dependent_is_rejected_not_removed(self):
        self.write("test/unit/matrix_extra.cpp", '#include "math/matrix.hpp"\n')
        self.head = self.commit("new dependent requires review\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exceeds reviewed gate budget"):
            gate.selected(self.root, *self.push())

    def test_actual_mixed_production_changes_fail_closed(self):
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing other_changed = int;\n")
        self.head = self.commit("mixed intrinsic changes\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exactly one reviewed intrinsic mode"):
            gate.selected(self.root, *self.push())

    def test_matrix_dependency_proofs_include_exact_production_bytes(self):
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="matrix-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            for target in sorted(gate.MATRIX_REQUIRED_UNITS | gate.MATRIX_PRODUCTS):
                generated = target in gate.MATRIX_PRODUCTS
                source = Path(directory) / "header.cpp" if generated else self.root / target
                if generated:
                    source.write_text(gate.header_source(target))
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source)} | {str(self.root / name) for name in names}
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                proof = gate.dependency_proof(self.root, tree, depfile, source, target, generated, "matrix")
                self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, names)
                self.assertEqual(set(proof["production_headers"]), gate.MATRIX_PRODUCTS & names)
                for product in gate.MATRIX_PRODUCTS & names:
                    with self.subTest(target=target, omitted=product):
                        depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(self.root / product)})) + "\n")
                        with self.assertRaisesRegex(ValueError, "current production MATRIX headers"):
                            gate.dependency_proof(self.root, tree, depfile, source, target, generated, "matrix")

    def test_matrix_baseline_foreign_unresolved_and_changed_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/matrix.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="matrix-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "matrix_arithmetic.hpp"
            old.write_bytes(tree.content("components/matrix_arithmetic.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            paths = {str(self.root / name) for name in {target} | gate.MATRIX_PRODUCTS}
            depfile.write_text("gate-output: " + " ".join(sorted(paths | {str(self.root / 'math/unrelated.hpp')})) + "\n")
            with self.assertRaisesRegex(ValueError, "not resolved by reviewed source selector"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(source)})) + "\n")
            with self.assertRaisesRegex(ValueError, "actual source"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
            self.write("components/matrix_arithmetic.hpp", "wrong source bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")

    def test_matrix_scalar_flags_and_runtime_dispatch_macro_are_required(self):
        scalar = MACROS + b"#define MATRIX_DISABLE_AVX2 1\n"
        self.assertEqual(gate.macro_proof(scalar, "scalar", "matrix")["MATRIX_DISABLE_AVX2"], "1")
        self.assertIsNone(gate.macro_proof(NATIVE_MACROS, "native", "matrix")["MATRIX_DISABLE_AVX2"])
        for raw, mode in ((MACROS, "scalar"), (scalar.replace(b"AVX2 1", b"AVX2 0"), "scalar"),
                          (scalar + b"#define __AVX2__ 1\n", "scalar"),
                          (scalar + b"#define NDEBUG 1\n", "scalar"),
                          (NATIVE_MACROS + b"#define MATRIX_DISABLE_AVX2 1\n", "native"),
                          (NATIVE_MACROS + b"#define NDEBUG 1\n", "native"),
                          (scalar.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "scalar")):
            with self.subTest(raw=raw, mode=mode), self.assertRaises(ValueError):
                gate.macro_proof(raw, mode, "matrix")
        self.assertEqual(gate.mode_flags("native", "matrix"), gate.COMMON + gate.MODES["native"])
        self.assertEqual(gate.mode_flags("scalar", "matrix"),
                         gate.COMMON + gate.MODES["scalar"] + ["-DMATRIX_DISABLE_AVX2"])

    def test_matrix_driver_runs_every_target_with_checked_mode_flags(self):
        selected, plan = gate.selected(self.root, *self.push())
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="matrix-driver-") as directory:
            directory = Path(directory)
            compiler, cc1 = directory / "g++-15", directory / "cc1plus"
            compiler.write_bytes(b"compiler fixture")
            cc1.write_bytes(b"cc1 fixture")
            commands = gate.Commands(directory)
            compiled, executed, probes = [], [], []

            def run(command, *, source=b"", seconds=gate.COMMAND_SECONDS):
                command = [str(value) for value in command]
                commands.count += 1
                if command == [str(compiler), "-dumpfullversion"]:
                    return b"15.2.0\n"
                if command == [str(compiler), "-print-prog-name=cc1plus"]:
                    return str(cc1).encode() + b"\n"
                if command[0] != str(compiler):
                    self.assertEqual(len(command), 1)
                    executed.append(command[0])
                    return b""
                mode = "native" if "-march=native" in command else "scalar"
                flags = gate.mode_flags(mode, "matrix")
                self.assertEqual(command[1:1 + len(flags)], flags)
                if "-dM" in command:
                    self.assertEqual(source, b"#include <cassert>\n")
                    probes.append(mode)
                    return NATIVE_MACROS if mode == "native" else MACROS + b"#define MATRIX_DISABLE_AVX2 1\n"
                self.assertEqual(command[1 + len(flags):3 + len(flags)], ["-I", str(self.root)])
                source_path, binary = Path(command[-3]), Path(command[-1])
                if source_path.is_relative_to(self.root):
                    target = source_path.relative_to(self.root).as_posix()
                else:
                    target = source_path.read_text().split('"')[1]
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source_path)} | {str(self.root / name) for name in names}
                depfile = Path(command[command.index("-MF") + 1])
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                binary.write_bytes(b"test binary fixture")
                compiled.append((mode, target, str(binary)))
                return b""

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands), \
                 patch.object(commands, "run", side_effect=run), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "emit") as emit:
                gate.execute(self.root, selected, plan, directory)
            self.assertEqual(probes, ["native", "scalar"])
            self.assertEqual([(mode, target) for mode, target, _ in compiled],
                             [(mode, target) for mode in ("native", "scalar")
                              for group in ("headers", "unit") for target in plan[group]])
            self.assertEqual(executed, [binary for _, _, binary in compiled])
            self.assertEqual(commands.count, 24)
            self.assertEqual(emit.call_args.args, ("gate_completed",))

    def test_normal_matrix_event_does_not_plan_or_run_cpp(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        self.write("components/matrix_arithmetic.hpp", "#pragma once\nusing normal = int;\n")
        head = self.commit("ordinary matrix event", bind=False)
        event, context = self.push(head, route.MAIN_REF)
        with tempfile.TemporaryDirectory(prefix="matrix-event-") as directory:
            event_path = Path(directory) / "event.json"
            event_path.write_text(json.dumps(event))
            with patch.dict(os.environ, {**context, "GITHUB_EVENT_PATH": str(event_path)}), \
                 patch.object(gate, "__file__", str(self.root / ".verify-helper/intrinsic_gate.py")), \
                 patch.object(gate, "build_affected_plan", side_effect=AssertionError("unexpected planner")), \
                 patch.object(gate, "execute", side_effect=AssertionError("unexpected C++")), \
                 patch.object(gate, "emit") as emit:
                gate.main()
            emit.assert_called_once_with("gate_skipped", reason="normal event; no extra C++ commands")


class FlowIntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.git("checkout", "--quiet", self.base)
        self.write("components/flow_graph.hpp", "#pragma once\n")
        self.write("components/push_relabel.hpp", '#include "flow_graph.hpp"\n')
        self.write("graph/max_flow.hpp", '#include "../components/push_relabel.hpp"\n')
        self.write("graph/min_cost_flow.hpp", '#include "../components/flow_graph.hpp"\n')
        self.write("test/unit/max_flow.cpp", '#include "graph/max_flow.hpp"\n')
        self.write("test/unit/flow_capacity.cpp", '#include "graph/max_flow.hpp"\n'
                   '#include "graph/min_cost_flow.hpp"\n')
        self.write("test/unit/standalone.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n'
                   '#include "graph/max_flow.hpp"\n#include "graph/min_cost_flow.hpp"\n')
        for name in ("max_flow", "max_flow_yuki"):
            self.write("test/" + name + ".test.cpp", '#include "../graph/max_flow.hpp"\n')
        self.base = self.commit("flow baseline", bind=False)
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
        self.write("components/push_relabel.hpp", '#include "flow_graph.hpp"\nusing flow_changed = int;\n')
        self.head = self.commit("flow candidate\n\n" + route.TRAILER)

    def test_exact_flow_push_pr_and_main_select_transitive_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.scope_mode(plan), "flow")
        self.assertEqual(set(plan["unit"]), gate.FLOW_REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.FLOW_PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        self.assertEqual(plan["verification"], ["test/max_flow.test.cpp", "test/max_flow_yuki.test.cpp"])
        self.assertEqual(plan["sanitize"], plan["unit"])
        self.assertEqual(plan["bundles"], plan["headers"])
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(gate.scope_mode(plan), "flow")
            self.assertEqual(set(plan["unit"]), gate.FLOW_REQUIRED_UNITS)
            self.assertEqual(set(plan["headers"]), gate.FLOW_PRODUCTS)
            self.assertEqual(plan["verification"], ["test/max_flow.test.cpp", "test/max_flow_yuki.test.cpp"])

    def test_missing_flow_units_or_headers_fail_closed(self):
        head = self.head
        for name in sorted(gate.FLOW_REQUIRED_UNITS | gate.FLOW_PRODUCTS):
            with self.subTest(name=name):
                self.git("checkout", "--quiet", head)
                (self.root / name).unlink()
                self.head = self.commit("remove flow input\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "(Required FLOW units|Production FLOW headers|Include escapes source tree)"):
                    gate.selected(self.root, *self.push())

    def test_reduced_extra_or_mixed_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        variants = []
        for group in ("headers", "unit"):
            for target in plan[group]:
                altered = copy.deepcopy(plan)
                altered[group].remove(target)
                variants.append(altered)
            for extra in ([plan[group][0]], ["test/unit/extra.cpp" if group == "unit" else "math/extra.hpp"]):
                altered = copy.deepcopy(plan)
                altered[group].extend(extra)
                variants.append(altered)
        for field, values in (("changed_paths", []), ("changed_paths", ["math/unrelated.hpp"]),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.PRODUCTS)),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.MATRIX_PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.MATRIX_PRODUCTS))):
            altered = copy.deepcopy(plan)
            altered[field] = values
            variants.append(altered)
        for altered in variants:
            with self.subTest(plan=altered), patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())

    def test_actual_extra_dependent_is_rejected_not_removed(self):
        self.write("test/unit/flow_extra.cpp", '#include "graph/max_flow.hpp"\n')
        self.head = self.commit("new dependent requires review\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exceeds reviewed gate budget"):
            gate.selected(self.root, *self.push())

    def test_actual_mixed_production_changes_fail_closed(self):
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing other_changed = int;\n")
        self.head = self.commit("mixed intrinsic changes\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exactly one reviewed intrinsic mode"):
            gate.selected(self.root, *self.push())

    def test_flow_dependency_proofs_include_exact_production_bytes(self):
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="flow-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            for target in sorted(gate.FLOW_REQUIRED_UNITS | gate.FLOW_PRODUCTS):
                generated = target in gate.FLOW_PRODUCTS
                source = Path(directory) / "header.cpp" if generated else self.root / target
                if generated:
                    source.write_text(gate.header_source(target))
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source)} | {str(self.root / name) for name in names}
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                proof = gate.dependency_proof(self.root, tree, depfile, source, target, generated, "flow")
                self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, names)
                self.assertEqual(set(proof["production_headers"]), gate.FLOW_PRODUCTS & names)
                for product in gate.FLOW_PRODUCTS & names:
                    with self.subTest(target=target, omitted=product):
                        depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(self.root / product)})) + "\n")
                        with self.assertRaisesRegex(ValueError, "current production FLOW headers"):
                            gate.dependency_proof(self.root, tree, depfile, source, target, generated, "flow")

    def test_flow_baseline_foreign_unresolved_and_changed_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/max_flow.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="flow-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "push_relabel.hpp"
            old.write_bytes(tree.content("components/push_relabel.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="flow")
            paths = {str(self.root / name) for name in {target} | gate.FLOW_PRODUCTS}
            depfile.write_text("gate-output: " + " ".join(sorted(paths | {str(self.root / 'math/unrelated.hpp')})) + "\n")
            with self.assertRaisesRegex(ValueError, "not resolved by reviewed source selector"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="flow")
            depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(source)})) + "\n")
            with self.assertRaisesRegex(ValueError, "actual source"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="flow")
            depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
            self.write("components/push_relabel.hpp", "wrong source bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="flow")

    def test_flow_modes_keep_compiler_assertion_and_isa_guards(self):
        self.assertEqual(gate.macro_proof(MACROS, "scalar", "flow")["isa"], {})
        self.assertEqual(gate.macro_proof(NATIVE_MACROS, "native", "flow")["gcc"], "15.2.0")
        self.assertEqual(gate.FLOW_PRODUCTS, {"components/push_relabel.hpp", "graph/max_flow.hpp"})
        self.assertEqual(gate.FLOW_REQUIRED_UNITS, {
            "test/unit/max_flow.cpp", "test/unit/flow_capacity.cpp", "test/unit/standalone.cpp"})
        self.assertEqual(gate.SCOPES["flow"][2:], (2, 3))
        for raw, mode in ((MACROS, "native"), (NATIVE_MACROS, "scalar"),
                          (NATIVE_MACROS.replace(b"#define __BMI2__ 1\n", b""), "native"),
                          (NATIVE_MACROS + b"#define NDEBUG 1\n", "native"),
                          (MACROS.replace(b"#define assert(expr) check(expr)\n", b""), "scalar"),
                          (MACROS.replace(b"__GNUC__ 15", b"__GNUC__ 14"), "scalar"),
                          (MACROS.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "scalar"),
                          (MACROS.replace(b"__GNUC_PATCHLEVEL__ 0", b"__GNUC_PATCHLEVEL__ 1"), "scalar"),
                          (MACROS.replace(b"ONLINE_JUDGE 1", b"ONLINE_JUDGE 0"), "scalar"),
                          (MACROS.replace(b"ATCODER 1", b"ATCODER 0"), "scalar"),
                          (MACROS.replace(b"__x86_64__ 1", b"__x86_64__ 0"), "scalar")):
            with self.subTest(raw=raw, mode=mode), self.assertRaises(ValueError):
                gate.macro_proof(raw, mode, "flow")
        for mode in gate.MODES:
            self.assertEqual(gate.mode_flags(mode, "flow"), gate.COMMON + gate.MODES[mode])
            self.assertNotIn("-DMATRIX_DISABLE_AVX2", gate.mode_flags(mode, "flow"))
            self.assertNotIn("-DNDEBUG", gate.mode_flags(mode, "flow"))
            self.assertNotIn("MATRIX_DISABLE_AVX2", gate.macro_proof(
                NATIVE_MACROS if mode == "native" else MACROS, mode, "flow"))
        self.assertIn("-fno-lto", gate.COMMON)

    def test_flow_driver_runs_every_target_with_checked_mode_flags(self):
        selected, plan = gate.selected(self.root, *self.push())
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="flow-driver-") as directory:
            directory = Path(directory)
            compiler, cc1 = directory / "g++-15", directory / "cc1plus"
            compiler.write_bytes(b"compiler fixture")
            cc1.write_bytes(b"cc1 fixture")
            commands = gate.Commands(directory)
            compiled, executed, probes = [], [], []

            def run(command, *, source=b"", seconds=gate.COMMAND_SECONDS):
                command = [str(value) for value in command]
                commands.count += 1
                if command == [str(compiler), "-dumpfullversion"]:
                    return b"15.2.0\n"
                if command == [str(compiler), "-print-prog-name=cc1plus"]:
                    return str(cc1).encode() + b"\n"
                if command[0] != str(compiler):
                    self.assertEqual(len(command), 1)
                    executed.append(command[0])
                    return b""
                mode = "native" if "-march=native" in command else "scalar"
                flags = gate.mode_flags(mode, "flow")
                self.assertEqual(command[1:1 + len(flags)], flags)
                if "-dM" in command:
                    self.assertEqual(source, b"#include <cassert>\n")
                    probes.append(mode)
                    return NATIVE_MACROS if mode == "native" else MACROS
                self.assertEqual(command[1 + len(flags):3 + len(flags)], ["-I", str(self.root)])
                source_path, binary = Path(command[-3]), Path(command[-1])
                if source_path.is_relative_to(self.root):
                    target = source_path.relative_to(self.root).as_posix()
                else:
                    target = source_path.read_text().split('"')[1]
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source_path)} | {str(self.root / name) for name in names}
                depfile = Path(command[command.index("-MF") + 1])
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                binary.write_bytes(b"test binary fixture")
                compiled.append((mode, target, str(binary)))
                return b""

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands), \
                 patch.object(commands, "run", side_effect=run), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "emit") as emit:
                gate.execute(self.root, selected, plan, directory)
            self.assertEqual(probes, ["native", "scalar"])
            self.assertEqual([(mode, target) for mode, target, _ in compiled],
                             [(mode, target) for mode in ("native", "scalar")
                              for group in ("headers", "unit") for target in plan[group]])
            self.assertEqual(executed, [binary for _, _, binary in compiled])
            self.assertEqual(commands.count, 24)
            self.assertEqual(emit.call_args.args, ("gate_completed",))

    def test_normal_flow_event_does_not_plan_or_run_cpp(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        self.write("components/push_relabel.hpp", "#pragma once\nusing normal = int;\n")
        head = self.commit("ordinary flow event", bind=False)
        event, context = self.push(head, route.MAIN_REF)
        with tempfile.TemporaryDirectory(prefix="flow-event-") as directory:
            event_path = Path(directory) / "event.json"
            event_path.write_text(json.dumps(event))
            with patch.dict(os.environ, {**context, "GITHUB_EVENT_PATH": str(event_path)}), \
                 patch.object(gate, "__file__", str(self.root / ".verify-helper/intrinsic_gate.py")), \
                 patch.object(gate, "build_affected_plan", side_effect=AssertionError("unexpected planner")), \
                 patch.object(gate, "execute", side_effect=AssertionError("unexpected C++")), \
                 patch.object(gate, "emit") as emit:
                gate.main()
            emit.assert_called_once_with("gate_skipped", reason="normal event; no extra C++ commands")


class IntrinsicCommandTests(unittest.TestCase):
    def test_clean_environment_and_nonzero_exit_fail_closed(self):
        directory = Path("/tmp/intrinsic-command-fixture")
        commands = gate.Commands(directory)
        self.assertEqual(commands.environment, {"PATH": "/usr/bin:/bin", "HOME": str(directory),
                         "TMPDIR": str(directory), "LC_ALL": "C", "TZ": "UTC"})
        process = Mock(returncode=3)
        process.communicate.return_value = (b"assertion failed", None)
        with patch.object(gate.subprocess, "Popen") as popen, patch.object(gate, "emit"):
            popen.return_value.__enter__.return_value = process
            with self.assertRaisesRegex(ValueError, "Command failed: 1"):
                commands.run(["fixture-binary"])
            self.assertEqual(popen.call_args.kwargs["env"], commands.environment)
            self.assertTrue(popen.call_args.kwargs["start_new_session"])

    def test_timeout_kills_group_and_cannot_be_reported_as_success(self):
        commands = gate.Commands(Path("/tmp/intrinsic-command-fixture"))
        process = Mock(pid=1234, returncode=0)
        process.communicate.side_effect = [subprocess.TimeoutExpired(["fixture-binary"], 1), (b"", None)]
        with patch.object(gate.subprocess, "Popen") as popen, patch.object(gate.os, "killpg") as kill, \
             patch.object(gate, "emit") as emit:
            popen.return_value.__enter__.return_value = process
            with self.assertRaisesRegex(ValueError, "Command failed: 1"):
                commands.run(["fixture-binary"], seconds=1)
            kill.assert_called_once_with(1234, signal.SIGKILL)
            self.assertTrue(emit.call_args.kwargs["timed_out"])

    def test_command_and_total_time_budgets_fail_before_launch(self):
        commands = gate.Commands(Path("/tmp/intrinsic-command-fixture"))
        with patch.object(gate.subprocess, "Popen") as popen:
            commands.count = gate.MAX_COMMANDS
            with self.assertRaisesRegex(ValueError, "Command budget exceeded"):
                commands.run(["fixture-binary"])
            commands.count, commands.deadline = 0, 0
            with self.assertRaisesRegex(ValueError, "Total runtime budget exceeded"):
                commands.run(["fixture-binary"])
            popen.assert_not_called()


if __name__ == "__main__":
    unittest.main()
