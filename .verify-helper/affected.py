"""Draft: exact-Git-source scope, without successful-result cache semantics."""

import argparse
import hashlib
import os
import json
from pathlib import Path
import posixpath
import re
import subprocess


MODE = "affected-no-cache-v1"
BINDING_PATH = ".verify-helper/affected-binding.json"
BINDING_DOMAIN = "qandori/cp-library/affected-push-binding-v1"
GROUPS = ("headers", "unit", "sanitize", "verification", "bundles")
LIBRARIES = {"components", "convolution", "fps", "data_structure", "graph", "math", "string"}
CHECK_INPUTS = {
    "test/verify.py", "test/requirements.txt", ".verify-helper/config.toml",
    ".verify-helper/check-docs.py", ".verify-helper/documentation.py",
    ".verify-helper/verification.py",
}


def git(root, *arguments):
    return subprocess.check_output(["git", "-C", str(root), *arguments])


def require(condition, message):
    if not condition:
        raise ValueError(message)


def digest(value):
    return hashlib.sha256(value).hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode()


class SourceTree:
    def __init__(self, root, commit):
        self.root, self.commit = root, commit
        self.entries, self.contents, self.dependencies = {}, {}, {}
        for item in git(root, "ls-tree", "-rz", "--full-tree", commit).split(b"\0"):
            if not item:
                continue
            fields, name = item.split(b"\t", 1)
            mode, kind, oid = fields.decode().split()
            self.entries[name.decode()] = (mode, kind, oid)
        self.uncertain = set()

    def content(self, name):
        if name not in self.contents:
            entry = self.entries.get(name)
            if entry is None:
                self.contents[name] = None
            else:
                require(entry[0] in ("100644", "100755") and entry[1] == "blob",
                        "Unsupported non-regular source: " + name)
                self.contents[name] = git(self.root, "cat-file", "blob", entry[2])
        return self.contents[name]

    def includes(self, name):
        if name in self.dependencies:
            return self.dependencies[name]
        self.dependencies[name] = result = set()
        data = self.content(name)
        if data is None:
            return result
        source = data.decode("utf-8").replace("\\\n", "")
        if re.search(r"\b__has_include\b|^\s*#\s*(?:include_next|embed)\b", source, re.MULTILINE):
            self.uncertain.add(name)
        for directive in re.findall(r"^\s*#\s*include\b([^\n]*)", source, re.MULTILINE):
            match = re.match(r'\s*(?:"([^"]+)"|<([^>]+)>)', directive)
            if match is None:
                self.uncertain.add(name)
                continue
            quoted, angled = match.groups()
            candidates = [angled] if angled else [posixpath.join(posixpath.dirname(name), quoted), quoted]
            for candidate in candidates:
                require(not candidate.startswith("/"), "Absolute include: " + name)
                prefix = ""
                parts = candidate.split("/")
                for index, part in enumerate(parts):
                    prefix = posixpath.normpath(posixpath.join(prefix, part))
                    require(prefix != ".." and not prefix.startswith("../"), "Include escapes source tree: " + name)
                    entry = self.entries.get(prefix)
                    if index < len(parts) - 1 and entry is not None:
                        raise ValueError("Non-directory include path component: " + prefix)
                path = posixpath.normpath(candidate)
                require(not path.startswith(("/", "../")) and path != "..",
                        "Include escapes source tree: " + name)
                result.add(path)
                if self.content(path) is not None:
                    break
        return result

    def closure(self, name):
        visited, pending = set(), [name]
        while pending:
            path = pending.pop()
            if path not in visited:
                visited.add(path)
                pending.extend(self.includes(path) - visited)
        return visited

    def targets(self):
        headers = sorted(name for name in self.entries
                         if name.split("/", 1)[0] in LIBRARIES and name.endswith(".hpp"))
        units = sorted(name for name in self.entries if name.startswith("test/unit/") and name.endswith(".cpp"))
        judges = sorted(name for name in self.entries if name.startswith("test/") and name.endswith(".test.cpp"))
        return {"headers": headers, "unit": units, "sanitize": units,
                "verification": judges, "bundles": headers}


def checked_source(root, base, head):
    for name, commit in (("base", base), ("head", head)):
        require(isinstance(commit, str) and re.fullmatch("[0-9a-f]{40}", commit),
                name + " must be an exact full commit ID")
        require(git(root, "rev-parse", "--verify", commit + "^{commit}").decode().strip() == commit,
                "Cannot resolve exact " + name)
    require(git(root, "rev-parse", "HEAD").decode().strip() == head, "Checkout is not the expected head")
    subprocess.run(["git", "-C", str(root), "merge-base", "--is-ancestor", base, head], check=True)
    require(not git(root, "diff", "--name-only", "HEAD", "--"), "Tracked checkout differs from head")
    require(not git(root, "ls-files", "--others", "--exclude-standard"), "Unexpected untracked checkout files")



def binding_source_digest(old, new, ref):
    changed = sorted(name for name in old.entries.keys() | new.entries.keys()
                     if name != BINDING_PATH and old.entries.get(name) != new.entries.get(name))
    def signature(tree, name):
        entry = tree.entries.get(name)
        if entry is None:
            return None
        data = git(tree.root, "cat-file", "blob", entry[2]) if entry[1] == "blob" else None
        return [*entry, digest(data) if data is not None else None]
    return digest(canonical({"domain": BINDING_DOMAIN, "repository": "qandori/cp-library",
                             "base": old.commit, "ref": ref,
                             "changes": [[name, signature(old, name), signature(new, name)] for name in changed]}))


def read_binding(tree):
    entry = tree.entries.get(BINDING_PATH)
    require(entry is not None and entry[:2] == ("100644", "blob"), "Binding must be the fixed regular data file")
    def unique(pairs):
        result = {}
        for name, value in pairs:
            require(name not in result, "Duplicate binding field")
            result[name] = value
        return result
    raw = tree.content(BINDING_PATH)
    value = json.loads(raw, object_pairs_hook=unique)
    require(isinstance(value, dict) and set(value) == {"version", "base", "ref", "source_digest"}, "Invalid binding schema")
    require(type(value["version"]) is int and value["version"] == 1, "Unsupported binding version")
    for name, size in (("base", 40), ("source_digest", 64)):
        require(isinstance(value[name], str) and re.fullmatch("[0-9a-f]{%d}" % size, value[name]), "Invalid binding " + name)
    require(isinstance(value["ref"], str)
            and (value["ref"] in ("refs/heads/codex/edit-distance-trim-20261018",
                                 "refs/heads/codex/fps-scalar-return-20261018")
                 or re.fullmatch(r"refs/heads/codex/(?:dwm|matrix)-[a-z0-9][a-z0-9._-]*", value["ref"]))
            and ".." not in value["ref"], "Binding requires an exact temporary branch")
    require(raw == canonical(value) + b"\n", "Binding must use exact canonical JSON plus one newline")
    return value


def validate_binding(old, new, expected_ref):
    binding = read_binding(new)
    require(binding["base"] == old.commit, "Binding baseline differs from expected baseline")
    require(binding["ref"] == expected_ref, "Binding branch differs from expected branch")
    require(binding["source_digest"] == binding_source_digest(old, new, expected_ref), "Binding source digest mismatch")
    return binding


def verify_push_binding(root, event, context, expected_ref):
    require(context.get("GITHUB_EVENT_NAME") == "push", "Push-only reviewed binding required")
    require(context.get("GITHUB_REPOSITORY") == "qandori/cp-library", "Wrong repository")
    require(context.get("GITHUB_REF") == expected_ref, "Wrong temporary branch")
    require(isinstance(event, dict) and event.get("deleted") is False and event.get("ref") == expected_ref, "Invalid push payload")
    head = context.get("GITHUB_SHA")
    require(event.get("after") == head, "Push payload differs from event head")
    require(isinstance(head, str) and re.fullmatch("[0-9a-f]{40}", head), "Invalid push head")
    current = SourceTree(root, head)
    binding = read_binding(current)
    checked_source(root, binding["base"], head)
    validate_binding(SourceTree(root, binding["base"]), current, expected_ref)
    return {"base": binding["base"], "head": head, "ref": expected_ref,
            "source_digest": binding["source_digest"]}


def build_affected_plan(root, base, head, *, expected_ref, force=False):
    root = Path(root).resolve()
    checked_source(root, base, head)
    return source_affected_plan(root, base, head, expected_ref=expected_ref, force=force)


def source_affected_plan(root, base, head, *, expected_ref, force=False):
    """Build an immutable-source inventory; this is not evidence of execution."""
    old, new = SourceTree(root, base), SourceTree(root, head)
    old_targets, targets = old.targets(), new.targets()
    changed = sorted(name for name in old.entries.keys() | new.entries.keys()
                     if old.entries.get(name) != new.entries.get(name))
    changed_set = set(changed)
    if not force:
        require(not changed_set.intersection(CHECK_INPUTS),
                "Check implementation/configuration changed; use full verification or a separately reviewed extension")
    closures, inputs = {}, set()
    for tree, inventory in ((old, old_targets), (new, targets)):
        for name in set().union(*(set(paths) for paths in inventory.values())):
            closure = tree.closure(name)
            closures[tree.commit, name] = closure
            inputs.update(closure)
    extras = sorted(changed_set - inputs)
    binding = validate_binding(old, new, expected_ref)
    selected, fingerprints, reasons = {}, {}, {}
    for group, names in targets.items():
        selected[group], fingerprints[group], reasons[group] = [], {}, {}
        for name in names:
            current = closures[head, name]
            previous = closures.get((base, name), set())
            why = sorted((current | previous) & changed_set)
            uncertain = bool(current & new.uncertain or previous & old.uncertain)
            if force or name not in old_targets[group] or why or uncertain:
                selected[group].append(name)
                reasons[group][name] = {"changed_dependencies": why, "uncertain": uncertain,
                                        "new_target": name not in old_targets[group], "forced": force}
            source_inputs = [(path, new.entries.get(path)) for path in sorted(current)]
            fingerprints[group][name] = digest(canonical(source_inputs))
    plan = {**selected, "mode": MODE, "base": base, "head": head,
            "parent": git(root, "rev-list", "--parents", "-n", "1", head).decode().strip().split()[1:],
            "base_tree": git(root, "rev-parse", base + "^{tree}").decode().strip(),
            "head_tree": git(root, "rev-parse", head + "^{tree}").decode().strip(),
            "changed_paths": changed, "reviewed_extra_paths": extras,
            "binding": binding, "binding_path": BINDING_PATH, "expected_ref": expected_ref,
            "scope_reasons": reasons, "full": force,
            "reason": "Full verification requested; no cache" if force else "Exact-source affected scope; unaffected checks NOT RUN",
            "state": {"version": "affected-inventory-only-v1", "targets": fingerprints}}
    plan["plan_sha256"] = digest(canonical(plan))
    return plan


def validate_affected_plan(root, plan):
    require(plan.get("mode") == MODE, "Unsupported affected plan")
    expected = build_affected_plan(root, plan["base"], plan["head"],
                                   expected_ref=plan["expected_ref"], force=plan["full"])
    require(plan == expected, "Selection changed or source provenance no longer matches")


def record_affected_success(plan, groups):
    require(set(groups) <= set(GROUPS), "Unknown verification group")
    return {"mode": MODE, "event": "selected_checks_completed", "head": plan["head"],
            "plan_sha256": plan["plan_sha256"],
            "groups": {group: plan[group] for group in groups},
            "unselected_checks": "not_run", "successful_cache_written": False}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verify-push", action="store_true", required=True)
    parser.add_argument("--expected-ref", required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text())
    binding = verify_push_binding(root, event, os.environ, args.expected_ref)
    with open(os.environ["GITHUB_OUTPUT"], "a") as stream:
        for name, value in binding.items():
            stream.write(name + "=" + value + "\n")
    print(json.dumps({"event": "reviewed_push_binding", **binding}, sort_keys=True), flush=True)


if __name__ == "__main__":
    main()
