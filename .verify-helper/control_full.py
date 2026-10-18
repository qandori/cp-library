"""One bounded, control-only full validation without successful-result storage."""

import json
import os

from affected import (GROUPS, SourceTree, binding_source_digest, canonical, checked_source,
                      digest, git, require)


MODE = "control-full-no-cache-v1"
REF = "refs/heads/codex/ci-tree-reuse-20261018"
BASE = "82fe5f034fca2805aeb59aee1be56216493ea5de"
TRAILER = "Verify-Mode: control-full"
PATHS = {
    ".github/workflows/verify.yml", ".github/workflows/macos-portability.yml",
    ".verify-helper/affected.py", ".verify-helper/affected_test.py",
    ".verify-helper/ci.py", ".verify-helper/ci_test.py",
    ".verify-helper/no_cache_route.py", ".verify-helper/no_cache_route_test.py",
    ".verify-helper/tree_baseline.py", ".verify-helper/tree_baseline_test.py",
    ".verify-helper/control_full.py", ".verify-helper/control_full_test.py",
    ".verify-helper/review/tree-equivalent-baseline.md",
}
IDENTITY_PREFIX = "uname -a\ng++ --version\ng++ -dumpmachine\nclang++ --version\n"


def marked(root, commit):
    message = git(root, "show", "-s", "--format=%B", commit).decode().rstrip()
    return bool(message) and message.splitlines()[-1] == TRAILER


def expected_workflow(source, name):
    def replace(before, after):
        nonlocal source
        require(source.count(before) == 1, "Unsupported bootstrap workflow source")
        source = source.replace(before, after)
    if name == "verify.yml":
        replace("if: steps.route.outputs.mode == 'no-cache'\n        with:\n          python-version: '3.12'",
                "if: steps.route.outputs.mode == 'no-cache' || steps.route.outputs.mode == 'control-full'\n        with:\n          python-version: '3.12'")
        replace("- name: Install dependencies without cache\n        if: steps.route.outputs.mode == 'no-cache'",
                "- name: Install dependencies without cache\n        if: steps.route.outputs.mode == 'no-cache' || steps.route.outputs.mode == 'control-full'")
        replace("      - name: Run unit tests with GCC and Clang\n        run: |\n",
                "      - name: Run unit tests with GCC and Clang\n        run: |\n"
                + "".join("          " + line + "\n" for line in IDENTITY_PREFIX.splitlines()))
    anchor = "      - name: " + ("Run unit tests with GCC and Clang" if name == "verify.yml" else "Check Apple Clang") + "\n"
    replace(anchor, "      - name: Select exact-head full scope without cache\n"
            "        if: steps.route.outputs.mode == 'control-full'\n"
            "        env:\n          VERIFY_EXPECTED_HEAD: ${{ steps.route.outputs.head }}\n"
            '        run: python .verify-helper/ci.py plan --control-full --expected-head "$VERIFY_EXPECTED_HEAD"\n' + anchor)
    return source


def check_changes(root, head):
    old, current = SourceTree(root, BASE), SourceTree(root, head)
    changed = {name for name in old.entries.keys() | current.entries.keys()
               if old.entries.get(name) != current.entries.get(name)}
    require(bool(changed) and changed <= PATHS, "Control validation changed a non-control source")
    git(root, "merge-base", "--is-ancestor", BASE, head)
    for workflow in ("verify.yml", "macos-portability.yml"):
        path = ".github/workflows/" + workflow
        expected = expected_workflow(old.content(path).decode(), workflow).encode()
        require(current.content(path) == expected, "Control validation changed unapproved workflow execution")
    return current


def select_event(root, event, context):
    import no_cache_route as route
    head, source, source_ref = route.event_source(root, event, context)
    kind, ref = context.get("GITHUB_EVENT_NAME"), context.get("GITHUB_REF")
    pair = route.parents(root, head)
    merge_source = pair[1] if ref == route.MAIN_REF and len(pair) == 2 else None
    triggered = source_ref == REF or marked(root, source) or (merge_source is not None and marked(root, merge_source))
    if not triggered:
        return None
    require(context.get("GITHUB_REPOSITORY") == route.REPOSITORY, "Wrong control repository")
    checked_source(root, BASE, head)
    check_changes(root, head)
    if kind == "push":
        require(event.get("deleted") is False and event.get("after") == head and event.get("ref") == ref,
                "Control push differs from checkout")
        if ref == REF:
            require(source == head, "Control source differs from checkout")
        else:
            require(ref == route.MAIN_REF and pair == [BASE, merge_source] and event.get("before") == BASE
                    and merge_source is not None and route.same_tree(root, head, merge_source),
                    "Control main requires the exact direct merge")
            require(marked(root, head), "Control main requires its explicit marker")
            source = merge_source
    elif kind == "pull_request":
        pull = event.get("pull_request", {})
        require(source_ref == REF and pull.get("head", {}).get("ref") == REF.removeprefix("refs/heads/")
                and pull.get("head", {}).get("repo", {}).get("full_name") == route.REPOSITORY,
                "Wrong control PR source")
        require(pull.get("base", {}).get("sha") == BASE and pull.get("base", {}).get("ref") == "main"
                and pull.get("base", {}).get("repo", {}).get("full_name") == route.REPOSITORY,
                "Wrong control PR baseline")
        require(type(event.get("number")) is int and ref == "refs/pull/" + str(event["number"]) + "/merge"
                and pair == [BASE, source] and route.same_tree(root, head, source), "Wrong control PR merge")
    else:
        raise ValueError("Control validation accepts push and pull_request only")
    require(marked(root, source), "Control source requires its explicit marker")
    return {"mode": "control-full", "base": BASE, "head": head, "source_head": source, "ref": REF,
            "source_digest": binding_source_digest(SourceTree(root, BASE), SourceTree(root, source), REF), "event": kind}


def source_coverage(tree):
    return {group: {target: digest(canonical([(path, tree.entries.get(path))
                                             for path in sorted(tree.closure(target))]))
                   for target in targets} for group, targets in tree.targets().items()}


def build_plan(root, head):
    import ci
    checked_source(root, head, head)
    tree = check_changes(root, head)
    plan = ci.build_plan(root, None, environment=ci.environment_key(), force=True)
    require(all(plan[group] == tree.targets()[group] for group in GROUPS), "Full plan differs from exact Git inventory")
    return {**plan, "mode": MODE, "head": head, "head_tree": git(root, "rev-parse", head + "^{tree}").decode().strip(),
            "source_coverage_sha256": digest(canonical(source_coverage(tree))), "cache_reads": False, "cache_writes": False}


def validate_plan(root, plan):
    require(plan == build_plan(root, plan["head"]), "Full control plan or source provenance changed")


def completed(plan, groups):
    require(set(groups) <= set(GROUPS), "Unknown full validation group")
    return {"event": "control_full_checks_completed", "mode": MODE, "head": plan["head"],
            "head_tree": plan["head_tree"], "source_coverage_sha256": plan["source_coverage_sha256"],
            "groups": {group: plan[group] for group in groups}, "cache_written": False,
            "intrinsic_checks": "not_run_control_only"}
