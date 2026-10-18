"""Local, synthetic receipts only: no compilers, network, or Actions dispatch."""

import copy
import json
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

import affected_test
import check_context
import control_delta as control
import no_cache_route as routing
from affected import GROUPS, SourceTree, canonical, digest


LINUX, MAC = tuple(control.PLATFORMS)
BOOST = {"libboost-dev": "1.83.0", "libboost1.83-dev": "1.83.0"}


def runtime(system="Linux"):
    keys = {group: json.dumps({"platform": [system, "x86_64" if system == "Linux" else "arm64", "fixture"]})
            for group in GROUPS}
    return check_context.runtime_context(keys, {}, system_packages=BOOST if system == "Linux" else {})


def row(number=1, *, workflow=LINUX, head="a" * 40, kind="anchor", attempt=1):
    return {"kind": kind, "head": head, "workflow": workflow, "id": number, "attempt": attempt}


def input_tree(rows=None, *, value=None, raw=None):
    value = {"version": control.VERSION, "receipts": rows or []} if value is None else value
    return SimpleNamespace(content=lambda path: canonical(value) + b"\n" if raw is None else raw)


class ReceiptInputTests(unittest.TestCase):
    def test_empty_or_bounded_canonical_inputs_are_accepted(self):
        self.assertEqual(control.inputs(input_tree()), [])
        rows = [row(number) for number in range(1, control.MAX_RECEIPTS + 1)]
        self.assertEqual(control.inputs(input_tree(rows)), rows)

    def test_top_level_schema_version_and_bounds_are_strict(self):
        values = [[], None, {}, {"version": True, "receipts": []}, {"version": 2, "receipts": []},
                  {"version": 1, "receipts": [], "extra": None}, {"version": 1, "receipts": {}},
                  {"version": 1, "receipts": [row(n) for n in range(1, control.MAX_RECEIPTS + 2)]}]
        for value in values:
            with self.subTest(value=value), self.assertRaises(ValueError):
                control.inputs(input_tree(raw=canonical(value) + b"\n"))

    def test_receipt_schema_scalar_types_and_attempt_bound_are_strict(self):
        bad = [None, [], {}, {**row(), "extra": 1}, {key: value for key, value in row().items() if key != "id"}]
        for key, values in {"kind": ("cache", None, []), "workflow": ("verify", None, [], {}),
                            "head": ("a" * 39, "A" * 40, None), "id": (0, -1, True, "1"),
                            "attempt": (0, -1, True, "1", control.proof.MAX_ATTEMPTS + 1)}.items():
            bad.extend({**row(), key: value} for value in values)
        for value in bad:
            with self.subTest(value=value), self.assertRaises(ValueError):
                control.inputs(input_tree([value]))

    def test_duplicate_run_attempt_is_rejected_across_platforms_and_kinds(self):
        for duplicate in (row(), row(workflow=MAC), row(kind="delta")):
            with self.subTest(duplicate=duplicate), self.assertRaisesRegex(ValueError, "Duplicate"):
                control.inputs(input_tree([row(), duplicate]))

    def test_noncanonical_duplicate_json_and_missing_inputs_are_rejected(self):
        for raw in (b'{"receipts":[],"version":1}', b'{ "receipts": [], "version": 1 }\n',
                    b'{"receipts":[],"version":1,"version":1}\n', b'null\n', b'not json'):
            with self.subTest(raw=raw), self.assertRaises(ValueError):
                control.inputs(input_tree(raw=raw))
        with self.assertRaises(ValueError):
            control.inputs(SimpleNamespace(content=lambda path: None))

    def test_explicit_fresh_groups_are_optional_sorted_unique_known_names(self):
        for fresh in ([], ["verification"], sorted(GROUPS)):
            value = {"version": 1, "receipts": [row()], "fresh_groups": fresh}
            with self.subTest(fresh=fresh):
                self.assertEqual(control.inputs(input_tree(value=value)), [row()])
        for fresh in (None, "verification", {}, ["unknown"], [None], [{}],
                      ["verification", "headers"], ["verification", "verification"]):
            value = {"version": 1, "receipts": [], "fresh_groups": fresh}
            with self.subTest(fresh=fresh), self.assertRaises(ValueError):
                control.inputs(input_tree(value=value))


class UpstreamControlReviewTests(unittest.TestCase):
    path = ".verify-helper/ci_test.py"
    before, after = control.REVIEWED_UPSTREAM_TEST[path]

    def changes(self, old_entry=None, *, original_entry=None, other=False):
        original = {self.path: self.before if original_entry is None else original_entry,
                    ".verify-helper/ci.py": ("100644", "blob", "a" * 40)}
        upstream = {**original, self.path: self.after if old_entry is None else old_entry}
        if other:
            upstream[".verify-helper/ci.py"] = ("100644", "blob", "b" * 40)
        current = {**upstream, control.INPUT: ("100644", "blob", "c" * 40)}
        trees = [SimpleNamespace(entries=entries, content=lambda path: b"reviewed workflow")
                 for entries in (original, upstream, current)]
        with (patch.object(control, "SourceTree", side_effect=trees),
              patch.object(control, "git") as git,
              patch.object(control.control_full, "expected_workflow", return_value="reviewed workflow") as expected):
            result = control.changes("root", "base", "head")
        self.assertIs(result, trees[-1])
        git.assert_called_once_with("root", "merge-base", "--is-ancestor", control.ANCHOR_BASE, "base")
        self.assertEqual(expected.call_count, 2)

    def test_exact_reviewed_test_blob_pair_is_accepted(self):
        self.changes()

    def test_unchanged_historical_test_is_still_accepted(self):
        self.changes(self.before)

    def test_near_match_and_unreviewed_original_test_are_rejected(self):
        for entry in (("100644", "blob", self.after[2][:-1] + "0"),
                      ("100644", "blob", "f" * 40)):
            with self.subTest(entry=entry), self.assertRaisesRegex(ValueError, "renewed review"):
                self.changes(entry)
        with self.assertRaisesRegex(ValueError, "renewed review"):
            self.changes(original_entry=("100644", "blob", "f" * 40))

    def test_mode_or_object_kind_changes_are_rejected(self):
        for entry in (("100755", "blob", self.after[2]), ("120000", "blob", self.after[2]),
                      ("160000", "commit", self.after[2])):
            with self.subTest(entry=entry), self.assertRaisesRegex(ValueError, "renewed review"):
                self.changes(entry)

    def test_other_upstream_helper_change_remains_blocked(self):
        with self.assertRaisesRegex(ValueError, "renewed review"):
            self.changes(other=True)


class RuntimeSchemaTests(unittest.TestCase):
    def test_current_collected_profiles_are_valid_for_both_platforms(self):
        for system in ("Linux", "Darwin"):
            with self.subTest(system=system):
                control.runtime_valid(runtime(system))

    def test_missing_top_level_context_is_not_historical_evidence(self):
        for key in runtime():
            value = runtime()
            value.pop(key)
            with self.subTest(key=key), self.assertRaises(ValueError):
                control.runtime_valid(value)
        with self.assertRaises(ValueError):
            control.runtime_valid(None)

    def test_malformed_context_hashes_and_ambient_presence_fail_closed(self):
        mutations = [lambda v: v.update(profile="legacy"), lambda v: v.update(version=True),
                     lambda v: v.update(groups=None), lambda v: v.update(groups=list(GROUPS)),
                     lambda v: v.update(ambient=None), lambda v: v.update(ambient=list(check_context.AMBIENT_KEYS)),
                     lambda v: v.update(system_packages_sha256="bad"),
                     lambda v: v["groups"].pop("unit"),
                     lambda v: v["groups"].update(unit={"runtime_sha256": "A" * 64}),
                     lambda v: v["groups"].update(unit={"runtime_sha256": "a" * 64, "extra": 1}),
                     lambda v: v["ambient"].pop(check_context.AMBIENT_KEYS[0]),
                     lambda v: v["ambient"].update(CXX={"present": 0, "sha256": None}),
                     lambda v: v["ambient"].update(CXX={"present": True, "sha256": None}),
                     lambda v: v["ambient"].update(CXX={"present": False, "sha256": "a" * 64})]
        for index, mutate in enumerate(mutations):
            value = runtime()
            mutate(value)
            with self.subTest(index=index), self.assertRaises(ValueError):
                control.runtime_valid(value)


class DirectReceiptTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit

    def setUp(self):
        affected_test.AffectedVerification.setUp(self)
        for path in set(control.CODE) | set(control.RECEIPT_CONTROLS) | set(control.PLATFORMS) | set().union(*map(set, control.SETTINGS.values())):
            self.write(path, "# Synthetic check input, never executed.\n")
        self.write(control.INPUT, (canonical({"version": 1, "receipts": []}) + b"\n").decode())
        self.head = self.commit("synthetic receipt source", bind=False)
        self.context = runtime()

    def plan(self, donors=(), *, workflow=LINUX, head=None, mode="control-delta", context=None):
        head = head or self.head
        selected = {"mode": mode, "base": self.base, "head": head, "source_head": head,
                    "ref": control.ANCHOR_REF if mode == "control-anchor" else control.REF, "event": "push"}
        producer = {"run_id": 1, "run_attempt": 1, "job": control.PLATFORMS[workflow][1]}
        return control.make_plan(self.root, selected, workflow, context or self.context, list(donors), producer=producer)

    def donor(self, plan=None, *, executed=None, number=1):
        plan = plan or self.plan(mode="control-anchor")
        return {**row(number, workflow=plan["workflow"], head=plan["head"],
                      kind="anchor" if plan["mode"] == control.ANCHOR_MODE else "delta"),
                "job": number * 10, "log_sha256": "a" * 64, "runtime": copy.deepcopy(plan["runtime"]),
                "checks": copy.deepcopy(plan["checks"]), "sources": copy.deepcopy(plan["sources"]),
                "executed": copy.deepcopy(executed if executed is not None else {group: plan[group] for group in GROUPS})}

    def lines(self, plan, chunks=None):
        chunks = chunks if chunks is not None else [("headers", "unit"), ("sanitize",), ("verification", "bundles")]
        return [json.dumps(record) for record in control.log_records(plan)] + [
            json.dumps(control.completed(plan, groups)) for groups in chunks]

    def completion_index(self, lines):
        return next(index for index, line in enumerate(lines)
                    if json.loads(line)["event"] == "control_evidence_checks_completed")

    def read_receipt(self, plan, identity, responses, log):
        # Event/topology validation is independently covered by route tests.
        with patch.object(control, "select_event", return_value=plan["route"]):
            return control.read_receipt(self.root, identity, "unused", lambda path, token: responses[path],
                                        lambda job, token: log)

    def receipt_fixture(self, plan):
        identity = row(workflow=plan["workflow"], head=plan["head"],
                       kind="anchor" if plan["mode"] == control.ANCHOR_MODE else "delta")
        branch = (control.ANCHOR_REF if identity["kind"] == "anchor" else control.REF).removeprefix("refs/heads/")
        _, name, groups = control.PLATFORMS[plan["workflow"]]
        run = {"id": 1, "run_attempt": 1, "head_sha": plan["head"], "head_branch": branch, "event": "push",
               "path": plan["workflow"], "status": "completed", "conclusion": "success",
               "repository": {"full_name": routing.REPOSITORY}, "head_repository": {"full_name": routing.REPOSITORY}}
        steps = [control.SELECTOR if step == "Select exact-head affected scope without cache" else step
                 for step in routing.BASELINE_JOBS[plan["workflow"]][1]]
        steps += ["Set up job", "Run actions/checkout@v4", "Resolve explicit no-cache event"]
        if name == "verify":
            steps += ["Install dependencies without cache", "Install Boost"]
        job = {"id": 10, "run_id": 1, "run_attempt": 1, "head_sha": plan["head"], "head_branch": branch,
               "name": name, "status": "completed", "conclusion": "success",
               "steps": [{"name": step, "status": "completed", "conclusion": "success"} for step in steps]}
        stored = (["Restore successful verification results", "Cache judge test cases and checkers",
                   "Save completed verification results", "Upload library website", "Install dependencies",
                   "Select affected libraries and tests"] if name == "verify" else
                  ["Run actions/cache/restore@v4", "Run actions/cache/save@v4", "Select affected headers and unit tests"])
        stored += ["Select exact-head affected scope without cache", "Select exact-head full scope without cache"]
        job["steps"] += [{"name": step, "status": "completed", "conclusion": "skipped"} for step in stored]
        job["steps"] += [{"name": "Run actions/setup-python@v5", "status": "completed", "conclusion": conclusion}
                         for conclusion in (("skipped", "success") if name == "verify" else ("success",))]
        jobs = [job]
        if name == "verify":
            jobs += [{"id": index, "name": skipped, "status": "completed", "conclusion": "skipped", "steps": []}
                     for index, skipped in enumerate(("intrinsic", "deploy"), 11)]
        prefix = "/repos/" + routing.REPOSITORY + "/actions/runs/1"
        responses = {prefix: run, prefix + "/attempts/1": copy.deepcopy(run),
                     prefix + "/attempts/1/jobs?per_page=100": {"total_count": len(jobs), "jobs": jobs}}
        runtime_lines = (["Image: ubuntu-24.04", "Linux runner 6.0 x86_64 x86_64 x86_64 GNU/Linux",
                          "g++ (Ubuntu 13.3) 13.3.0", "x86_64-linux-gnu", "Ubuntu clang version 18.1.3",
                          "Target: x86_64-pc-linux-gnu"] if name == "verify" else
                         ["Image: macos-14-arm64", "Darwin runner 23.6 Darwin Kernel Version arm64",
                          "Apple clang version 15.0.0", "Target: arm64-apple-darwin23.6.0"])
        log = "\n".join("2026-01-01T00:00:00.000Z " + line
                        for line in self.lines(plan, [groups]) + runtime_lines) + "\n"
        return identity, responses, log

    def second_attempt_fixture(self):
        original = self.plan(mode="control-anchor")
        plan = control.make_plan(self.root, original["route"], LINUX, self.context, [],
                                 producer={"run_id": 1, "run_attempt": 2, "job": "verify"})
        identity, responses, log = self.receipt_fixture(plan)
        prefix = "/repos/" + routing.REPOSITORY + "/actions/runs/1"
        identity["attempt"] = 2
        responses[prefix]["run_attempt"] = 2
        responses[prefix + "/attempts/2"] = copy.deepcopy(responses[prefix])
        current = copy.deepcopy(responses[prefix + "/attempts/1/jobs?per_page=100"])
        for job in current["jobs"]:
            job["id"] += 10
            if "run_attempt" in job:
                job["run_attempt"] = 2
        responses[prefix + "/attempts/2/jobs?per_page=100"] = current
        return plan, identity, responses, log, prefix

    def reject_prior_attempt(self, mutate):
        plan, identity, responses, log, prefix = self.second_attempt_fixture()
        mutate(responses, prefix)
        with self.assertRaises(ValueError):
            self.read_receipt(plan, identity, responses, log)

    def test_anchor_is_full_and_cannot_inherit_receipts(self):
        plan = self.plan(mode="control-anchor")
        self.assertEqual({group: plan[group] for group in GROUPS}, plan["inventory"])
        self.assertTrue(plan["full"])
        self.assertTrue(all(not value for value in plan["inherited"].values()))
        for key in ("cache_reads", "cache_writes", "artifact_writes"):
            self.assertIs(plan[key], False)
        with self.assertRaisesRegex(ValueError, "Anchor cannot inherit"):
            self.plan([self.donor(plan)], mode="control-anchor")

    def test_direct_executed_targets_only_are_reused(self):
        original = self.plan()
        direct = {group: original[group][:1] for group in GROUPS}
        donor = self.donor(original, executed=direct)
        plan = self.plan([donor])
        for group in GROUPS:
            self.assertEqual(plan[group], original[group][1:])
            self.assertEqual(set(plan["inherited"][group]), set(direct[group]))
            self.assertEqual(plan["inherited"][group][direct[group][0]],
                             {key: donor[key] for key in ("head", "id", "attempt", "job")})
        self.assertFalse(plan["full"])

    def test_inherited_coverage_never_becomes_second_generation_execution(self):
        first = self.donor()
        inherited_plan = self.plan([first])
        self.assertTrue(all(not inherited_plan[group] for group in GROUPS))
        identity, responses, log = self.receipt_fixture(inherited_plan)
        receipt, logged = self.read_receipt(inherited_plan, identity, responses, log)
        self.assertEqual(receipt["executed"], {group: [] for group in GROUPS})
        self.assertEqual(logged, inherited_plan)
        fresh = self.plan([receipt])
        self.assertEqual({group: fresh[group] for group in GROUPS}, fresh["inventory"])
        self.assertTrue(all(not value for value in fresh["inherited"].values()))

    def test_source_dependency_change_requires_only_matching_fresh_coverage(self):
        donor = self.donor()
        self.write("math/unrelated.hpp", "#pragma once\nusing changed_type = long;\n")
        changed = self.commit("changed target dependency", bind=False)
        plan = self.plan([donor], head=changed)
        expected = {"headers": ["math/unrelated.hpp"], "unit": ["test/unit/unrelated.cpp"],
                    "sanitize": ["test/unit/unrelated.cpp"], "verification": ["test/unrelated.test.cpp"],
                    "bundles": ["math/unrelated.hpp"]}
        self.assertEqual({group: plan[group] for group in GROUPS}, expected)

    def test_runtime_or_check_mismatch_requires_fresh_group(self):
        for field in ("runtime", "checks"):
            for group in GROUPS:
                donor = self.donor()
                if field == "runtime":
                    donor[field]["groups"][group]["runtime_sha256"] = "f" * 64
                else:
                    donor[field][group] = "f" * 64
                with self.subTest(field=field, group=group):
                    plan = self.plan([donor])
                    self.assertEqual(plan[group], plan["inventory"][group])
                    self.assertTrue(all(not plan[other] for other in GROUPS if other != group))

    def test_explicit_fresh_verification_only_adds_execution_and_never_inherits_that_group(self):
        donor = self.donor()
        rows = [row(1, head=self.head), row(2, workflow=MAC, head=self.head)]
        self.write(control.INPUT, (canonical({"version": 1, "receipts": rows,
                                              "fresh_groups": ["verification"]}) + b"\n").decode())
        head = self.commit("explicitly fresh judge coverage", bind=False)
        plan = self.plan([donor], head=head)
        self.assertEqual(plan["verification"], plan["inventory"]["verification"])
        self.assertEqual(plan["inherited"]["verification"], {})
        for group in GROUPS:
            if group != "verification":
                self.assertEqual(plan[group], [])
                self.assertEqual(set(plan["inherited"][group]), set(plan["inventory"][group]))
        self.assertEqual(control.logged_plan(self.root, self.lines(plan), head, LINUX)[0], plan)

    def test_check_implementation_change_invalidates_all_direct_coverage(self):
        donor = self.donor()
        for path in (".verify-helper/control_delta.py", ".verify-helper/ci.py"):
            with self.subTest(path=path):
                self.write(path, "# Changed check implementation, including gate-only edits.\n")
                changed = self.commit("changed check implementation", bind=False)
                plan = self.plan([donor], head=changed)
                self.assertEqual({group: plan[group] for group in GROUPS}, plan["inventory"])
                self.assertTrue(all(plan["checks"][group] != donor["checks"][group] for group in GROUPS))
                self.assertTrue(all(not inherited for inherited in plan["inherited"].values()))

    def test_uncertain_includes_are_fresh_even_with_matching_source_receipt(self):
        self.write("math/unrelated.hpp", '#pragma once\n#include UNKNOWN_HEADER\n')
        self.head = self.commit("uncertain dependency", bind=False)
        plan = self.plan([self.donor()])
        self.assertIn("math/unrelated.hpp", plan["headers"])
        self.assertIn("test/unit/unrelated.cpp", plan["unit"])
        self.assertNotIn("components/dynamic_bit_vector.hpp", plan["headers"])

    def test_platforms_are_separate_and_mac_has_no_linux_only_groups(self):
        mac = self.plan(workflow=MAC, mode="control-anchor", context=runtime("Darwin"))
        self.assertEqual(mac["platform"], "mac")
        for group in ("headers", "unit"):
            self.assertEqual(mac[group], mac["inventory"][group])
        for group in ("sanitize", "verification", "bundles"):
            self.assertEqual(mac[group], [])
        with self.assertRaisesRegex(ValueError, "platform differs"):
            self.plan([self.donor(mac)])
        with self.assertRaisesRegex(ValueError, "Unsupported"):
            control.completed(mac, ("sanitize",))
        _, executed = control.logged_plan(self.root, self.lines(mac, [("headers",), ("unit",)]), self.head, MAC)
        self.assertEqual(set(executed), {"headers", "unit"})

    def test_complete_disjoint_group_logs_match_exact_plan(self):
        plan = self.plan()
        logged, executed = control.logged_plan(self.root, self.lines(plan), self.head, LINUX)
        self.assertEqual(logged, plan)
        self.assertEqual(executed, {group: plan[group] for group in GROUPS})

    def test_missing_duplicate_and_wrong_group_logs_are_rejected(self):
        plan = self.plan()
        lines = self.lines(plan)
        index = self.completion_index(lines)
        variants = [lines[:-1], lines + lines[index:index + 1], lines + lines[:1]]
        wrong = json.loads(lines[index])
        wrong["executed_now"]["headers"] = {"count": 0, "targets_sha256": digest(canonical([]))}
        variants.append(lines[:index] + [json.dumps(wrong)] + lines[index + 1:])
        for lines in variants:
            with self.subTest(lines=len(lines)), self.assertRaises(ValueError):
                control.logged_plan(self.root, lines, self.head, LINUX)

    def test_log_cannot_claim_inherited_targets_were_executed(self):
        plan = self.plan([self.donor()])
        lines = self.lines(plan)
        index = self.completion_index(lines)
        completed = json.loads(lines[index])
        completed["executed_now"]["headers"] = {"count": len(plan["inventory"]["headers"]),
                                                  "targets_sha256": digest(canonical(plan["inventory"]["headers"]))}
        lines[index] = json.dumps(completed)
        with self.assertRaisesRegex(ValueError, "completion differs"):
            control.logged_plan(self.root, lines, self.head, LINUX)

    def test_plan_or_completion_identity_tampering_is_rejected(self):
        plan = self.plan()
        for field, value in (("head", "f" * 40), ("plan_sha256", "f" * 64),
                             ("inherited_checks", "executed"), ("cache_written", True)):
            lines = self.lines(plan)
            index = self.completion_index(lines)
            completion = json.loads(lines[index])
            completion[field] = value
            lines[index] = json.dumps(completion)
            with self.subTest(field=field), self.assertRaises(ValueError):
                control.logged_plan(self.root, lines, self.head, LINUX)
        lines = self.lines(plan)
        header = json.loads(lines[0])
        header["plan_sha256"] = "f" * 64
        lines[0] = json.dumps(header)
        with self.assertRaisesRegex(ValueError, "differs"):
            control.logged_plan(self.root, lines, self.head, LINUX)

    def test_full_inventory_chunk_transport_rejects_missing_duplicate_or_offset_drift(self):
        # Match the current full inventory without using the mutable outer checkout.
        inventory = SourceTree(self.root, self.head).targets()
        for group, count, template in (("headers", 127, "math/chunk_%03d.hpp"),
                                       ("unit", 105, "test/unit/chunk_%03d.cpp"),
                                       ("verification", 122, "test/chunk_%03d.test.cpp")):
            for index in range(count - len(inventory[group])):
                self.write(template % index, "#pragma once\n" if group == "headers" else "// Synthetic target.\n")
        self.head = self.commit("full-sized target inventory", bind=False)
        plan = self.plan([self.donor()])
        self.assertEqual({group: len(targets) for group, targets in plan["inventory"].items()},
                         {"headers": 127, "unit": 105, "sanitize": 105, "verification": 122, "bundles": 127})
        self.assertGreater(len(canonical(plan)), control.LOG_LINE_LIMIT)
        records = control.log_records(plan)
        self.assertTrue(all(len(canonical(record)) <= control.LOG_LINE_LIMIT for record in records))
        self.assertTrue(all(len(record["targets"]) <= control.TARGET_CHUNK
                            for record in records if record["event"] == "control_evidence_targets"))
        lines = self.lines(plan)
        self.assertEqual(control.logged_plan(self.root, lines, self.head, LINUX)[0], plan)
        index = next(index for index, record in enumerate(records)
                     if record["event"] == "control_evidence_targets" and record.get("donor") == 0
                     and record["group"] == "headers" and record["offset"] == control.TARGET_CHUNK)
        wrong = copy.deepcopy(records[index])
        wrong["offset"] += 1
        variants = [lines[:index] + lines[index + 1:], lines[:index] + lines[index:index + 1] + lines[index:],
                    lines[:index] + [json.dumps(wrong)] + lines[index + 1:]]
        for variant in variants:
            with self.subTest(lines=len(variant)), self.assertRaises(ValueError):
                control.logged_plan(self.root, variant, self.head, LINUX)

    def test_receipts_require_successful_exact_run_attempt_and_executed_steps(self):
        plan = self.plan(mode="control-anchor")
        for field, value in (("run_attempt", 2), ("head_sha", "f" * 40), ("event", "pull_request"),
                             ("status", "in_progress"), ("conclusion", "failure")):
            identity, responses, log = self.receipt_fixture(plan)
            responses[next(iter(responses))][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.read_receipt(plan, identity, responses, log)
        identity, responses, log = self.receipt_fixture(plan)
        jobs = responses[next(path for path in responses if "/jobs?" in path)]["jobs"]
        jobs[0]["steps"][0]["conclusion"] = "skipped"
        with self.assertRaisesRegex(ValueError, "not executed"):
            self.read_receipt(plan, identity, responses, log)

    def test_second_attempt_requires_and_accepts_explicit_successful_history(self):
        plan, identity, responses, log, prefix = self.second_attempt_fixture()
        request = Mock(side_effect=lambda path, token: responses[path])
        with patch.object(control, "select_event", return_value=plan["route"]):
            receipt, logged = control.read_receipt(self.root, identity, "unused", request, lambda job, token: log)
        self.assertEqual(logged, plan)
        self.assertEqual(receipt["attempt"], 2)
        self.assertEqual(receipt["job"], 20)
        self.assertEqual(receipt["executed"], {group: plan[group] for group in GROUPS})
        self.assertEqual([call.args[0] for call in request.call_args_list],
                         [prefix, prefix + "/attempts/2", prefix + "/attempts/1",
                          prefix + "/attempts/1/jobs?per_page=100", prefix + "/attempts/2/jobs?per_page=100"])

    def test_prior_run_must_be_well_formed_exact_completed_successful_push(self):
        for value in (None, [], {}, "unknown"):
            with self.subTest(run=value):
                self.reject_prior_attempt(lambda responses, prefix: responses.__setitem__(prefix + "/attempts/1", value))
        invalid = {"event": (None, "pull_request", "unknown"),
                   "status": (None, "queued", "in_progress", "unknown"),
                   "conclusion": (None, "skipped", "failure", "cancelled", "timed_out", "unknown"),
                   "id": (2,), "run_attempt": (2,), "head_sha": ("f" * 40,), "head_branch": ("other",)}
        for field, values in invalid.items():
            for value in values:
                with self.subTest(field=field, value=value):
                    self.reject_prior_attempt(lambda responses, prefix: responses[prefix + "/attempts/1"].update({field: value}))
        for field in ("event", "status", "conclusion"):
            with self.subTest(missing=field):
                self.reject_prior_attempt(lambda responses, prefix: responses[prefix + "/attempts/1"].pop(field))

    def test_prior_jobs_require_complete_known_success_or_skip_states(self):
        for value in (None, [], {}, "unknown", {"total_count": 0, "jobs": []}, {"total_count": 1}):
            with self.subTest(payload=value):
                self.reject_prior_attempt(lambda responses, prefix: responses.__setitem__(
                    prefix + "/attempts/1/jobs?per_page=100", value))
        for field, values in {"status": (None, "queued", "in_progress", "unknown"),
                              "conclusion": (None, "failure", "cancelled", "timed_out", "unknown")}.items():
            for value in values:
                with self.subTest(field=field, value=value):
                    self.reject_prior_attempt(lambda responses, prefix: responses[
                        prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].update({field: value}))
            with self.subTest(missing=field):
                self.reject_prior_attempt(lambda responses, prefix: responses[
                    prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].pop(field))
        with self.subTest(primary_job="absent"):
            self.reject_prior_attempt(lambda responses, prefix: responses[
                prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].update(name="unrelated"))
        with self.subTest(primary_job="skipped"):
            self.reject_prior_attempt(lambda responses, prefix: responses[
                prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].update(conclusion="skipped", steps=[]))

    def test_prior_job_steps_must_be_explicit_and_match_execution_disposition(self):
        for value in (None, {}, "unknown", [], [None], ["unknown"]):
            with self.subTest(steps=value):
                self.reject_prior_attempt(lambda responses, prefix: responses[
                    prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].update(steps=value))
        with self.subTest(steps="missing"):
            self.reject_prior_attempt(lambda responses, prefix: responses[
                prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0].pop("steps"))
        with self.subTest(skipped_job="unexpected steps"):
            self.reject_prior_attempt(lambda responses, prefix: responses[
                prefix + "/attempts/1/jobs?per_page=100"]["jobs"][1].update(
                    steps=[{"name": "unexpected", "status": "completed", "conclusion": "success"}]))

    def test_prior_steps_reject_unknown_pending_missing_and_failed_execution(self):
        for field, values in {"status": (None, "queued", "in_progress", "unknown"),
                              "conclusion": (None, "failure", "cancelled", "timed_out", "unknown")}.items():
            for value in values:
                with self.subTest(field=field, value=value):
                    self.reject_prior_attempt(lambda responses, prefix: responses[
                        prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0]["steps"][0].update({field: value}))
            with self.subTest(missing=field):
                self.reject_prior_attempt(lambda responses, prefix: responses[
                    prefix + "/attempts/1/jobs?per_page=100"]["jobs"][0]["steps"][0].pop(field))

    def test_successful_anchor_receipts_record_only_own_platform_execution(self):
        for workflow, system in ((LINUX, "Linux"), (MAC, "Darwin")):
            plan = self.plan(workflow=workflow, context=runtime(system), mode="control-anchor")
            identity, responses, log = self.receipt_fixture(plan)
            with self.subTest(workflow=workflow):
                receipt, logged = self.read_receipt(plan, identity, responses, log)
                self.assertEqual(logged, plan)
                self.assertEqual(receipt["executed"], {group: plan[group] for group in control.PLATFORMS[workflow][2]})
                self.assertEqual(receipt["log_sha256"], digest(log.encode()))

    def test_receipts_reject_missing_or_stale_producer_run_attempt_and_job(self):
        original = self.plan(mode="control-anchor")
        for producer in (None, {"run_id": 2, "run_attempt": 1, "job": "verify"},
                         {"run_id": 1, "run_attempt": 2, "job": "verify"},
                         {"run_id": True, "run_attempt": 1, "job": "verify"},
                         {"run_id": 1, "run_attempt": True, "job": "verify"},
                         {"run_id": 1, "run_attempt": 1, "job": "portable"}):
            plan = control.make_plan(self.root, original["route"], LINUX, self.context, [], producer=producer)
            identity, responses, log = self.receipt_fixture(plan)
            with self.subTest(producer=producer), self.assertRaises(ValueError):
                self.read_receipt(plan, identity, responses, log)

    def test_receipts_cannot_execute_cache_artifact_intrinsic_or_deploy(self):
        for disposition in ("Restore successful verification results", "Upload library website", "Install dependencies",
                            "Select affected libraries and tests", "Select exact-head affected scope without cache",
                            "Select exact-head full scope without cache", "intrinsic", "deploy"):
            plan = self.plan(mode="control-anchor")
            identity, responses, log = self.receipt_fixture(plan)
            jobs = responses[next(path for path in responses if "/jobs?" in path)]["jobs"]
            entries = jobs if disposition in ("intrinsic", "deploy") else jobs[0]["steps"]
            next(entry for entry in entries if entry["name"] == disposition)["conclusion"] = "success"
            with self.subTest(disposition=disposition), self.assertRaises(ValueError):
                self.read_receipt(plan, identity, responses, log)

    def test_receipts_require_exactly_one_successful_python_setup(self):
        for conclusions in ([], ["skipped", "skipped"], ["success", "success"], ["success", "failure"]):
            plan = self.plan(mode="control-anchor")
            identity, responses, log = self.receipt_fixture(plan)
            jobs = responses[next(path for path in responses if "/jobs?" in path)]["jobs"]
            jobs[0]["steps"] = [step for step in jobs[0]["steps"] if step["name"] != "Run actions/setup-python@v5"]
            jobs[0]["steps"] += [{"name": "Run actions/setup-python@v5", "status": "completed", "conclusion": conclusion}
                                  for conclusion in conclusions]
            with self.subTest(conclusions=conclusions), self.assertRaisesRegex(ValueError, "Python setup"):
                self.read_receipt(plan, identity, responses, log)

    def test_receipts_reject_missing_actual_runtime_lines(self):
        plan = self.plan(mode="control-anchor")
        identity, responses, log = self.receipt_fixture(plan)
        log = "\n".join(line for line in log.splitlines() if "Image:" not in line) + "\n"
        with self.assertRaisesRegex(ValueError, "runner evidence"):
            self.read_receipt(plan, identity, responses, log)

    def test_donor_inputs_require_two_platform_anchor_but_read_only_own_platform(self):
        rows = [row(1, head=self.head), row(2, workflow=MAC, head=self.head)]
        self.write(control.INPUT, (canonical({"version": 1, "receipts": rows}) + b"\n").decode())
        head = self.commit("explicit anchor receipts", bind=False)
        with patch.object(control, "read_receipt", return_value=({"direct": True}, {})) as read:
            result = control.donor_receipts(self.root, SourceTree(self.root, head), LINUX, "unused", Mock(), Mock())
        self.assertEqual(result, [{"direct": True}])
        self.assertEqual(read.call_count, 1)
        self.assertEqual(read.call_args.args[1], rows[0])
        for invalid in (rows[:1], [rows[0], {**rows[1], "workflow": LINUX}],
                        [rows[0], {**rows[1], "head": "f" * 40}]):
            with self.subTest(rows=invalid), patch.object(control, "inputs", return_value=invalid), self.assertRaises(ValueError):
                control.donor_receipts(self.root, SourceTree(self.root, head), LINUX, "unused", Mock(), Mock())

    def test_fresh_override_does_not_bypass_failed_or_unknown_explicit_donor(self):
        plan = self.plan(mode="control-anchor")
        identity, responses, log = self.receipt_fixture(plan)
        rows = [identity, row(2, workflow=MAC, head=self.head)]
        self.write(control.INPUT, (canonical({"version": 1, "receipts": rows,
                                              "fresh_groups": sorted(GROUPS)}) + b"\n").decode())
        head = self.commit("all fresh still validates explicit donor", bind=False)
        responses[next(iter(responses))]["conclusion"] = "failure"
        for response in (responses, None):
            request = Mock(side_effect=lambda path, token: response[path] if response is not None else None)
            read_log = Mock(return_value=log)
            with self.subTest(known=response is not None), self.assertRaises(ValueError):
                control.donor_receipts(self.root, SourceTree(self.root, head), LINUX, "unused", request, read_log)
            request.assert_called()
            read_log.assert_not_called()

    def bootstrap_fixture(self):
        # Real source/main merge parent order, with synthetic execution evidence.
        anchor = self.head
        previous = self.commit("synthetic prior main", bind=False)
        tree_id = self.git("rev-parse", previous + "^{tree}").decode().strip()
        source = self.git("commit-tree", tree_id, "-p", anchor, "-p", previous,
                          "-m", "synthetic delta\n\n" + control.MARKER).decode().strip()
        main = self.git("commit-tree", tree_id, "-p", previous, "-p", source,
                        "-m", "synthetic main merge\n\n" + control.MARKER).decode().strip()
        tree = SourceTree(self.root, main)
        selected = {"mode": "control-delta", "base": previous, "head": main, "source_head": source,
                    "ref": control.REF, "event": "push"}
        evidence = {"runs": {}, "qualification": {"reviewed_source_control_sha256":
                    routing.source_hash(tree, tuple(dict.fromkeys((*control.CODE, *control.RECEIPT_CONTROLS))))}}
        plans, donors = {}, {}
        for number, (workflow, system) in enumerate(((LINUX, "Linux"), (MAC, "Darwin")), 1):
            original = self.plan(workflow=workflow, context=runtime(system), mode="control-anchor", head=anchor)
            executed = {group: original[group] for group in control.PLATFORMS[workflow][2]}
            donors[workflow] = [self.donor(original, number=number, executed=executed)]
            plans[workflow] = control.make_plan(self.root, selected, workflow, runtime(system), donors[workflow])
            evidence["runs"][workflow] = {"id": number + 10, "attempt": 1}
        return {"base": main}, evidence, tree, plans, donors

    def bootstrap(self, selected, evidence, tree, plans, donors, *, main_checks=None):
        def receipt(root, identity, token, request, log, *, main):
            self.assertIs(main, True)
            plan = plans[identity["workflow"]]
            executed = {group: plan[group] for group in control.PLATFORMS[identity["workflow"]][2]}
            return self.donor(plan, number=identity["id"], executed=executed), plan
        with (patch.object(control, "changes", return_value=tree),
              patch.object(control.proof, "historical_controls", return_value=tuple(dict.fromkeys((*control.CODE, *control.RECEIPT_CONTROLS)))),
              patch.object(control.proof, "check_main", side_effect=main_checks) as checked,
              patch.object(control, "read_receipt", side_effect=receipt),
              patch.object(control, "donor_receipts", side_effect=lambda root, tree, workflow, *args: donors[workflow])):
            result = control.bootstrap(self.root, selected, evidence, "unused", Mock(), Mock())
        self.assertEqual(checked.call_count, 2)
        return result

    def test_delta_bootstrap_requires_its_new_historical_controls(self):
        selected, evidence, tree, plans, donors = self.bootstrap_fixture()
        for missing in control.RECEIPT_CONTROLS:
            controls = tuple(p for p in dict.fromkeys((*control.CODE, *control.RECEIPT_CONTROLS)) if p != missing)
            with self.subTest(missing=missing), patch.object(control, "changes", return_value=tree), \
                    patch.object(control.proof, "historical_controls", return_value=controls), \
                    self.assertRaisesRegex(ValueError, "inventory is incomplete"):
                control.bootstrap(self.root, selected, evidence, "unused", Mock(), Mock())

    def test_anchor_cannot_declare_receipt_inputs(self):
        self.write(control.INPUT, (canonical({"version": 1, "receipts": [row()]}) + b"\n").decode())
        self.head = self.commit("invalid anchor input", bind=False)
        with self.assertRaisesRegex(ValueError, "declare inherited"):
            self.plan(mode="control-anchor")

    def test_metadata_only_ancestor_receipts_preserve_direct_coverage(self):
        anchor = self.plan(mode="control-anchor")
        first = self.donor(anchor, number=10)
        self.write("math/unrelated.hpp", "#pragma once\n// new main product source\n")
        self.head = self.commit("fresh product delta", bind=False)
        delta = self.plan([first])
        second = self.donor(delta, number=12, executed={g: delta[g] for g in GROUPS})
        self.assertTrue(delta["headers"])
        rows = [row(10, head=anchor["head"]), row(11, head=anchor["head"], workflow=MAC),
                row(12, head=delta["head"], kind="delta"), row(13, head=delta["head"], kind="delta", workflow=MAC)]
        self.write(control.INPUT, (canonical({"version": 1, "receipts": rows}) + b"\n").decode())
        self.head = self.commit("reviewed earlier direct receipt IDs only", bind=False)
        metadata = self.plan([first, second])
        self.assertTrue(all(not metadata[g] for g in GROUPS))
        self.assertEqual(metadata["checks"], delta["checks"])
        self.assertTrue(all(r["head"] != self.head for r in rows))
        for r in rows:
            self.git("merge-base", "--is-ancestor", r["head"], self.head)

    def test_actual_main_bootstrap_keeps_direct_union_distinct_from_fresh_execution(self):
        selected, evidence, tree, plans, donors = self.bootstrap_fixture()
        result = self.bootstrap(selected, evidence, tree, plans, donors)
        self.assertEqual(result["basis"], "control-delta-bootstrap")
        self.assertEqual(result["ordinary_coverage"], "direct-success-receipt-union")
        self.assertEqual(result["full_baseline_target_coverage"], "not_independently_established")
        self.assertEqual(result["intrinsic_checks"], "not_run_control_only")
        self.assertEqual(len(result["receipts"]), 2)
        for receipt in result["receipts"]:
            self.assertTrue(all(not targets for targets in receipt["executed_now"].values()))
            self.assertEqual(receipt["inherited"], plans[receipt["workflow"]]["inherited"])

    def test_actual_main_bootstrap_rejects_union_without_revalidated_direct_receipts(self):
        selected, evidence, tree, plans, donors = self.bootstrap_fixture()
        donors[LINUX] = []
        with self.assertRaisesRegex(ValueError, "union lacks direct"):
            self.bootstrap(selected, evidence, tree, plans, donors)

    def test_actual_main_bootstrap_rechecks_main_success_before_return(self):
        selected, evidence, tree, plans, donors = self.bootstrap_fixture()
        with self.assertRaisesRegex(control.proof.MainFailure, "later failure"):
            self.bootstrap(selected, evidence, tree, plans, donors,
                           main_checks=[None, control.proof.MainFailure("later failure")])


if __name__ == "__main__":
    unittest.main()
