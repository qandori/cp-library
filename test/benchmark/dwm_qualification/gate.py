#!/usr/bin/env python3
"""Source-only draft of the phase-two correctness gate. No performance claims.

Invocation (also accepted with explicit --platform linux|macos):
  gate.py REPOSITORY EMPTY_WORK_DIRECTORY first-wave native|scalar|mac
  gate.py REPOSITORY EMPTY_WORK_DIRECTORY sanitizer-a sanitizer
  gate.py REPOSITORY EMPTY_WORK_DIRECTORY sanitizer-b sanitizer
  gate.py REPOSITORY EMPTY_WORK_DIRECTORY combinations native|scalar|mac

Package contract, checked before any compilation:
  --package-root contains bench.cpp, caller.cpp, escape.cpp, verify_oracle.py,
  plan.json, patches/, and the five supplemental tests/. Optional combination
  gates additionally require combinations.json.
  The repository's three original unit sources are used directly, never edited.
  The frozen plan pins baseline, variant component order, patch and header hashes,
  and package.source_sha256 for every used packaged source, including this gate.
  Baseline headers are obtained with git archive at plan.baseline, not by assuming
  the qualification commit is itself the baseline. Patches use --fuzz=0.

First-wave invocations are Linux first-wave/native, Linux first-wave/scalar,
Mac first-wave/mac, and Linux sanitizer-a/sanitizer plus sanitizer-b/sanitizer.
Sanitizers exercise native AVX2/BMI2 paths on all seven measured individuals.
Every required invocation
must succeed on the same package before timing. Combinations is an explicit,
later gate, not an implicitly passed or silently skipped first-wave gate.

All generated files, compiler module state, captures, and trace dumps stay in
one TemporaryDirectory. Nothing is uploaded or saved as an artifact/cache.
Every subprocess result, including failures, goes to ordinary stdout as JSONL.
Correctness logs have a 24 MiB cap; the separate timing runner has an 8 MiB cap.
These are different budgets, not a shared or interchangeable limit. A 128 KiB
part of the correctness cap is reserved for terminal failure/result records.

Every JSONL record has run_id, run_attempt, invocation_id, and a consecutive seq.
The initial digest is 64 zeroes. Hash bytes.fromhex(previous digest) followed by
canonical UTF-8 JSON of the record without event_chain_sha256 (sorted keys,
separators comma/colon, ensure_ascii=True, allow_nan=False). The resulting SHA256
is event_chain_sha256. Require contiguous sequences and a final gate_result with
matching record_count_including_result and final digest; a missing final record,
chain gap, or log_complete=false makes the retained log incomplete.
"""

import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import signal
import subprocess
import sys
import tarfile
import tempfile
import time
import traceback


HEADERS = (
    "components/dynamic_bit_vector.hpp",
    "data_structure/dynamic_wavelet_matrix.hpp",
)
INDIVIDUALS = (
    "neighbors", "uniformneighbors", "selectprefix", "rangeprefix",
    "pdep", "avxbuild", "leaf",
)
COMBINATIONS = {
    "queries": ["neighbors", "selectprefix", "rangeprefix"],
    "queries_uniform": ["uniformneighbors", "selectprefix", "rangeprefix"],
    "simd": ["pdep", "avxbuild"],
    "combo": ["neighbors", "selectprefix", "rangeprefix", "pdep", "avxbuild"],
    "combo_uniform": ["uniformneighbors", "selectprefix", "rangeprefix", "pdep", "avxbuild"],
    "combo_leaf": ["uniformneighbors", "selectprefix", "rangeprefix", "pdep", "avxbuild", "leaf"],
    "selectkernels": ["pdep", "selectprefix"],
}
SUITES = (
    "dynamic_architecture", "dynamic_wavelet_neighbors", "dynamic_leaf",
    "review_edge_cases", "simd_edges", "dynamic_bit_vector",
    "dynamic_wavelet_matrix", "dynamic_wavelet_count",
)
REPO_SUITES = SUITES[-3:]
COMPILE_TIMEOUT = 180
SUITE_TIMEOUT = 240
SANITIZER_SUITE_TIMEOUT = 600
REPLAY_TIMEOUT = 30
ORACLE_TIMEOUT = 30
PROVENANCE_TIMEOUT = 30
STREAM_LIMIT = 65536
TOTAL_LOG_LIMIT = 24 * 1024 * 1024
TERMINAL_LOG_RESERVE = 128 * 1024
TIMING_LOG_LIMIT_REFERENCE = 8 * 1024 * 1024
JOB_DEADLINE_SECONDS = 39 * 60
CORRECTNESS_SEEDS = (20261017, 1709)
COMMON_FLAGS = ["-O2", "-std=gnu++23", "-fmodules", "-ftrivial-auto-var-init=zero", "-DONLINE_JUDGE", "-DATCODER"]
SCALAR_FLAGS = ["-march=x86-64", "-mtune=generic", "-mno-avx", "-mno-avx2", "-mno-avx512f",
                "-mno-bmi", "-mno-bmi2", "-fno-tree-loop-vectorize", "-fno-tree-slp-vectorize"]


class GateFailure(RuntimeError):
    pass


def require(ok, message):
    if not ok:
        raise GateFailure(message)


def sha256(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


class Log:
    def __init__(self):
        self.bytes = 0
        self.seq = 0
        self.chain = "0" * 64
        self.incomplete = False
        self.run_id = os.environ.get("GITHUB_RUN_ID") or "local-" + str(time.time_ns())
        self.run_attempt = os.environ.get("GITHUB_RUN_ATTEMPT", "1")
        self.invocation_id = hashlib.sha256((self.run_id + ":" + self.run_attempt + ":" +
                                            str(os.getpid()) + ":" + str(time.time_ns())).encode()).hexdigest()[:24]

    def emit(self, event, **data):
        def canonical(value):
            return json.dumps(value, sort_keys=True, separators=(",", ":"),
                              ensure_ascii=True, allow_nan=False).encode("utf-8")

        next_seq = self.seq + 1
        if event == "gate_result":
            data.update(record_count_including_result=next_seq, record_digest_before_result=self.chain,
                        jsonl_bytes_before_result=self.bytes, log_complete=not self.incomplete,
                        correctness_log_limit_bytes=TOTAL_LOG_LIMIT,
                        separate_timing_log_limit_bytes=TIMING_LOG_LIMIT_REFERENCE)
        record = dict(event=event, **data, run_id=self.run_id, run_attempt=self.run_attempt,
                      invocation_id=self.invocation_id, seq=next_seq, previous_event_chain_sha256=self.chain)
        next_chain = hashlib.sha256(bytes.fromhex(self.chain) + canonical(record)).hexdigest()
        record["event_chain_sha256"] = next_chain
        raw = canonical(record) + b"\n"
        ceiling = TOTAL_LOG_LIMIT if event in ("fatal", "gate_result") else TOTAL_LOG_LIMIT - TERMINAL_LOG_RESERVE
        if self.bytes + len(raw) > ceiling:
            self.incomplete = True
            raise GateFailure("ordinary-stdout log budget exceeded; no qualification")
        sys.stdout.buffer.write(raw)
        sys.stdout.buffer.flush()
        self.bytes += len(raw)
        self.seq, self.chain = next_seq, next_chain


class Commands:
    def __init__(self, temp, log):
        self.temp, self.log = temp, log
        self.count = 0
        self.deadline = time.monotonic() + JOB_DEADLINE_SECONDS
        self.by_stage = collections.Counter()
        self.env = os.environ.copy()
        # Do not inherit options that disable assertions, redirect sanitizer logs,
        # inject headers, or place compiler/module/python caches outside temp.
        for key in ("CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "OBJC_INCLUDE_PATH",
                    "GCC_EXEC_PREFIX", "COMPILER_PATH", "LIBRARY_PATH", "LD_PRELOAD",
                    "DYLD_INSERT_LIBRARIES", "ASAN_OPTIONS", "UBSAN_OPTIONS",
                    "LSAN_OPTIONS", "DWM_EXPECTED_HASH", "DWM_EXPECTED_FINAL_HASH",
                    "DWM_VALIDATE_REPLAYS", "PYTHONOPTIMIZE"):
            self.env.pop(key, None)
        self.env.update(TMPDIR=str(temp), TMP=str(temp), TEMP=str(temp),
                        PYTHONDONTWRITEBYTECODE="1", CCACHE_DISABLE="1",
                        CLANG_MODULE_CACHE_PATH=str(temp / "clang-modules"), LC_ALL="C")

    def run(self, args, *, stage, timeout, env=None, allowed=(0,), input_text=None):
        self.count += 1
        self.by_stage[stage] += 1
        args = [str(x) for x in args]
        prefix = self.temp / ("capture-%06d" % self.count)
        stdout_path, stderr_path = prefix.with_suffix(".out"), prefix.with_suffix(".err")
        started = time.monotonic()
        timeout = min(timeout, self.deadline - started)
        require(timeout > 0, "39-minute gate deadline reached; checks incomplete")
        changes = env or {}
        child_env = dict(self.env, **changes)
        timed_out, rc, error = False, None, None
        self.log.emit("command_start", command_id=self.count, stage=stage, argv=args,
                      timeout_seconds=timeout, environment_changes=changes, cwd=str(self.temp))
        with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
            try:
                process = subprocess.Popen(args, stdout=stdout, stderr=stderr,
                                           stdin=subprocess.PIPE if input_text is not None else subprocess.DEVNULL,
                                           env=child_env, cwd=self.temp, start_new_session=True)
                try:
                    process.communicate(None if input_text is None else input_text.encode(), timeout=timeout)
                except subprocess.TimeoutExpired:
                    timed_out = True
                    os.killpg(process.pid, signal.SIGKILL)
                    process.communicate()
                rc = process.returncode
            except OSError as exc:
                error = repr(exc)
        streams = {}
        overflow = False
        for label, path in (("stdout", stdout_path), ("stderr", stderr_path)):
            size = path.stat().st_size
            with path.open("rb") as f:
                raw = f.read(STREAM_LIMIT + 1)
            truncated = size > STREAM_LIMIT
            overflow |= truncated
            streams[label] = raw[:STREAM_LIMIT].decode("utf-8", errors="replace")
            streams[label + "_bytes"] = size
            streams[label + "_sha256"] = sha256(path)
            streams[label + "_truncated"] = truncated
        self.log.emit("command", command_id=self.count, stage=stage, argv=args,
                      environment_changes=changes, timeout_seconds=timeout,
                      elapsed_seconds=time.monotonic() - started, returncode=rc,
                      timed_out=timed_out, launch_error=error, **streams)
        stdout_path.unlink()
        stderr_path.unlink()
        require(not overflow, "command output exceeded the bound; gate fails rather than concealing output")
        require(not timed_out and error is None and rc in allowed,
                "command %d failed: %s" % (self.count, " ".join(args)))
        return streams["stdout"]


def selected_variants(group):
    if group == "individuals":
        return ("baseline",) + INDIVIDUALS
    if group == "combinations":
        return ("baseline",) + tuple(COMBINATIONS)
    if group == "sanitizer-a":
        return ("baseline", "neighbors", "uniformneighbors", "selectprefix")
    if group == "sanitizer-b":
        return ("rangeprefix", "pdep", "avxbuild", "leaf")
    raise GateFailure("unknown explicit gate group " + group)


def compile_flags(args):
    common = ["-O2", "-std=gnu++2b", "-ftrivial-auto-var-init=zero", "-DONLINE_JUDGE", "-DATCODER", "-UNDEBUG"]
    if args.sanitizer:
        return ["-O1", "-g", "-std=gnu++23", "-ftrivial-auto-var-init=zero", "-DONLINE_JUDGE", "-DATCODER",
                "-UNDEBUG", "-march=native", "-fno-omit-frame-pointer",
                "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    if args.platform == "macos":
        # Do not pass x86-only, GCC-only, or Linux taskset options on macOS.
        return common + ["-arch", "arm64"]
    # These are the exact frozen timing flags, with an explicit assertion guard.
    return args.compiler_flags[args.mode] + ["-UNDEBUG"]


def source_closure(source, include_roots):
    """Hash every quoted project include, resolving relative includes as the compiler does."""
    found, pending = {}, [Path(source).resolve()]
    while pending:
        path = pending.pop()
        if str(path) in found:
            continue
        require(path.is_file(), "missing source dependency " + str(path))
        found[str(path)] = sha256(path)
        for name in re.findall(r'^\s*#\s*include\s*"([^"]+)"', path.read_text(), re.M):
            candidates = [path.parent / name] + [root / name for root in include_roots]
            target = next((candidate.resolve() for candidate in candidates if candidate.is_file()), None)
            require(target is not None, "unresolved quoted include %s in %s" % (name, path))
            pending.append(target)
    return found


def compiler_dependencies(depfile, source, output_name, cwd, log, *, variant, root, predicted):
    require(depfile.is_file(), "compiler did not emit its mandatory dependency file")
    raw = depfile.read_text().replace("\\\n", " ")
    target, colon, body = raw.partition(":")
    require(colon and target.strip() == output_name, "unexpected compiler dependency target")
    dependencies = []
    for name in shlex.split(body, comments=False, posix=True):
        path = Path(name)
        if not path.is_absolute():
            path = cwd / path
        path = path.resolve()
        require(path.is_file(), "unreadable actual compiler dependency " + str(path))
        if path not in dependencies:
            dependencies.append(path)
    require(Path(source).resolve() in dependencies, "compiler dependency list omitted the compiled source")
    expected_headers = [h for h in HEADERS if any(str(p).endswith("/" + h) for p in predicted)]
    for header in HEADERS:
        observed = [p for p in dependencies if str(p).endswith("/" + header)]
        if header in expected_headers:
            require(root is not None and observed == [(root / header).resolve()],
                    "compiler selected another variant/baseline for " + header + ": " + str(observed))
        else:
            require(not observed, "unexpected public header dependency " + header)
    log.emit("actual_compiler_dependencies", variant=variant, executable=output_name,
             dependency_file_sha256=sha256(depfile), expected_public_headers=expected_headers,
             dependencies={str(path): sha256(path) for path in dependencies})
    depfile.unlink()


def unit_source(args, suite):
    return (args.repo / "test/unit" if suite in REPO_SUITES else args.package_root / "tests") / (suite + ".cpp")


def package_variants(args, temp, log, command):
    plan = json.loads(args.plan.read_text())
    require(plan.get("frozen") is True, "proposal is not frozen; source-only plans may not execute")
    args.expected_base = plan["baseline"]
    require(re.fullmatch(r"[0-9a-f]{40}", args.expected_base), "baseline must be an exact commit")
    hashes = plan["package"]["source_sha256"]
    required = {"bench.cpp", "caller.cpp", "escape.cpp", "verify_oracle.py", Path(__file__).name}
    if args.group == "combinations":
        required.add("combinations.json")
    required.update("tests/" + s + ".cpp" for s in SUITES if s not in REPO_SUITES)
    required.update("patches/" + name + ".patch" for name in INDIVIDUALS)
    require(required <= set(hashes), "frozen package omits required gate sources: " + str(sorted(required - set(hashes))))
    for name, expected in hashes.items():
        relative = Path(name)
        require(not relative.is_absolute() and ".." not in relative.parts and "\\" not in name,
                "unsafe package manifest path " + name)
        path = args.package_root / relative
        require(path.is_file() and not path.is_symlink(), "missing/symlink packaged source " + name)
        require(sha256(path) == expected, "frozen package source hash mismatch " + name)
    require((args.package_root / Path(__file__).name).resolve() == Path(__file__).resolve(),
            "the executing gate is not the frozen packaged gate")
    compiler = plan["compiler"]
    require(compiler["common_flags"] == COMMON_FLAGS and compiler["native_flags"] == ["-march=native"]
            and compiler["scalar_flags"] == SCALAR_FLAGS and compiler.get("assertions") is True,
            "frozen timing flags differ from the reviewed correctness gate")
    args.compiler_flags = {mode: compiler["common_flags"] + compiler[mode + "_flags"] for mode in ("native", "scalar")}
    combinations = {}
    if args.group == "combinations":
        combinations = json.loads((args.package_root / "combinations.json").read_text())
        require(set(combinations) == set(COMBINATIONS), "combinations.json has an unexpected/missing composition")
        for name, components in COMBINATIONS.items():
            require(combinations[name]["components"] == components, "composition order changed for " + name)
    manifest = dict(plan["variants"])
    require(set(manifest) >= {"baseline", *INDIVIDUALS}, "missing first-wave individuals in frozen plan")
    for name, definition in combinations.items():
        require(name not in manifest or manifest[name] == definition, "conflicting combination definitions " + name)
        manifest[name] = definition
    names = selected_variants(args.group)
    head = command.run(["git", "-C", args.repo, "rev-parse", "HEAD"], stage="source_provenance", timeout=PROVENANCE_TIMEOUT).strip()
    dirty = command.run(["git", "-C", args.repo, "diff", "--name-only", "HEAD", "--", "components", "data_structure", "test/unit"],
                        stage="source_provenance", timeout=PROVENANCE_TIMEOUT)
    require(not dirty.strip(), "checkout source/tests are modified")
    log.emit("frozen_package", plan_sha256=sha256(args.plan), source_sha256=hashes,
             baseline=args.expected_base, checkout_head=head,
             existing_repo_tests={str(unit_source(args, s)): sha256(unit_source(args, s)) for s in REPO_SUITES})
    archive = temp / "baseline.tar"
    command.run(["git", "-C", args.repo, "archive", "--format=tar", "--output", archive,
                 args.expected_base, "components", "data_structure"], stage="materialize_base", timeout=PROVENANCE_TIMEOUT)
    log.emit("baseline_archive", sha256=sha256(archive), bytes=archive.stat().st_size)
    args.baseline_root = temp / "baseline-source"
    args.baseline_root.mkdir()
    with tarfile.open(archive, "r:") as tar:
        for member in tar.getmembers():
            relative = Path(member.name)
            require(not relative.is_absolute() and ".." not in relative.parts and relative.parts
                    and relative.parts[0] in ("components", "data_structure"), "unsafe git archive path")
            require(member.isdir() or member.isfile(), "unsupported archive entry; no links/devices")
            dest = args.baseline_root / relative
            if member.isdir():
                dest.mkdir(parents=True, exist_ok=True)
            else:
                dest.parent.mkdir(parents=True, exist_ok=True)
                with tar.extractfile(member) as src, dest.open("wb") as out:
                    shutil.copyfileobj(src, out)
    archive.unlink()
    args.variants_root = temp / "variants"
    for name in names:
        definition = manifest[name]
        require(definition["base"] == args.expected_base, "variant baseline mismatch " + name)
        components = [] if name == "baseline" else [name] if name in INDIVIDUALS else COMBINATIONS[name]
        require(definition["components"] == components, "variant component order changed " + name)
        dest = args.variants_root / name
        shutil.copytree(args.baseline_root, dest)
        for component in components:
            patch = args.package_root / "patches" / (component + ".patch")
            require(sha256(patch) == definition["component_patch_sha256"][component], "patch digest mismatch " + component)
            command.run(["patch", "--batch", "--forward", "--fuzz=0", "-d", dest, "-p1", "-i", patch],
                        stage="apply_patch", timeout=PROVENANCE_TIMEOUT)
        for header in HEADERS:
            path = dest / header
            require(path.is_file() and not path.is_symlink(), "missing/non-materialized candidate header " + str(path))
            actual = sha256(path)
            require(actual == definition["headers"][header], "variant header digest mismatch " + str(path))
            log.emit("variant_source", variant=name, header=header, sha256=actual)
    return names


def preflight(args, temp, log, command):
    require(sys.flags.optimize == 0, "run Python without -O; oracle assertions are mandatory")
    require(sys.byteorder == "little", "the trace format requires the actual little-endian target")
    expected_system = "Linux" if args.platform == "linux" else "Darwin"
    require(platform.system() == expected_system, "wrong host OS: " + platform.platform())
    machine = platform.machine().lower()
    require(machine in (("x86_64", "amd64") if args.platform == "linux" else ("arm64", "aarch64")),
            "wrong runtime architecture; x86 emulation is not Mac/ARM testing: " + machine)
    log.emit("host", platform=platform.platform(), machine=machine, python=sys.version,
             python_executable=str(Path(sys.executable).resolve()), python_sha256=sha256(Path(sys.executable).resolve()),
             platform_argument=args.platform, group=args.group, mode=args.mode,
             environment_policy="explicit flags; assertions enabled; all generated files temporary; no uploads/cache/artifacts",
             inherited_option_environment_removed=True)
    command.run(["uname", "-a"], stage="host", timeout=PROVENANCE_TIMEOUT)
    if args.platform == "macos":
        arm = command.run(["/usr/sbin/sysctl", "-n", "hw.optional.arm64"], stage="host", timeout=PROVENANCE_TIMEOUT)
        require(arm.strip() == "1", "actual ARM64-capable Mac is required")
        uname = command.run(["uname", "-m"], stage="host", timeout=PROVENANCE_TIMEOUT)
        require(uname.strip() == "arm64", "the process is not running natively on ARM64")
        command.run(["sw_vers"], stage="host", timeout=PROVENANCE_TIMEOUT)
    else:
        command.run(["lscpu"], stage="host", timeout=PROVENANCE_TIMEOUT)
    names = package_variants(args, temp, log, command)
    sources = [args.package_root / (s + ".cpp") for s in ("bench", "caller", "escape")]
    sources += [unit_source(args, s) for s in SUITES]
    sources += [args.package_root / "verify_oracle.py", args.plan, Path(__file__).resolve()]
    if args.group == "combinations":
        sources.append(args.package_root / "combinations.json")
    log.emit("package_sources", hashes={str(path): sha256(path) for path in sources}, expected_base=args.expected_base)
    cxx_name = args.cxx or ("clang++" if args.platform == "macos" or args.sanitizer else "g++-15")
    located = shutil.which(cxx_name)
    require(located is not None, "compiler unavailable: " + cxx_name)
    # Preserve the clang++/g++ invocation name: resolving clang++ to clang would
    # silently change the link-driver mode and omit the C++ standard library.
    cxx = Path(located).absolute()
    version = command.run([cxx, "--version"], stage="compiler_provenance", timeout=PROVENANCE_TIMEOUT)
    if args.platform == "macos":
        require("Apple clang version" in version, "actual AppleClang is required, not upstream Clang/GCC")
    elif args.sanitizer:
        require("clang version" in version and "Apple clang" not in version, "Linux Clang is required for sanitizers")
    else:
        full = command.run([cxx, "-dumpfullversion"], stage="compiler_provenance", timeout=PROVENANCE_TIMEOUT).strip()
        require(full == "15.2.0", "exact GCC 15.2.0 is required; observed " + full)
        require("clang" not in version.lower(), "GCC 15.2.0 may not be substituted with Clang")
    triple = command.run([cxx, "-dumpmachine"], stage="compiler_provenance", timeout=PROVENANCE_TIMEOUT).strip()
    log.emit("compiler", path=str(cxx), resolved_path=str(cxx.resolve()), sha256=sha256(cxx),
             version=version, target_triple=triple, flags=compile_flags(args))
    if args.platform == "linux" and not args.sanitizer:
        cc1_name = command.run([cxx, "-print-prog-name=cc1plus"], stage="compiler_provenance", timeout=PROVENANCE_TIMEOUT).strip()
        cc1 = Path(cc1_name)
        if not cc1.is_absolute():
            resolved = shutil.which(cc1_name)
            require(resolved is not None, "cannot resolve GCC cc1plus")
            cc1 = Path(resolved)
        cc1 = cc1.resolve()
        log.emit("compiler_component", path=str(cc1), sha256=sha256(cc1))
    macros_text = command.run([cxx, *compile_flags(args), "-dM", "-E", "-x", "c++", "-"],
                              stage="compiler_macros", timeout=PROVENANCE_TIMEOUT, input_text="")
    macros = {line.split()[1] for line in macros_text.splitlines() if line.startswith("#define ")}
    require("NDEBUG" not in macros, "NDEBUG would disable required correctness assertions")
    if args.platform == "macos":
        require("__x86_64__" not in macros and ({"__aarch64__", "__arm64__"} & macros), "AppleClang did not select ARM64")
    elif args.sanitizer or args.mode == "native":
        require({"__x86_64__", "__AVX2__", "__BMI2__"} <= macros,
                "native host must exercise both AVX2 and BMI2 candidates")
    else:
        require("__x86_64__" in macros and not ({"__AVX__", "__AVX2__", "__AVX512F__", "__BMI__", "__BMI2__", "__POPCNT__"} & macros),
                "explicit x86-64 scalar fallback macros were not enforced")
    return cxx, names


def trace_cases(profile):
    # Fixed small corpus, not a product of types x layouts x distributions x APIs.
    # The supplied C++ suites already instantiate the supported layout extremes.
    cases = []

    def add(kind, dist="full", loc="random", n=257, q=384):
        if kind == "select-churn":
            q = 400
        cases.append(dict(kind=kind, dist=dist, loc=loc, n=n, q=q))

    if profile == "bench-u20":
        for kind in ("mixed", "updates", "access", "select", "insert", "erase", "neighbors", "prev", "next",
                     "count", "rank", "freq", "range", "range-narrow", "kth", "largest", "queries", "short", "pushpop"):
            add(kind)
        for kind in ("select-lastbit-miss", "select-high-k", "select-churn"):
            add(kind)
        add("select-valid-dup", "duplicates")
        add("build")
        add("rebuild")
        add("mixed", "duplicates", "boundary")
        add("mixed", "equal", "front")
        add("mixed", "sparse", "back")
        add("mixed", "sorted", "local")
        add("mixed", n=0)
        add("build", n=0, q=0)
    elif profile in ("bench-u64", "bench-i64"):
        for kind in ("mixed", "select-lastbit-miss", "select-high-k", "select-churn", "range-narrow", "build", "rebuild"):
            add(kind)
        add("select-valid-dup", "duplicates")
        add("mixed", "narrow20", "boundary")
    else:
        kind = "access" if profile == "caller-access" else "simplemixed"
        add(kind)
        add(kind, "duplicates", "boundary")
        add(kind, "sorted", "local")
    return [dict(case, seed=seed) for seed in CORRECTNESS_SEEDS for case in cases]


PROFILES = {
    "bench-u20": ("bench.cpp", ["-DBITS=20", "-DVALUE_TYPE=unsigned"]),
    "bench-u64": ("bench.cpp", ["-DBITS=64", "-DVALUE_TYPE=std::uint64_t"]),
    "bench-i64": ("bench.cpp", ["-DBITS=64", "-DVALUE_TYPE=std::int64_t"]),
    "caller-access": ("caller.cpp", ["-DBITS=20", "-DVALUE_TYPE=unsigned", "-DCALLER_SHAPE=1"]),
    "caller-simplemixed": ("caller.cpp", ["-DBITS=20", "-DVALUE_TYPE=unsigned", "-DCALLER_SHAPE=2"]),
}


def trace_contract(path, case, profile):
    lines = Path(str(path) + ".input").read_text().splitlines()
    n, q = map(int, lines[0].split())
    initial = list(map(int, lines[1].split()))
    require((n, q) == (case["n"], case["q"]) and len(initial) == n and len(lines) == q + 2,
            "trace generator size contract failed")
    types, results = collections.Counter(), collections.Counter()
    a = initial[:]
    for line in lines[2:]:
        op, p, l, r, k, x, y = map(int, line.split())
        require(0 <= op <= 15, "unknown trace opcode")
        types[op] += 1
        if profile == "caller-access":
            require(op == 6, "pure-access caller generated another opcode")
        if profile == "caller-simplemixed":
            require(op in (6, 2, 3, 4), "simplemixed caller generated another opcode")
        if case["kind"] in ("build", "rebuild"):
            continue
        require(0 <= l <= r <= len(a), "invalid generated interval")
        if op in (0, 14):
            require(op == 14 or 0 <= p <= len(a), "invalid generated insert position")
        if op in (1, 2, 6, 12):
            require(0 <= p < len(a), "invalid generated element position")
        if op in (3, 11):
            require(0 <= k < r - l, "invalid generated order statistic")
        if op == 7:
            count = a.count(x)
            results["valid" if 0 <= k < count else "missing_or_high_k"] += 1
            if case["kind"] == "select-lastbit-miss":
                require(count == 0 and (x & 1) == 1 and (x ^ 1) in a and k == 0,
                        "last-bit-miss corpus does not reach the missing final branch")
            if case["kind"] == "select-high-k":
                require(count > 0 and k == n and k >= count, "high-k select lost its present-value/high-index contract")
            if case["kind"] == "select-valid-dup":
                require(count >= 2 and 0 <= k < count, "duplicate-valid select lost duplicate/valid-index coverage")
        if op == 0:
            a.insert(p, x)
        elif op == 1:
            a.pop(p)
        elif op == 2:
            a[p] = x
        elif op == 14:
            a.append(x)
        elif op == 15:
            require(bool(a), "invalid generated pop on empty container")
            a.pop()
    if case["kind"] in ("select-lastbit-miss", "select-high-k", "select-valid-dup"):
        require(types == {7: q}, "select-specialized generator emitted non-select operations")
    if case["kind"] == "select-churn":
        require(set(types) == {2, 7} and results["valid"] and results["missing_or_high_k"],
                "mutation-select corpus missed successful or removed-duplicate select")
        require(q % 10 == 0 and types == {2: q // 10 * 4, 7: q // 10 * 6} and a == initial,
                "10-step churn did not restore initial contents with four sets and six selects per cycle")
        require(results == {"valid": q // 10 * 3, "missing_or_high_k": q // 10 * 3},
                "10-step churn missing absent/singleton/later-duplicate/removed-instance coverage")
    if case["dist"] == "narrow20" and profile in ("bench-u64", "bench-i64"):
        sign = (1 << 63) if profile == "bench-i64" else 0
        require(all((((x & ((1 << 64) - 1)) ^ sign) >> 20) == 0 for x in initial),
                "Bits=64 narrow20 did not keep encoded initial keys inside twenty bits")
    return dict(opcode_counts={str(op): count for op, count in types.items()}, select_results=dict(results), final_size=len(a))


def build(args, temp, log, command, cxx, variant, label, source, extra=(), link_escape=False):
    root = args.variants_root / variant
    binary = temp / (variant + "--" + label)
    deps = source_closure(source, (root, args.baseline_root))
    log.emit("compile_sources", variant=variant, executable=label, dependencies=deps)
    depfile = temp / (binary.name + ".d")
    argv = [cxx, *compile_flags(args), *extra, "-I", root, "-I", args.baseline_root,
            "-MMD", "-MF", depfile, "-MT", binary.name, source]
    if link_escape:
        argv.append(args.escape_object)
    argv += ["-o", binary]
    command.run(argv, stage="compile_link", timeout=COMPILE_TIMEOUT)
    compiler_dependencies(depfile, source, binary.name, temp, log, variant=variant, root=root, predicted=deps)
    log.emit("binary", variant=variant, executable=label, path=str(binary), sha256=sha256(binary), bytes=binary.stat().st_size)
    return binary


def build_escape(args, temp, log, command, cxx):
    source = args.package_root / "escape.cpp"
    args.escape_object = temp / "escape.o"
    depfile = temp / "escape.d"
    command.run([cxx, *compile_flags(args), "-MMD", "-MF", depfile, "-MT", "escape.o",
                 "-c", source, "-o", args.escape_object], stage="compile_support", timeout=COMPILE_TIMEOUT)
    compiler_dependencies(depfile, source, "escape.o", temp, log, variant=None, root=None, predicted={str(source): sha256(source)})
    log.emit("binary", variant=None, executable="escape.o", kind="shared_non_lto_support_object",
             path=str(args.escape_object), sha256=sha256(args.escape_object), bytes=args.escape_object.stat().st_size)


def execute(args):
    log = Log()
    command = None
    try:
        require(not args.work.exists() or (args.work.is_dir() and not any(args.work.iterdir())),
                "work directory must be new or empty; no stale qualification output is reused")
        args.work.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="dwm-correctness-", dir=args.work) as directory:
            temp = Path(directory)
            command = Commands(temp, log)
            cxx, names = preflight(args, temp, log, command)
            sanitizer = args.sanitizer
            per_variant_traces = 0 if sanitizer else sum(len(trace_cases(p)) for p in PROFILES)
            expected = dict(compile_link=len(names) * (len(SUITES) + (0 if sanitizer else len(PROFILES))),
                            compile_support=0 if sanitizer else 1,
                            unit_run=len(names) * len(SUITES), replay_run=len(names) * per_variant_traces,
                            independent_oracle=len(names) * per_variant_traces)
            log.emit("gate_plan", variants=names, suites=SUITES, profiles=[] if sanitizer else list(PROFILES),
                     expected_execution_counts=expected, replay_count=0 if sanitizer else 3,
                     per_command_timeouts_seconds=dict(compile=COMPILE_TIMEOUT, unit=SANITIZER_SUITE_TIMEOUT if sanitizer else SUITE_TIMEOUT,
                                                      replay=REPLAY_TIMEOUT, oracle=ORACLE_TIMEOUT, provenance=PROVENANCE_TIMEOUT),
                     stream_limit_bytes=STREAM_LIMIT, correctness_log_limit_bytes=TOTAL_LOG_LIMIT,
                     separate_timing_log_limit_bytes=TIMING_LOG_LIMIT_REFERENCE,
                     overall_deadline_seconds=JOB_DEADLINE_SECONDS, correctness_seeds=CORRECTNESS_SEEDS,
                     performance_authorization=False, external_reference_sources=False,
                     timing_prerequisite="all first-wave Linux native, Linux scalar, Mac ARM64, sanitizer-a and sanitizer-b groups pass on identical source package",
                     later_combinations_timing_requires="separate full combination Linux native/scalar and Mac gates",
                     no_test_filtering=True, no_source_ignores=True, no_persistent_output=True)
            if not sanitizer:
                build_escape(args, temp, log, command, cxx)
            observed_traces = {}
            completed = []
            sanitized_env = dict(ASAN_OPTIONS="detect_leaks=1:halt_on_error=1:abort_on_error=1",
                                 UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1") if sanitizer else {}
            for variant in names:
                for suite in SUITES:
                    binary = build(args, temp, log, command, cxx, variant, suite,
                                   unit_source(args, suite))
                    command.run([binary], stage="unit_run", timeout=SANITIZER_SUITE_TIMEOUT if sanitizer else SUITE_TIMEOUT,
                                env=sanitized_env)
                    completed.append((variant, suite))
                    log.emit("unit_pass", variant=variant, suite=suite, sanitizer=sanitizer)
                    binary.unlink()
                if sanitizer:
                    log.emit("variant_pass", variant=variant, mode=args.mode, platform=args.platform, sanitizer=True)
                    continue
                for profile, (filename, extra) in PROFILES.items():
                    binary = build(args, temp, log, command, cxx, variant, profile,
                                   args.package_root / filename, extra=extra, link_escape=True)
                    for index, case in enumerate(trace_cases(profile)):
                        dump = temp / (variant + "--" + profile + "--" + str(index) + ".bin")
                        argv = [binary, case["kind"], case["n"], case["q"], case["seed"],
                                case["dist"], case["loc"], 3, "warm", dump]
                        output = command.run(argv, stage="replay_run", timeout=REPLAY_TIMEOUT,
                                             env={"DWM_VALIDATE_REPLAYS": "1"})
                        record = json.loads(output)
                        require(record["reps"] == 3, "exact-three replay contract failed")
                        # The C++ executable checks exact output and final-state vectors
                        # on every replay. This independent vector oracle then checks the
                        # complete first replay, not only a checksum or another candidate.
                        details = trace_contract(dump, case, profile)
                        command.run([sys.executable, "-I", "-B", args.package_root / "verify_oracle.py", dump, case["kind"]],
                                    stage="independent_oracle", timeout=ORACLE_TIMEOUT)
                        input_hash, output_hash = sha256(Path(str(dump) + ".input")), sha256(dump)
                        exact_pair = (Path(str(dump) + ".input").read_bytes(), dump.read_bytes())
                        key = (profile, index)
                        if key in observed_traces:
                            require(observed_traces[key] == exact_pair,
                                    "candidate differs from baseline exact input/output " + str(key))
                        else:
                            require(variant == "baseline", "baseline must establish the exact trace comparison")
                            observed_traces[key] = exact_pair
                        log.emit("oracle_pass", variant=variant, profile=profile, case=case,
                                 replay_count=3, validation="independent full vector oracle plus exact output/final-state equality on all three replays",
                                 input_sha256=input_hash, full_output_sha256=output_hash, **details)
                        dump.unlink()
                        Path(str(dump) + ".input").unlink()
                    binary.unlink()
                log.emit("variant_pass", variant=variant, mode=args.mode, platform=args.platform)
            for stage, count in expected.items():
                require(command.by_stage[stage] == count, "execution count mismatch for " + stage)
            log.emit("gate_result", success=True, platform=args.platform, group=args.group, mode=args.mode,
                     variants=names, completed_units=len(completed), executed_counts=dict(command.by_stage),
                     total_commands=command.count, timings_qualified=False,
                     statement="This exact correctness group passed; timing still requires every workflow prerequisite.")
        return 0
    except Exception as exc:
        log.emit("fatal", error=repr(exc), traceback=traceback.format_exc(),
                 command_count=0 if command is None else command.count)
        log.emit("gate_result", success=False, platform=args.platform, group=args.group, mode=args.mode,
                 timings_qualified=False, statement="Required checks failed or did not run; no timing authorization.")
        return 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("repo", type=Path)
    parser.add_argument("work", type=Path)
    parser.add_argument("group", choices=("first-wave", "combinations", "sanitizer-a", "sanitizer-b"))
    parser.add_argument("mode", choices=("native", "scalar", "mac", "sanitizer"))
    parser.add_argument("--platform", choices=("linux", "macos"), help="Optional explicit OS, checked against selected mode and runtime")
    parser.add_argument("--package-root", type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument("--plan", type=Path, help="Defaults to package-root/plan.json")
    parser.add_argument("--cxx", help="Explicit compiler executable; its identity/version is still enforced")
    args = parser.parse_args()
    args.sanitizer = args.group in ("sanitizer-a", "sanitizer-b")
    if args.sanitizer != (args.mode == "sanitizer"):
        parser.error("sanitizer-a/sanitizer-b group and sanitizer mode must be selected together")
    selected_platform = "macos" if args.mode == "mac" else "linux"
    if args.platform and args.platform != selected_platform:
        parser.error("platform and mode disagree")
    args.platform = selected_platform
    args.group = {"first-wave": "individuals"}.get(args.group, args.group)
    args.mode = {"mac": "arm64", "sanitizer": "native"}.get(args.mode, args.mode)
    if args.plan is None:
        args.plan = args.package_root / "plan.json"
    for name in ("repo", "work", "package_root", "plan"):
        setattr(args, name, getattr(args, name).resolve())
    return execute(args)


if __name__ == "__main__":
    sys.exit(main())
