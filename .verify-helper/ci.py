import argparse
import hashlib
import importlib.metadata
import json
import logging
import os
from pathlib import Path
import platform
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parent.parent
VERSION = 1


def digest(value):
    return hashlib.sha256(value).hexdigest()


def read_json(path):
    try:
        return json.loads(path.read_text())
    except (OSError, ValueError):
        return None


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")


def environment_key():
    return json.dumps({
        "python": sys.version,
        "platform": [platform.system(), platform.machine(), os.environ.get("ImageVersion")],
        "compilers": {cxx: subprocess.check_output([cxx, "--version"], text=True)
                      for cxx in ("g++", "clang++")},
        "packages": sorted((dist.metadata["Name"], dist.version)
                           for dist in importlib.metadata.distributions()),
        "environment": {key: os.environ.get(key) for key in
                        ("CXX", "CXXFLAGS", "CPATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH", "LD_LIBRARY_PATH")},
    }, sort_keys=True)


def build_plan(root, previous=None, *, environment, force=False):
    root = root.resolve()
    targets = {
        "headers": sorted(path.relative_to(root).as_posix()
                          for folder in ("convolution", "data_structure", "graph", "math")
                          for path in (root / folder).rglob("*.hpp")),
        "unit": sorted(path.relative_to(root).as_posix() for path in (root / "test/unit").rglob("*.cpp")),
        "verification": sorted(path.relative_to(root).as_posix() for path in (root / "test").rglob("*.test.cpp")),
    }
    contents = {}
    dependencies = {}
    uncertain = False

    def content(name):
        if name not in contents:
            path = root / name
            contents[name] = path.read_bytes() if path.is_file() else None
        return contents[name]

    def includes(name):
        nonlocal uncertain
        if name in dependencies:
            return dependencies[name]
        dependencies[name] = result = set()
        data = content(name)
        if data is None:
            return result
        if (root / name).is_symlink():
            uncertain = True
        source = data.decode("utf-8").replace("\\\n", "")
        if re.search(r"\b__has_include\b|^\s*#\s*(?:include_next|embed)\b", source, re.MULTILINE):
            uncertain = True
        for include in re.findall(r"^\s*#\s*include\b([^\n]*)", source, re.MULTILINE):
            match = re.match(r'\s*(?:"([^"]+)"|<([^>]+)>)', include)
            if match is None:
                uncertain = True
                continue
            quoted, angled = match.groups()
            candidates = [root / angled] if angled else [root / Path(name).parent / quoted, root / quoted]
            for path in candidates:
                if path.is_symlink():
                    uncertain = True
                try:
                    dependency = path.resolve().relative_to(root).as_posix()
                except ValueError:
                    uncertain = True
                    continue
                result.add(dependency)
                if content(dependency) is not None:
                    break
        return result

    common = {root / name for name in (".gitignore", ".verify-helper/config.toml", ".verify-helper/check-docs.py", "test/requirements.txt")}
    common.update((root / "test").glob("*.py"))
    common.update((root / ".verify-helper").glob("*.py"))
    common.update((root / ".github/workflows").glob("*.yml"))
    common.update((root / ".github/workflows").glob("*.yaml"))
    settings = [(path.relative_to(root).as_posix(), digest(path.read_bytes()) if path.is_file() else None)
                for path in sorted(common)]
    environment = digest(json.dumps([environment, settings]).encode())
    fingerprints = {}
    for group, paths in targets.items():
        fingerprints[group] = {}
        for name in paths:
            visited, pending = set(), [name]
            while pending:
                path = pending.pop()
                if path not in visited:
                    visited.add(path)
                    pending.extend(includes(path) - visited)
            inputs = [(path, digest(content(path)) if content(path) is not None else None)
                      for path in sorted(visited)]
            fingerprints[group][name] = digest(json.dumps(inputs).encode())
    valid = (isinstance(previous, dict) and previous.get("version") == VERSION
             and previous.get("environment") == environment and isinstance(previous.get("targets"), dict)
             and all(isinstance(previous["targets"].get(group), dict) for group in targets))
    full = force or uncertain or not valid
    reason = ("Full verification requested" if force else "Non-literal or external includes require full verification" if uncertain
              else "No matching successful verification cache" if not valid else "Reusing successful verification results")
    plan = {group: [name for name in paths
                    if full or previous["targets"][group].get(name) != fingerprints[group][name]]
            for group, paths in targets.items()}
    return {**plan, "full": full, "reason": reason,
            "state": {"version": VERSION, "environment": environment, "targets": fingerprints}}


def verify_selected(plan, marker, run):
    paths = set(plan["state"]["targets"]["verification"])
    selected = set(plan["verification"])
    for name in sorted(selected):
        marker.mark_failed(Path(name))
    for name in sorted(paths - selected):
        marker.mark_verified(Path(name))
    if selected:
        summary = run([Path(name) for name in sorted(selected)])
        summary.show()
        if not summary.succeeded():
            raise RuntimeError("Library Checker verification failed")
        unfinished = [name for name in sorted(selected) if not marker.is_verified(Path(name))]
        if unfinished:
            raise RuntimeError("Verification did not finish: " + ", ".join(unfinished))
    print(f"Library Checker: verified {len(selected)}, reused {len(paths - selected)}", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("plan", "docs", "save"))
    parser.add_argument("--state", type=Path, default=Path(".verify-helper/verified-state.json"))
    parser.add_argument("--selection", type=Path, default=Path(".verify-helper/selection.json"))
    parser.add_argument("--full", action="store_true")
    args = parser.parse_args()
    os.chdir(ROOT)
    if args.command == "plan":
        plan = build_plan(ROOT, read_json(args.state), environment=environment_key(),
                          force=args.full or os.environ.get("VERIFY_FULL") == "true")
        write_json(args.selection, plan)
        print(plan["reason"], flush=True)
        rows = ["| Check | Selected | Total |", "| --- | ---: | ---: |"]
        for group in ("headers", "unit", "verification"):
            total = len(plan["state"]["targets"][group])
            print(f"{group}: {len(plan[group])}/{total}", flush=True)
            if not plan["full"]:
                for name in plan[group]:
                    print(f"  {name}", flush=True)
            rows.append(f"| {group} | {len(plan[group])} | {total} |")
        if os.environ.get("GITHUB_STEP_SUMMARY"):
            with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as stream:
                stream.write(plan["reason"] + "\n\n" + "\n".join(rows) + "\n")
    else:
        plan = json.loads(args.selection.read_text())
        if args.command == "save":
            write_json(args.state, plan["state"])
        else:
            import onlinejudge.service
            from onlinejudge_verify import marker, verify
            from onlinejudge_verify.documentation import main as documentation

            logging.basicConfig(level=logging.INFO)
            verification = marker.get_verification_marker(jobs=2)
            with verification:
                verify_selected(plan, verification, lambda paths: verify.main(
                    paths, marker=verification, timeout=3000, tle=10, jobs=2))
            documentation.main(jobs=2)


if __name__ == "__main__":
    main()
