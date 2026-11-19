import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(root / '.verify-helper'))
from atcoder_flags import constexpr_flags

parser = argparse.ArgumentParser()
parser.add_argument("--cxx", nargs="+", default=["g++", "clang++"])
parser.add_argument("--unit-only", action="store_true")
parser.add_argument("--sanitize", action="store_true")
parser.add_argument("--selection", type=Path)
parser.add_argument("--jobs", type=int, default=1)
args = parser.parse_args()
if args.jobs < 1:
    parser.error("--jobs must be positive")
if args.selection and not args.unit_only:
    parser.error("--selection requires --unit-only; use .verify-helper/ci.py judge for Library Checker tests")
selection = json.loads(args.selection.read_text()) if args.selection else None
headers = ([root / path for path in selection["headers"]] if selection is not None else
           sorted(path for folder in ("components", "convolution", "fps", "data_structure", "graph", "flow", "tree", "math", "sequence", "string", "geometry", "integer_geometry")
                  for path in (root / folder).rglob("*.hpp")))
source_group = "sanitize" if args.sanitize and selection is not None and "sanitize" in selection else "unit"
sources = ([root / path for path in selection[source_group]] if selection is not None else
           sorted((root / "test/unit").rglob("*.cpp")))


def check(task):
    compiler, path, standalone = task
    with tempfile.TemporaryDirectory(prefix="kyopro-verify-") as directory:
        flags = ["-std=c++2b", "-O2", "-Wall", "-Wextra", "-DDEBUG=1",
                 *constexpr_flags(compiler), "-I", str(root)]
        if standalone:
            source = Path(directory) / "header.cpp"
            name = path.relative_to(root).as_posix()
            source.write_text(f'#include "{name}"\n#include "{name}"\n'
                              '#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) '
                              '|| defined(all) || defined(sz) || defined(uint)\n'
                              '#error "Library headers must not load template macros"\n#endif\n'
                              'int main() {}\n')
            print(f"{compiler}: standalone {name}", flush=True)
            subprocess.run([compiler, *flags, "-Werror", "-fsyntax-only", str(source)], check=True, cwd=root)
        else:
            executable = Path(directory) / path.stem
            test_flags = flags.copy()
            if args.sanitize:
                test_flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g"]
            print(f"{compiler}: {path.name}", flush=True)
            subprocess.run([compiler, *test_flags, str(path), "-o", str(executable)], check=True, cwd=root)
            subprocess.run([str(executable)], check=True, timeout=120, cwd=root)


tasks = [(compiler, path, True) for compiler in args.cxx for path in ([] if args.sanitize else headers)]
tasks += [(compiler, path, False) for compiler in args.cxx for path in sources]
with ThreadPoolExecutor(max_workers=args.jobs) as pool:
    futures = [pool.submit(check, task) for task in tasks]
    try:
        for future in as_completed(futures):
            future.result()
    except BaseException:
        for future in futures:
            future.cancel()
        raise
if not args.unit_only:
    command = shutil.which("oj-verify")
    if command is None:
        parser.error("oj-verify is missing; install test/requirements.txt first")
    environment = os.environ.copy()
    environment["PATH"] = str(Path(command).resolve().parent) + os.pathsep + environment["PATH"]
    subprocess.run([command, "run", "--timeout", "1200", "--tle", "10", "-j", "2"],
                   check=True, cwd=root, env=environment)
print("Verification passed", flush=True)
