"""Fresh exact-head intrinsic checks. No benchmarks, caches, artifacts, or network."""

import json
import os
from pathlib import Path
import re
import signal
import subprocess
import tempfile
import time

from affected import SourceTree, build_affected_plan, checked_source, digest, require
from no_cache_route import select_event


PRODUCTS = {"components/dynamic_bit_vector.hpp", "data_structure/dynamic_wavelet_matrix.hpp"}
REQUIRED_UNITS = {"test/unit/" + name + ".cpp" for name in (
    "dynamic_bit_vector", "dynamic_wavelet_core", "dynamic_wavelet_count",
    "dynamic_wavelet_matrix", "standalone",
)}
MATRIX_PRODUCTS = {"components/matrix_arithmetic.hpp", "math/matrix.hpp"}
MATRIX_REQUIRED_UNITS = {"test/unit/" + name + ".cpp" for name in (
    "matrix", "matrix_characteristic", "standalone",
)}
FLOW_PRODUCTS = {"components/push_relabel.hpp", "graph/max_flow.hpp"}
FLOW_REQUIRED_UNITS = {"test/unit/" + name + ".cpp" for name in (
    "max_flow", "flow_capacity", "standalone",
)}
SCOPES = {
    "dwm": (PRODUCTS, REQUIRED_UNITS, 4, 8),
    "matrix": (MATRIX_PRODUCTS, MATRIX_REQUIRED_UNITS, 2, 3),
    "flow": (FLOW_PRODUCTS, FLOW_REQUIRED_UNITS, 2, 3),
}
COMMON = ["-O2", "-std=gnu++23", "-ftrivial-auto-var-init=zero", "-fmodules",
          "-DONLINE_JUDGE", "-DATCODER", "-fno-lto"]
MODES = {
    "native": ["-march=native"],
    "scalar": ["-march=x86-64", "-mtune=generic", "-mno-avx", "-mno-avx2",
               "-mno-avx512f", "-mno-bmi", "-mno-bmi2",
               "-fno-tree-loop-vectorize", "-fno-tree-slp-vectorize"],
}
MAX_COMMANDS, MAX_LOG_BYTES, COMMAND_SECONDS = 64, 1024 * 1024, 120
JOB_SECONDS = 18 * 60
LOG_BYTES = 0


def emit(event, **values):
    global LOG_BYTES
    line = json.dumps({"event": event, **values}, sort_keys=True, ensure_ascii=True)
    LOG_BYTES += len(line) + 1
    require(LOG_BYTES <= MAX_LOG_BYTES, "Normal log budget exceeded")
    print(line, flush=True)


def file_proof(path):
    data = path.read_bytes()
    return {"path": str(path), "bytes": len(data), "sha256": digest(data)}


class Commands:
    def __init__(self, directory):
        self.directory, self.count = directory, 0
        self.deadline = time.monotonic() + JOB_SECONDS
        self.environment = {"PATH": "/usr/bin:/bin", "HOME": str(directory),
                            "TMPDIR": str(directory), "LC_ALL": "C", "TZ": "UTC"}

    def run(self, command, *, source=b"", seconds=COMMAND_SECONDS):
        self.count += 1
        require(self.count <= MAX_COMMANDS, "Command budget exceeded")
        timeout = min(seconds, self.deadline - time.monotonic())
        require(timeout > 0, "Total runtime budget exceeded")
        command = [str(value) for value in command]
        emit("command_start", id=self.count, argv=command, cwd=str(self.directory),
             timeout_seconds=round(timeout, 3), stdin_sha256=digest(source))
        start = time.monotonic()
        try:
            with subprocess.Popen(command, cwd=self.directory, env=self.environment,
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT, start_new_session=True) as process:
                timed_out = False
                try:
                    output, _ = process.communicate(source, timeout=timeout)
                except subprocess.TimeoutExpired:
                    timed_out = True
                    try:
                        os.killpg(process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                    output, _ = process.communicate()
        except OSError as error:
            emit("command_end", id=self.count, returncode=None, error=str(error),
                 seconds=round(time.monotonic() - start, 6))
            raise
        emit("command_end", id=self.count, returncode=process.returncode, timed_out=timed_out,
             seconds=round(time.monotonic() - start, 6), output_bytes=len(output),
             output_sha256=digest(output), output_tail=output[-4096:].decode(errors="replace"))
        require(not timed_out and process.returncode == 0, "Command failed: " + str(self.count))
        return output


def selected(root, event, context):
    route = select_event(root, event, context)
    if route["mode"] == "normal":
        return route, None
    plan = build_affected_plan(root, route["base"], route["head"], expected_ref=route["ref"])
    scope_mode(plan)
    return route, plan


def scope_mode(plan):
    changed = set(plan["changed_paths"])
    matched = [family for family, (products, _, _, _) in SCOPES.items() if products & changed]
    require(len(matched) == 1, "Production changes must select exactly one reviewed intrinsic mode")
    family = matched[0]
    products, required, max_headers, max_units = SCOPES[family]
    require(all(not (other_products & set(plan["headers"]))
                for other, (other_products, _, _, _) in SCOPES.items() if other != family),
            "Mixed production intrinsic scopes require separate review")
    require(required <= set(plan["unit"]), "Required " + family.upper() + " units absent from affected scope")
    require(products <= set(plan["headers"]), "Production " + family.upper() + " headers absent from affected scope")
    require(len(plan["unit"]) <= max_units and len(plan["headers"]) <= max_headers,
            "Affected scope exceeds reviewed gate budget; review before expanding")
    return family


def mode_flags(mode, family="dwm"):
    require(family in SCOPES and mode in MODES, "Unknown intrinsic mode")
    flags = COMMON + MODES[mode]
    if family == "matrix" and mode == "scalar":
        flags += ["-DMATRIX_DISABLE_AVX2"]
    return flags


def macro_proof(raw, mode, family="dwm"):
    require(family in SCOPES, "Unknown intrinsic mode")
    macros = dict(re.findall(rb"^#define (\S+)(?: ([^\n]*))?$", raw, re.MULTILINE))
    require(b"NDEBUG" not in macros and b"assert(expr)" in macros, "Assertions are not enabled")
    for name, value in ((b"ONLINE_JUDGE", b"1"), (b"ATCODER", b"1"),
                        (b"__GNUC__", b"15"), (b"__GNUC_MINOR__", b"2"),
                        (b"__GNUC_PATCHLEVEL__", b"0"), (b"__x86_64__", b"1")):
        require(macros.get(name) == value, "Wrong compiler/target macro: " + name.decode())
    isa = {key.decode(): value.decode() for key, value in macros.items()
           if key.startswith((b"__AVX", b"__BMI"))}
    if mode == "native":
        require(isa.get("__AVX2__") == "1" and isa.get("__BMI2__") == "1",
                "Native runner must expose AVX2 and BMI2; do not retry or downgrade")
    else:
        require(mode == "scalar" and not isa, "Explicit scalar AVX/BMI disables did not take effect")
    proof = {"sha256": digest(raw), "isa": isa, "assertions": True,
             "ONLINE_JUDGE": "1", "ATCODER": "1", "gcc": "15.2.0"}
    if family == "matrix":
        disabled = macros.get(b"MATRIX_DISABLE_AVX2")
        require(disabled == b"1" if mode == "scalar" else disabled is None,
                "Matrix runtime AVX2 dispatch does not match the selected mode")
        proof["MATRIX_DISABLE_AVX2"] = "1" if mode == "scalar" else None
    return proof


def dependency_proof(root, tree, depfile, source, target, generated=False, family="dwm"):
    require(family in SCOPES, "Unknown intrinsic mode")
    products, required, _, _ = SCOPES[family]
    raw = depfile.read_bytes()
    text = raw.decode().replace("\\\n", " ")
    require(text.startswith("gate-output: "), "Unexpected dependency rule")
    tokens = text.removeprefix("gate-output: ").split()
    require(0 < len(tokens) <= 256 and all(re.fullmatch(r"/[A-Za-z0-9_./+-]+", item) for item in tokens),
            "Unsupported dependency path or dependency budget exceeded")
    paths = {Path(item).resolve(strict=True) for item in tokens}
    require(source.resolve() in paths, "Compiler did not report the actual source")
    closure = tree.closure(target)
    expected_products = products & closure
    if target in required:
        expected_products = products if not target.endswith("/dynamic_bit_vector.cpp") else {
            "components/dynamic_bit_vector.hpp"}
    found, project = set(), []
    for path in sorted(paths):
        if generated and path == source.resolve():
            continue
        require(path.is_relative_to(root), "Non-system dependency escaped production checkout: " + str(path))
        name = path.relative_to(root).as_posix()
        require(name in closure, "Dependency was not resolved by reviewed source selector: " + name)
        require(path.read_bytes() == tree.content(name), "Dependency bytes differ from exact head: " + name)
        found.add(name)
        project.append({"project_path": name, **file_proof(path)})
    require(expected_products <= found,
            "Compilation did not consume the current production " + family.upper() + " headers")
    return {"depfile": file_proof(depfile), "source": file_proof(source),
            "project_dependencies": project, "production_headers": sorted(expected_products),
            "system_headers": "excluded by -MMD"}


def header_source(name):
    return (f'#include "{name}"\n#include "{name}"\n'
            '#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) '
            '|| defined(all) || defined(sz) || defined(uint) || defined(NDEBUG)\n'
            '#error "Standalone header or assertion contract changed"\n#endif\n'
            'int main() {}\n')


def execute(root, route, plan, directory):
    family = scope_mode(plan)
    commands = Commands(directory)
    compiler = Path("/usr/bin/g++-15").resolve(strict=True)
    require(commands.run([compiler, "-dumpfullversion"], seconds=30).strip() == b"15.2.0",
            "Exact GCC 15.2.0 is required; do not substitute or install another toolchain")
    cc1 = Path(commands.run([compiler, "-print-prog-name=cc1plus"], seconds=30).decode().strip())
    require(cc1.is_absolute(), "Compiler did not resolve an absolute cc1plus")
    emit("compiler", compiler=file_proof(compiler), cc1plus=file_proof(cc1.resolve(strict=True)),
         environment=commands.environment, lto=False, assertions=True)
    tree = SourceTree(root, route["head"])
    emit("scope", family=family, route=route, plan_sha256=plan["plan_sha256"], headers=plan["headers"],
         units=plan["unit"], source_controls={name: digest(tree.content(name)) for name in (
             ".verify-helper/intrinsic_gate.py", ".verify-helper/intrinsic_gate_test.py",
             ".github/workflows/verify.yml")},
         max_commands=MAX_COMMANDS, max_seconds=JOB_SECONDS, max_log_bytes=MAX_LOG_BYTES)
    for mode in MODES:
        flags = mode_flags(mode, family)
        raw = commands.run([compiler, *flags, "-dM", "-E", "-x", "c++", "-"],
                           source=b"#include <cassert>\n", seconds=30)
        emit("macro_proof", family=family, mode=mode, flags=flags, **macro_proof(raw, mode, family))
        for group in ("headers", "unit"):
            for index, target in enumerate(plan[group]):
                stem = directory / (mode + "-" + group + "-" + str(index))
                source = root / target
                if group == "headers":
                    source = stem.with_suffix(".cpp")
                    source.write_text(header_source(target))
                depfile, binary = stem.with_suffix(".d"), stem.with_suffix(".bin")
                commands.run([compiler, *flags, "-I", root, "-MMD", "-MF", depfile,
                              "-MT", "gate-output", source, "-o", binary])
                proof = dependency_proof(root, tree, depfile, source, target, group == "headers", family)
                binary_before = file_proof(binary)
                emit("binary", mode=mode, group=group, target=target,
                     binary=binary_before, dependencies=proof)
                commands.run([binary])
                require(file_proof(binary) == binary_before, "Test binary changed during execution")
                require(dependency_proof(root, tree, depfile, source, target, group == "headers", family) == proof,
                        "Source dependency changed during execution")
        emit("mode_completed", mode=mode, unit_count=len(plan["unit"]), header_count=len(plan["headers"]))
    checked_source(root, route["base"], route["head"])
    emit("gate_completed", head=route["head"], source_head=route["source_head"],
         base=route["base"], source_digest=route["source_digest"], commands=commands.count,
         plan_sha256=plan["plan_sha256"], timings="command durations only; no performance comparison",
         unaffected_checks="not_run", cache_written=False, artifacts_uploaded=False)


def main():
    root = Path(__file__).resolve().parent.parent
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text())
    route, plan = selected(root, event, os.environ)
    if plan is None:
        emit("gate_skipped", reason="normal event; no extra C++ commands")
        return
    with tempfile.TemporaryDirectory(prefix="intrinsic-gate-") as directory:
        execute(root, route, plan, Path(directory).resolve())


if __name__ == "__main__":
    main()
