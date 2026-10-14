import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser()
parser.add_argument("--cxx", nargs="+", default=["g++", "clang++"])
parser.add_argument("--unit-only", action="store_true")
parser.add_argument("--sanitize", action="store_true")
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix="kyopro-verify-") as directory:
    for compiler in args.cxx:
        for source in sorted((root / "test/unit").glob("*.cpp")):
            executable = Path(directory) / source.stem
            flags = ["-std=c++23", "-O2", "-Wall", "-Wextra", "-DDEBUG=1", "-I", str(root)]
            if args.sanitize:
                flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g"]
            print(f"{compiler}: {source.name}", flush=True)
            subprocess.run([compiler, *flags, str(source), "-o", str(executable)], check=True, cwd=root)
            subprocess.run([str(executable)], check=True, timeout=120, cwd=root)
if not args.unit_only:
    command = shutil.which("oj-verify")
    if command is None:
        parser.error("oj-verify is missing; install test/requirements.txt first")
    environment = os.environ.copy()
    environment["PATH"] = str(Path(command).resolve().parent) + os.pathsep + environment["PATH"]
    subprocess.run([command, "run", "--timeout", "1200", "--tle", "10", "-j", "2"],
                   check=True, cwd=root, env=environment)
print("Verification passed", flush=True)
