#!/usr/bin/env python3
"""Reproduce the frozen tree-only comparison without modifying the checkout."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import posixpath
import re
import shutil
import subprocess
import sys

BASELINE = "31d238c62d0b297b1ffaf8b32a9b08461c2cadd9"
PRODUCTION = "fps/multipoint_eval.hpp"
ORIGINAL_SHA = "ac33415ec7aaead7adba8e1114f1ffc303a666c10549158be1ff1612fc839a94"
CANDIDATE_SHA = "4cb93dd9d9d2b0bbbece729c7de93217a19cd4915cf9ace9aa797c0eefcb049e"
RENAMED_SHA = "6f99a23ad5154ce6c91365cac199d58d2bc18d8057207e8efe0cbf7817e8e416"
SOURCE_HASHES = {'benchmark.cpp': 'a84c5588a54f1a5fc871258fd6be26296b5db78f2d25a01872b95e9a8e3c3fc2', 'correctness.cpp': 'e190343b6b498c6da193a11265b1b2ec0f4a82237d7d1c541d880fbef41ed09f', 'summarize.py': '1d658cda0c86b62a0c4890ffde416ef37818070df60a3fed6a373051b3921aa9', 'structural.cpp': '6706a092443061993aed4eb77f2e25add91eb1a987545d06dc47724ab45cd072', 'source-manifest.json': 'f255757e4cd91cf8173ed929f0764f992e6fae86d1e3934341d4541251a43f74', 'protocol.md': '7bddb1a273ec4fb981c3ce08de052032a91b785ebe2f3f25facca3e35fd33ae6', 'memory.cpp': 'd98d6f3e915dc6994bfbc56610bf65d578952e0405e48984ee471201f48b470b', 'shapes.inc': '186177732627bf375ceec481d8854457aa117c2f21ad3899434b9691bcbbb3f9', 'adversarial.cpp': '2488ae6301db92f01e7a542eadb66ba9aa91536e398ab915012875bd505b389d'}
BASELINE_CLOSURE = {'fps/multipoint_eval.hpp': 'ac33415ec7aaead7adba8e1114f1ffc303a666c10549158be1ff1612fc839a94', 'fps/fps.hpp': 'b69f0eab0cdd168550d9d13e0f6b700b109826cc491e10ce1306ac806ab7a357', 'components/fps_operations.hpp': '03bec508671214aaaa61b3cf4fa4449a09659c38b1d04240db2ad6a91145c760', 'components/ntt.hpp': '645cd0a0b3cd322b12189915e402f2d5d0d9c35a1a4ff7285e75a23e1553a776', 'math/mint.hpp': '67cec2452dda0af8b4cd307449fc65b3f1d678613e0392fdd666031dae062171', 'components/fps_product_tree.hpp': 'd72a0a371bcf404028ca1fd970355b01efc097dbc7431a59c26ad3be347d9d58', 'components/fps_direct_eval.hpp': '73f3c335566cad50271ed5a0c8336ec10c57c6ec2c69a193c463939d15136380'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require_hash(data, expected, label):
    if digest(data) != expected:
        raise RuntimeError(f"Frozen hash mismatch: {label}")


def dependencies(path, read, found=None):
    if found is None:
        found = {}
    if path in found:
        return found
    if path.startswith("../") or path.startswith("/"):
        raise RuntimeError(f"Include escapes repository: {path}")
    data = read(path)
    found[path] = data
    for include in re.findall(rb'^\s*#\s*include\s*"([^"\n]+)"', data, re.MULTILINE):
        child = posixpath.normpath(posixpath.join(posixpath.dirname(path), include.decode()))
        dependencies(child, read, found)
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--cxx", default="g++")
    parser.add_argument("--cpu", type=int, default=0)
    parser.add_argument("--git-repository", type=Path, help="Object source, normally this checkout")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--prepare-only", action="store_true", help="Hash/dependency checks only; no compiler or timing")
    modes.add_argument("--checks-only", action="store_true", help="Run correctness checks, but no timing")
    modes.add_argument("--memory-only", action="store_true", help="Separate allocation/structural accounting only")
    args = parser.parse_args()
    source = Path(__file__).resolve().parent
    root = source.parents[2]
    git = (args.git_repository or root).resolve()
    out = args.output.resolve()
    if out == root or root in out.parents:
        raise RuntimeError("Raw outputs must be outside the repository")
    out.mkdir(parents=True, exist_ok=True)
    if any(out.glob("*.csv")):
        raise RuntimeError("Refusing to overwrite prior observations; use a new output directory")
    for name, expected in SOURCE_HASHES.items():
        require_hash((source / name).read_bytes(), expected, name)

    def old_read(path):
        return subprocess.check_output(["git", "-C", str(git), "show", f"{BASELINE}:{path}"])

    old = dependencies(PRODUCTION, old_read)
    current = dependencies(PRODUCTION, lambda path: (root / path).read_bytes())
    if old.keys() != current.keys():
        raise RuntimeError("Recursive real include dependency set changed")
    if set(old) != set(BASELINE_CLOSURE):
        raise RuntimeError("Pinned seven-header closure mismatch")
    for path in old:
        require_hash(old[path], BASELINE_CLOSURE[path], path)
        if path != PRODUCTION and old[path] != current[path]:
            raise RuntimeError(f"Baseline dependency changed: {path}")
    require_hash(old[PRODUCTION], ORIGINAL_SHA, "original production")
    require_hash(current[PRODUCTION], CANDIDATE_SHA, "candidate production")
    renamed = old[PRODUCTION].decode().replace("CP_FPS_MULTIPOINT_EVAL_HPP", "CP_BASELINE_MULTIPOINT_EVAL_HPP")
    renamed = renamed.replace("multipoint_eval", "baseline_eval")
    renamed = re.sub(r'(#include ")([^"\n]+)(")', lambda match: match[1] + posixpath.normpath(posixpath.join("fps", match[2])) + match[3], renamed)
    require_hash(renamed.encode(), RENAMED_SHA, "renamed baseline")
    (out / "baseline.hpp").write_text(renamed)
    metadata = {
        "baseline": BASELINE,
        "candidate_sha256": CANDIDATE_SHA,
        "dependency_sha256": {path: digest(data) for path, data in current.items()},
        "source_sha256": {path.name: digest(path.read_bytes()) for path in source.iterdir() if path.is_file()},
        "cpu": args.cpu,
        "flags": ["-std=c++20", "-O2"],
        "native_extra": ["-march=native"],
    }
    (out / "snapshot.json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(f"Verified exact baseline and {len(old) - 1} unchanged recursive dependencies.", flush=True)
    if args.prepare_only:
        return
    if not sys.platform.startswith("linux"):
        raise RuntimeError("Frozen affinity/timing reproduction requires Linux; use normal unit tests for Mac portability")
    if hasattr(os, "sched_getaffinity") and args.cpu not in os.sched_getaffinity(0):
        raise RuntimeError(f"CPU {args.cpu} is not available; choose and record an allowed CPU explicitly")
    metadata["compiler"] = subprocess.check_output([args.cxx, "--version"], text=True)
    metadata["cpuinfo"] = Path("/proc/cpuinfo").read_text()
    (out / "snapshot.json").write_text(json.dumps(metadata, indent=2) + "\n")

    def compile_one(name, extra=()):
        binary = out / (name + ("-native" if extra else "") + ".bin")
        subprocess.run([args.cxx, "-std=c++20", "-O2", *extra, "-I", str(root), "-iquote", str(out), str(source / (name + ".cpp")), "-o", str(binary)], check=True)
        return binary

    def run(binary, output, arguments=()):
        with output.open("w") as stream:
            subprocess.run(["taskset", "-c", str(args.cpu), str(binary), *map(str, arguments)], stdout=stream, check=True)

    if not args.memory_only:
        scalar = out / "forced-scalar"
        for path, data in current.items():
            target = scalar / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        ntt = scalar / "components/ntt.hpp"
        text = ntt.read_text()
        if text.count("static bool vectorized() {") != 1:
            raise RuntimeError("Cannot make the declared forced-scalar test clone")
        ntt.write_text(text.replace("static bool vectorized() {", "static bool vectorized() {\n\t\treturn false;"))
        with (out / "correctness.log").open("w") as stream:
            def check(name, path, include=root, sanitize=False):
                binary = out / (name + ".bin")
                flags = ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"] if sanitize else ["-O2"]
                command = [args.cxx, "-std=c++20", *flags, "-I", str(include), "-I", str(root), "-iquote", str(out), str(path), "-o", str(binary)]
                print("Compile:", " ".join(command), file=stream, flush=True)
                subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT, check=True)
                environment = os.environ.copy()
                if sanitize:
                    environment["ASAN_OPTIONS"] = "detect_leaks=0"
                subprocess.run(["taskset", "-c", str(args.cpu), str(binary)], stdout=stream, stderr=subprocess.STDOUT, env=environment, check=True)
                print(name, "PASS", file=stream, flush=True)
            for name in ("correctness", "adversarial"):
                check(name + "-default", source / (name + ".cpp"))
                check(name + "-scalar", source / (name + ".cpp"), include=scalar)
                check(name + "-sanitize", source / (name + ".cpp"), sanitize=True)
            for name in ("fps", "fps_algorithms", "standalone", "multipoint_tree"):
                check("unit-" + name, root / "test/unit" / (name + ".cpp"))
        if args.checks_only:
            return

    if args.memory_only:
        binary = compile_one("memory")
        header = "n,m,variant,start_live_requested,peak_live_requested,end_live_requested,total_requested,allocations,vector_size\n"
        with (out / "memory.csv").open("w") as stream:
            stream.write(header)
            stream.flush()
            for n, m in ((1024, 33), (32768, 33), (131072, 129), (131072, 131072)):
                for variant in (0, 1):
                    subprocess.run(["taskset", "-c", str(args.cpu), str(binary), str(n), str(m), str(variant)], stdout=stream, check=True)
        run(compile_one("structural"), out / "structural.csv")
    else:
        for build, extra in (("portable", ()), ("native", ("-march=native",))):
            run(compile_one("benchmark", extra), out / (build + ".csv"))
            with (out / (build + "-summary.txt")).open("w") as stream:
                subprocess.run([sys.executable, str(source / "summarize.py"), str(out / (build + ".csv"))], stdout=stream, check=True)


if __name__ == "__main__":
    main()
