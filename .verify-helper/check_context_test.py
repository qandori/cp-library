import contextlib
import copy
import io
import json
import os
import subprocess
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

import ci
from affected import GROUPS, canonical, digest
from check_context import AMBIENT_KEYS, PROFILE, VERSION, collect_runtime, installed_boost, runtime_context


BOOST = {"libboost-dev": "1.83.0.1ubuntu2", "libboost1.83-dev": "1.83.0-2.1ubuntu3"}


class RuntimeContext(unittest.TestCase):
    def collect(self, *, environment=None, compiler="compiler 1", image="image 1",
                system="Linux", machine="x86_64", python="Python 3.12", package="5.6.0", boost=None):
        environment = {"ImageVersion": image, **(environment or {})}
        distribution = SimpleNamespace(metadata={"Name": "online-judge-verify-helper"}, version=package)
        boost = boost or BOOST
        query = Mock(side_effect=["installed\t" + boost["libboost-dev"] + "\tlibboost1.83-dev\n",
                                  "installed\t" + boost["libboost1.83-dev"] + "\tlibc6-dev\n"])
        with (patch.dict(os.environ, environment, clear=True),
              patch.object(ci.platform, "system", return_value=system),
              patch.object(ci.platform, "machine", return_value=machine),
              patch.object(ci.subprocess, "check_output", return_value=compiler),
              patch.object(ci.sys, "version", python),
              patch.object(ci.importlib.metadata, "distributions", return_value=[distribution])):
            result = collect_runtime(package_query=query)
        self.assertEqual(query.call_count, 2 if system == "Linux" else 0)
        return result

    def changed_groups(self, before, after):
        return {group for group in GROUPS if before["groups"][group] != after["groups"][group]}

    def test_profile_and_deterministic_canonical_hashes(self):
        keys = {group: '{"platform": ["Darwin", "arm64", "image"], "a": {"y": 2}}' for group in GROUPS}
        original = copy.deepcopy(keys)
        result = runtime_context(keys, {}, system_packages={})
        alternate = {group: '{"a":{"y":2},"platform":["Darwin","arm64","image"]}' for group in reversed(GROUPS)}
        self.assertEqual(result, runtime_context(alternate, {}, system_packages={}))
        self.assertEqual(keys, original)
        self.assertEqual(result["version"], VERSION)
        self.assertEqual(result["profile"], PROFILE)
        self.assertEqual(set(result["groups"]), set(GROUPS))
        self.assertEqual(result["groups"]["unit"]["runtime_sha256"],
                         digest(canonical({"environment_key": json.loads(keys["unit"]), "system_packages": {}})))

    def test_unset_empty_and_set_ambient_values_are_distinct(self):
        unset = self.collect()
        empty = self.collect(environment={key: "" for key in AMBIENT_KEYS})
        populated = self.collect(environment={key: "private-value" for key in AMBIENT_KEYS})
        for key in AMBIENT_KEYS:
            self.assertEqual(unset["ambient"][key], {"present": False, "sha256": None})
            self.assertEqual(empty["ambient"][key], {"present": True, "sha256": digest(b"")})
            self.assertEqual(populated["ambient"][key],
                             {"present": True, "sha256": digest(b"private-value")})
        self.assertEqual(self.changed_groups(unset, empty), set(GROUPS))
        self.assertEqual(self.changed_groups(empty, populated), set(GROUPS))

    def test_each_ambient_key_invalidates_all_groups(self):
        before = self.collect()
        for key in AMBIENT_KEYS:
            with self.subTest(key=key):
                after = self.collect(environment={key: "changed"})
                self.assertEqual(self.changed_groups(before, after), set(GROUPS))

    def test_compiler_image_platform_changes_invalidate_all_groups(self):
        before = self.collect()
        for change in ({"compiler": "compiler 2"}, {"image": "image 2"},
                       {"system": "Darwin"}, {"machine": "arm64"}):
            with self.subTest(change=change):
                self.assertEqual(self.changed_groups(before, self.collect(**change)), set(GROUPS))

    def test_python_and_packages_only_invalidate_judge_and_bundle_groups(self):
        before = self.collect()
        for change in ({"package": "5.7.0"}, {"python": "Python 3.13"}):
            with self.subTest(change=change):
                self.assertEqual(self.changed_groups(before, self.collect(**change)),
                                 {"verification", "bundles"})

    def test_installed_boost_packages_invalidate_all_groups(self):
        before = self.collect()
        for name in BOOST:
            with self.subTest(name=name):
                after = self.collect(boost={**BOOST, name: "2.0"})
                self.assertEqual(self.changed_groups(before, after), set(GROUPS))
                self.assertNotEqual(before["system_packages_sha256"], after["system_packages_sha256"])

    def test_boost_query_resolves_installed_dependency(self):
        query = Mock(side_effect=["installed\t1.83.0.1ubuntu2\tlibboost1.83-dev (>= 1.83.0)\n",
                                  "installed\t1.83.0-2.1ubuntu3\tlibc6-dev\n"])
        self.assertEqual(installed_boost(query), BOOST)
        self.assertEqual([call.args[0][-1] for call in query.call_args_list], ["libboost-dev", "libboost1.83-dev"])

    def test_boost_unknown_uninstalled_or_ambiguous_fails_closed(self):
        for result in ("", "config-files\t1.83\tlibboost1.83-dev\n", "installed\t1.83\tunknown\n",
                       "installed\t1.83\tlibboost1.83-dev | libboost1.84-dev\n"):
            with self.subTest(result=result), self.assertRaises(ValueError):
                installed_boost(Mock(return_value=result))
        for error in (FileNotFoundError(), subprocess.CalledProcessError(1, "dpkg-query")):
            with self.subTest(error=error), self.assertRaises(ValueError):
                installed_boost(Mock(side_effect=error))
        with self.assertRaises(ValueError):
            installed_boost(Mock(side_effect=["installed\t1.83\tlibboost1.83-dev\n", "not-installed\t\t\n"]))

    def test_raw_context_values_are_not_returned_or_logged(self):
        output = io.StringIO()
        private = "private-context-value"
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            result = self.collect(environment={key: private for key in AMBIENT_KEYS},
                                  compiler=private, image=private, python=private, package=private)
        self.assertEqual(output.getvalue(), "")
        self.assertNotIn(private, json.dumps(result))
        self.assertEqual(set(result["ambient"]), set(AMBIENT_KEYS))

    def test_unrelated_environment_does_not_invalidate_context(self):
        self.assertEqual(self.collect(), self.collect(environment={"UNRELATED_SECRET": "private"}))

    def test_missing_or_invalid_runtime_values_fail_without_echoing_data(self):
        keys = {group: "{}" for group in GROUPS}
        invalid = [dict(keys, unit="private-bad-json"), dict(keys, unit="null"),
                   dict(keys, unit={}), {group: keys[group] for group in GROUPS if group != "unit"}]
        for value in invalid:
            with self.subTest(value=value), self.assertRaises(ValueError) as caught:
                runtime_context(value, {}, system_packages={})
            self.assertNotIn("private-bad-json", str(caught.exception))

    def test_unknown_image_or_architecture_is_not_a_complete_profile(self):
        for machine, image in (("x86_64", None), ("x86_64", ""), (None, "image"), ("", "image")):
            keys = {g: json.dumps({"platform": ["Linux", machine, image]}) for g in GROUPS}
            with self.subTest(machine=machine, image=image), self.assertRaises(ValueError):
                runtime_context(keys, {}, system_packages=BOOST)

    def test_missing_boost_or_unsupported_platform_fails_closed(self):
        for system, packages in (("Linux", {}), ("Darwin", BOOST), ("Windows", {})):
            keys = {group: json.dumps({"platform": [system, "x86_64", "image"]}) for group in GROUPS}
            with self.subTest(system=system), self.assertRaises(ValueError):
                runtime_context(keys, {}, system_packages=packages)


if __name__ == "__main__":
    unittest.main()
