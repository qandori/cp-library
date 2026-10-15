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
VERSION = 2
GROUPS = ("headers", "unit", "sanitize", "verification", "bundles")


def digest(value):
    return hashlib.sha256(value).hexdigest()


def read_json(path):
    try:
        return json.loads(path.read_text())
    except (OSError, ValueError):
        return None


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    temporary.replace(path)


def cached_targets(previous):
    if isinstance(previous, dict) and previous.get("version") == VERSION:
        targets = previous.get("targets")
        if isinstance(targets, dict) and all(isinstance(targets.get(group, {}), dict) for group in GROUPS):
            return targets
    return None


def environment_key():
    toolchain = {
        "platform": [platform.system(), platform.machine(), os.environ.get("ImageVersion")],
        "compilers": {cxx: subprocess.check_output([cxx, "--version"], text=True)
                      for cxx in ("g++", "clang++")},
        "environment": {key: os.environ.get(key) for key in
                        ("CXX", "CXXFLAGS", "CPATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH", "LD_LIBRARY_PATH")},
    }
    helper = {"toolchain": toolchain, "python": sys.version,
              "packages": sorted((dist.metadata["Name"], dist.version)
                                 for dist in importlib.metadata.distributions())}
    return {group: json.dumps(helper if group in ("verification", "bundles") else toolchain, sort_keys=True)
            for group in GROUPS}


def workflow_checks(root):
    checks = {group: [] for group in GROUPS}
    files = sorted((root / ".github/workflows").glob("*.y*ml"))
    for path in files:
        source = path.read_text()
        try:
            import yaml
            workflow = yaml.safe_load(source)
            jobs = workflow.get("jobs", {})
            if not isinstance(jobs, dict):
                raise ValueError("Invalid workflow")
            for job in jobs.values():
                for step in job.get("steps", []):
                    command = step.get("run", "")
                    if "test/verify.py" in command:
                        groups = ("sanitize",) if "--sanitize" in command else ("headers", "unit")
                    elif ".verify-helper/ci.py judge" in command:
                        groups = ("verification",)
                    elif ".verify-helper/check-docs.py" in command:
                        groups = ("bundles",)
                    else:
                        continue
                    for group in groups:
                        checks[group].append([command, workflow.get("env"), job.get("env"), step.get("env"),
                                              job.get("strategy"), workflow.get("defaults"), job.get("defaults")])
        except (ImportError, AttributeError, TypeError, ValueError):
            for group in GROUPS:
                checks[group].append(source)
    return checks


def build_plan(root, previous=None, *, environment, force=False):
    root = root.resolve()
    targets = {
        "headers": sorted(path.relative_to(root).as_posix()
                          for folder in ("convolution", "data_structure", "graph", "math", "string")
                          for path in (root / folder).rglob("*.hpp")),
        "unit": sorted(path.relative_to(root).as_posix() for path in (root / "test/unit").rglob("*.cpp")),
        "verification": sorted(path.relative_to(root).as_posix() for path in (root / "test").rglob("*.test.cpp")),
    }
    targets["sanitize"] = targets["unit"]
    targets["bundles"] = targets["headers"]
    contents = {}
    dependencies = {}
    uncertain = set()

    def content(name):
        if name not in contents:
            path = root / name
            contents[name] = path.read_bytes() if path.is_file() else None
        return contents[name]

    def includes(name):
        if name in dependencies:
            return dependencies[name]
        dependencies[name] = result = set()
        data = content(name)
        if data is None:
            return result
        if (root / name).is_symlink():
            uncertain.add(name)
        source = data.decode("utf-8").replace("\\\n", "")
        if re.search(r"\b__has_include\b|^\s*#\s*(?:include_next|embed)\b", source, re.MULTILINE):
            uncertain.add(name)
        for include in re.findall(r"^\s*#\s*include\b([^\n]*)", source, re.MULTILINE):
            match = re.match(r'\s*(?:"([^"]+)"|<([^>]+)>)', include)
            if match is None:
                uncertain.add(name)
                continue
            quoted, angled = match.groups()
            candidates = [root / angled] if angled else [root / Path(name).parent / quoted, root / quoted]
            for path in candidates:
                if path.is_symlink():
                    uncertain.add(name)
                try:
                    dependency = path.resolve().relative_to(root).as_posix()
                except ValueError:
                    uncertain.add(name)
                    continue
                result.add(dependency)
                if content(dependency) is not None:
                    break
        return result

    settings = {
        "headers": ("test/verify.py",),
        "unit": ("test/verify.py",),
        "sanitize": ("test/verify.py",),
        "verification": (".verify-helper/config.toml", "test/requirements.txt"),
        "bundles": (".verify-helper/check-docs.py", ".verify-helper/documentation.py",
                    ".verify-helper/config.toml", "test/requirements.txt"),
    }
    fingerprints = {}
    uncacheable = {group: set() for group in GROUPS}
    commands = workflow_checks(root)
    for group, paths in targets.items():
        context = environment[group] if isinstance(environment, dict) else environment
        configuration = [(name, digest(content(name)) if content(name) is not None else None)
                         for name in settings[group]]
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
            fingerprints[group][name] = digest(json.dumps([context, configuration, commands[group], inputs]).encode())
            if visited & uncertain:
                uncacheable[group].add(name)
    valid = cached_targets(previous) is not None
    full = force or not valid
    reason = ("Full verification requested" if force else "No compatible verification cache" if not valid
              else "Reusing successful checks with matching content, dependencies and tools")
    plan = {group: [name for name in paths
                    if full or name in uncacheable[group]
                    or previous["targets"].get(group, {}).get(name) != fingerprints[group][name]]
            for group, paths in targets.items()}
    return {**plan, "full": full, "reason": reason,
            "state": {"version": VERSION, "targets": fingerprints}}


def invalidate_selected(previous, plan):
    targets = cached_targets(previous) or {}
    return {"version": VERSION, "targets": {
        group: {name: fingerprint for name, fingerprint in targets.get(group, {}).items()
                if name in plan["state"]["targets"][group] and name not in plan[group]}
        for group in GROUPS}}


def record_success(previous, plan, groups):
    if not set(groups) <= set(GROUPS):
        raise ValueError("Unknown verification group")
    targets = cached_targets(previous) or {}
    return {"version": VERSION, "targets": {
        group: dict(plan["state"]["targets"][group] if group in groups else targets.get(group, {}))
        for group in GROUPS}}


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
            raise RuntimeError("Judge verification failed")
        unfinished = [name for name in sorted(selected) if not marker.is_verified(Path(name))]
        if unfinished:
            raise RuntimeError("Verification did not finish: " + ", ".join(unfinished))
    print(f"Judge tests: verified {len(selected)}, reused {len(paths - selected)}", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("plan", "judge", "docs", "record", "save"))
    parser.add_argument("--state", type=Path, default=Path(".verify-helper/verified-state.json"))
    parser.add_argument("--selection", type=Path, default=Path(".verify-helper/selection.json"))
    parser.add_argument("--full", action="store_true")
    parser.add_argument("--groups", nargs="+", choices=GROUPS, default=GROUPS)
    args = parser.parse_args()
    os.chdir(ROOT)
    if args.command == "plan":
        previous = read_json(args.state)
        plan = build_plan(ROOT, previous, environment=environment_key(),
                          force=args.full or os.environ.get("VERIFY_FULL") == "true")
        write_json(args.selection, plan)
        write_json(args.state, invalidate_selected(previous, plan))
        print(plan["reason"], flush=True)
        rows = ["| Check | Selected | Total |", "| --- | ---: | ---: |"]
        for group in GROUPS:
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
        if args.command in ("record", "save"):
            write_json(args.state, record_success(read_json(args.state), plan, args.groups))
        elif args.command == "judge":
            import onlinejudge.service
            from onlinejudge_verify import marker, verify

            logging.basicConfig(level=logging.INFO)
            verification = marker.get_verification_marker(jobs=2)
            with verification:
                verify_selected(plan, verification, lambda paths: verify.main(
                    paths, marker=verification, timeout=3000, tle=10, jobs=2))
        else:
            from documentation import generate
            generate(plan)


if __name__ == "__main__":
    main()
