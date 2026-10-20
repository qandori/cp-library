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


def boost_identity():
    if platform.system() != "Linux":
        return None
    try:
        def package(name):
            row = subprocess.check_output(
                ["dpkg-query", "-W", "-f=${db:Status-Status}\t${Version}\t${Depends}", name],
                text=True, stderr=subprocess.DEVNULL, timeout=10).split("\t")
            if len(row) != 3 or row[0] != "installed" or not row[1]:
                raise ValueError("Missing Boost package")
            return row
        meta = package("libboost-dev")
        headers = re.findall(r"(?:^|,)\s*(libboost[0-9.]+-dev)\s*(?:\([^)]*\))?\s*(?=,|$)", meta[2])
        if len(headers) != 1:
            return None
        resolved = package(headers[0])
        return [("libboost-dev", meta[1]), (headers[0], resolved[1])]
    except (OSError, subprocess.SubprocessError, ValueError):
        return None


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
                          for folder in ("components", "convolution", "fps", "data_structure", "graph", "tree", "math", "sequence", "string")
                          for path in (root / folder).rglob("*.hpp")),
        "unit": sorted(path.relative_to(root).as_posix() for path in (root / "test/unit").rglob("*.cpp")),
        "verification": sorted(path.relative_to(root).as_posix() for path in (root / "test").rglob("*.test.cpp")),
    }
    targets["sanitize"] = targets["unit"]
    targets["bundles"] = targets["headers"]
    contents = {}
    dependencies = {}
    uncertain = set()
    boost_consumers = set()
    boost = []

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
            else:
                if (angled or quoted).startswith("boost/"):
                    boost_consumers.add(name)
        return result

    settings = {
        "headers": ("test/verify.py",),
        "unit": ("test/verify.py",),
        "sanitize": ("test/verify.py",),
        "verification": (".verify-helper/config.toml", "test/requirements.txt",
                         ".verify-helper/ci.py", ".verify-helper/verification.py", ".verify-helper/judge_data.py"),
        "bundles": (".verify-helper/check-docs.py", ".verify-helper/documentation.py", ".verify-helper/verification.py",
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
            fingerprint = [context, configuration, commands[group], inputs]
            if visited & boost_consumers:
                if not boost:
                    boost.append(boost_identity())
                fingerprint.append({"boost": boost[0]})
                if boost[0] is None:
                    uncacheable[group].add(name)
            fingerprints[group][name] = digest(json.dumps(fingerprint).encode())
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


def affected_inventory(plan):
    return (plan.get("mode", "").startswith("affected-")
            or plan.get("state", {}).get("version") == "affected-inventory-only-v1"
            or any(key in plan for key in ("base", "head", "binding", "expected_ref", "plan_sha256")))


def no_cache_context():
    return os.environ.get("VERIFY_NO_CACHE") == "true"


def record_success(previous, plan, groups):
    if affected_inventory(plan):
        raise ValueError("An affected inventory is not successful verification cache state")
    if not set(groups) <= set(GROUPS):
        raise ValueError("Unknown verification group")
    targets = cached_targets(previous) or {}
    return {"version": VERSION, "targets": {
        group: dict(plan["state"]["targets"][group] if group in groups else targets.get(group, {}))
        for group in GROUPS}}


def checkpoint_success(previous, plan, group, names):
    if affected_inventory(plan):
        raise ValueError("Affected inventories cannot populate successful cache state")
    if group not in GROUPS or not set(names) <= set(plan[group]):
        raise ValueError("Invalid completed targets")
    targets = cached_targets(previous) or {}
    result = {"version": VERSION, "targets": {
        key: dict(targets.get(key, {})) for key in GROUPS}}
    for name in names:
        result["targets"][group][name] = plan["state"]["targets"][group][name]
    return result


def verify_selected(plan, marker, run, completed=None, starting=None):
    if affected_inventory(plan) and plan.get("mode") != "affected-no-cache-v1":
        raise ValueError("Affected provenance requires the exact affected mode")
    paths = set(plan["state"]["targets"]["verification"])
    selected = set(plan["verification"])
    for name in sorted(selected):
        marker.mark_failed(Path(name))
    if plan.get("mode") != "affected-no-cache-v1":
        for name in sorted(paths - selected):
            marker.mark_verified(Path(name))
    failed = []
    if selected and completed is None and starting is None:
        summary = run([Path(name) for name in sorted(selected)])
        summary.show()
        if not summary.succeeded():
            raise RuntimeError("Judge verification failed")
        unfinished = [name for name in sorted(selected) if not marker.is_verified(Path(name))]
        if unfinished:
            raise RuntimeError("Verification did not finish: " + ", ".join(unfinished))
    else:
        for name in sorted(selected):
            if starting:
                starting(name)
            summary = run([Path(name)])
            summary.show()
            if not summary.succeeded():
                failed.append(name)
            elif not marker.is_verified(Path(name)):
                raise RuntimeError("Verification did not finish: " + name)
            elif completed:
                completed(name)
    if failed:
        raise RuntimeError("Judge verification failed: " + ", ".join(failed))
    other = "unaffected/not run" if plan.get("mode") == "affected-no-cache-v1" else "reused"
    print(f"Judge tests: verified {len(selected)}, {other} {len(paths - selected)}", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("plan", "judge", "docs", "record", "save"))
    parser.add_argument("--state", type=Path, default=Path(".verify-helper/verified-state.json"))
    parser.add_argument("--selection", type=Path, default=Path(".verify-helper/selection.json"))
    parser.add_argument("--full", action="store_true")
    parser.add_argument("--groups", nargs="+", choices=GROUPS, default=GROUPS)
    parser.add_argument("--affected-base")
    parser.add_argument("--expected-head")
    parser.add_argument("--expected-ref")
    args = parser.parse_args()
    if bool(args.affected_base) != bool(args.expected_head):
        parser.error("--affected-base and --expected-head are required together")
    if bool(args.expected_ref) != bool(args.affected_base):
        parser.error("--expected-ref and --affected-base are required together")
    if args.command != "plan" and args.affected_base:
        parser.error("Affected source options apply only to plan")
    os.chdir(ROOT)
    if args.command == "plan":
        if no_cache_context() and not args.affected_base:
            parser.error("The explicit no-cache event requires explicit affected source options")
        force = args.full or os.environ.get("VERIFY_FULL") == "true"
        if args.affected_base:
            from affected import build_affected_plan
            plan = build_affected_plan(ROOT, args.affected_base, args.expected_head,
                                       expected_ref=args.expected_ref, force=force)
            print(json.dumps({key: value for key, value in plan.items() if key != "state"}, sort_keys=True), flush=True)
        else:
            previous = read_json(args.state)
            plan = build_plan(ROOT, previous, environment=environment_key(), force=force)
        write_json(args.selection, plan)
        if not args.affected_base:
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
        if no_cache_context() and plan.get("mode") != "affected-no-cache-v1":
            parser.error("The explicit no-cache event requires an exact affected plan")
        affected = affected_inventory(plan)
        if affected:
            from affected import record_affected_success, validate_affected_plan
            validate_affected_plan(ROOT, plan)
        if args.command in ("record", "save"):
            if affected:
                if args.command == "save":
                    parser.error("An affected plan cannot be saved as verification cache")
                print(json.dumps(record_affected_success(plan, args.groups), sort_keys=True), flush=True)
            else:
                write_json(args.state, record_success(read_json(args.state), plan, args.groups))
        elif args.command == "judge":
            import onlinejudge.service
            from onlinejudge_verify import verify
            from verification import get_marker, prepare_samples

            logging.basicConfig(level=logging.INFO)
            if affected and list((ROOT / ".verify-helper").glob("timestamps.*.json")):
                parser.error("Affected judge mode requires fresh disposable marker state")
            def starting(name):
                if os.environ.get("VERIFY_JUDGE_CACHE"):
                    from judge_data import prepare_selected
                    prepare_selected(ROOT, plan, Path(os.environ["VERIFY_JUDGE_CACHE"]), drivers=[name])
            verification = get_marker()
            def completed(name):
                write_json(args.state, checkpoint_success(read_json(args.state), plan, "verification", [name]))
                if os.environ.get("VERIFY_JUDGE_CACHE"):
                    from judge_data import mark_ready
                    mark_ready(Path(os.environ["VERIFY_JUDGE_CACHE"]), name)
            def run(paths):
                prepare_samples(ROOT, paths)
                return verify.main(paths, marker=verification, timeout=3000, tle=10, jobs=2)
            with verification:
                verify_selected(plan, verification, run,
                    completed=None if affected else completed, starting=None if affected else starting)
        else:
            from documentation import generate
            generate(plan)


if __name__ == "__main__":
    main()
