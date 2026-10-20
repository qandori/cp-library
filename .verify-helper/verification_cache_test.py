import copy
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import yaml

from ci import GROUPS, build_plan, environment_key, workflow_checks


ROOT = Path(__file__).resolve().parent.parent


class VerificationCacheTests(unittest.TestCase):
    def setUp(self):
        self.workflow = yaml.safe_load((ROOT / ".github/workflows/verify.yml").read_text())
        self.job = self.workflow["jobs"]["verify"]
        self.steps = self.job["steps"]
        self.image = next(step for step in self.steps if step.get("id") == "runner-image")
        self.cache = next(step for step in self.steps if step.get("id") == "verification-cache")

    def identify(self, version):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "output"
            output.write_text("existing=value\n")
            environment = os.environ.copy()
            environment.pop("ImageVersion", None)
            environment["GITHUB_OUTPUT"] = str(output)
            if version is not None:
                environment["ImageVersion"] = version
            result = subprocess.run(["bash", "-e", "-c", self.image["run"]],
                                    env=environment, text=True, capture_output=True, timeout=10)
            return result, output.read_text()

    def render(self, text, version, ref="refs/heads/topic"):
        values = {"runner.os": "Linux", "runner.arch": "X64",
                  "steps.runner-image.outputs.version": version, "github.ref": ref,
                  "github.run_id": "123", "github.run_attempt": "1"}
        for name, value in values.items():
            text = text.replace("${{ " + name + " }}", value)
        self.assertNotIn("${{", text)
        return text

    def test_reads_actual_process_image_and_appends_safe_output(self):
        identities = set()
        for version in ("20261012.334.1", "20261018.339.1", "ubuntu24-2026w43-release",
                        "版本-2026", "20261012.334.1 "):
            with self.subTest(version=version):
                result, output = self.identify(version)
                self.assertEqual(result.returncode, 0, result.stderr)
                identity = hashlib.sha256(os.fsencode(version)).hexdigest()
                self.assertEqual(output, f"existing=value\nversion={identity}\n")
                self.assertRegex(identity, r"^[0-9a-f]{64}$")
                self.assertNotIn(identity, identities)
                identities.add(identity)
                self.assertEqual(self.identify(version)[1], output)
        self.assertNotIn("env", self.image)
        self.assertNotIn("${{", self.image["run"])

    def test_missing_or_blank_image_fails_without_output(self):
        for version in (None, "", " ", "\r\n\t", "\u2003"):
            with self.subTest(version=version):
                result, output = self.identify(version)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Missing runner ImageVersion", result.stderr)
                self.assertEqual(output, "existing=value\n")

    def test_image_cannot_inject_output(self):
        for version in ("20261012.334.1\nother=value", "version\r\nother=value",
                        "../image", "${{ github.ref }}", "$(touch injected); `id`"):
            with self.subTest(version=version):
                result, output = self.identify(version)
                self.assertEqual(result.returncode, 0, result.stderr)
                identity = hashlib.sha256(os.fsencode(version)).hexdigest()
                self.assertEqual(output, f"existing=value\nversion={identity}\n")
                self.assertEqual(len(output.splitlines()), 2)

    def test_keys_partition_images_before_legacy_fallback(self):
        keys = self.cache["with"]
        first, second = [hashlib.sha256(version.encode()).hexdigest()
                         for version in ("20261012.334.1", "20261018.339.1")]
        expected = [
            f"Linux-X64-verified-v2-image-{first}-refs/heads/topic-",
            f"Linux-X64-verified-v2-image-{first}-refs/heads/main-",
            "Linux-X64-verified-v2-refs/heads/topic-",
            "Linux-X64-verified-v2-refs/heads/main-",
        ]
        prefixes = self.render(keys["restore-keys"], first).splitlines()
        self.assertEqual(prefixes, expected)
        first_key = self.render(keys["key"], first)
        second_key = self.render(keys["key"], second)
        self.assertEqual(first_key, expected[0] + "123-1")
        self.assertNotEqual(first_key, second_key)
        self.assertFalse(any(second_key.startswith(prefix) for prefix in prefixes))
        main_key = self.render(keys["key"], first, "refs/heads/main")
        self.assertTrue(main_key.startswith(prefixes[1]))
        self.assertEqual(keys["path"], ".verify-helper/verified-state.json")

    def test_cache_gates_and_save_key_are_unchanged(self):
        self.assertLess(self.steps.index(self.image), self.steps.index(self.cache))
        for step in (self.image, self.cache):
            self.assertEqual(step["if"], "steps.route.outputs.mode == 'normal'")
        save = next(step for step in self.steps if step.get("uses") == "actions/cache/save@v4")
        self.assertEqual(save["if"],
                         "${{ always() && steps.route.outputs.mode == 'normal' && steps.selection.outcome == 'success' }}")
        self.assertEqual(save["with"], {"path": ".verify-helper/verified-state.json",
                                       "key": "${{ steps.verification-cache.outputs.cache-primary-key }}"})
        self.assertFalse(any(step.get("continue-on-error") for step in self.steps))

    def test_timeouts_and_website_checks(self):
        self.assertEqual(self.job["timeout-minutes"], 90)
        site = next(step for step in self.steps if step.get("name") == "Build library website")
        self.assertEqual(site, {"name": "Build library website", "timeout-minutes": 10,
                                "uses": "actions/jekyll-build-pages@v1",
                                "with": {"source": ".verify-helper/markdown",
                                         "destination": ".verify-helper/site"}})
        self.assertEqual(self.workflow["jobs"]["intrinsic"]["timeout-minutes"], 20)
        self.assertEqual(self.workflow["permissions"], {"contents": "read", "actions": "read"})
        self.assertEqual(self.workflow["jobs"]["deploy"]["needs"], "verify")

    def test_image_remains_in_all_target_fingerprints(self):
        with patch("ci.subprocess.check_output", return_value="compiler"), \
                patch("ci.importlib.metadata.distributions", return_value=[]):
            with patch.dict(os.environ, {"ImageVersion": "20261012.334.1"}):
                first = environment_key()
            with patch.dict(os.environ, {"ImageVersion": "20261018.339.1"}):
                second = environment_key()
        for group in GROUPS:
            self.assertEqual(json.loads(first[group]).get("toolchain", json.loads(first[group]))["platform"][2],
                             "20261012.334.1")
            self.assertNotEqual(first[group], second[group])

    def test_cache_controls_do_not_change_target_fingerprints(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, content in {
                "math/example.hpp": "#pragma once\n",
                "test/unit/example.cpp": '#include "math/example.hpp"\n',
                "test/example.test.cpp": '#include "math/example.hpp"\n',
            }.items():
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(content)
            path = root / ".github/workflows/verify.yml"
            path.parent.mkdir(parents=True)
            path.write_text(yaml.safe_dump(self.workflow))
            original = build_plan(root, environment="same tools")
            checks = workflow_checks(root)
            legacy = copy.deepcopy(self.workflow)
            job = legacy["jobs"]["verify"]
            job["timeout-minutes"] = 60
            job["steps"] = [step for step in job["steps"] if step.get("id") != "runner-image"]
            for step in job["steps"]:
                if step.get("id") == "verification-cache":
                    step["with"]["key"] = "${{ runner.os }}-${{ runner.arch }}-verified-v2-${{ github.ref }}-${{ github.run_id }}-${{ github.run_attempt }}"
                    step["with"]["restore-keys"] = "\n".join(step["with"]["restore-keys"].splitlines()[2:]) + "\n"
                if step.get("name") == "Build library website":
                    del step["timeout-minutes"]
            path.write_text(yaml.safe_dump(legacy))
            self.assertEqual(workflow_checks(root), checks)
            restored = build_plan(root, original["state"], environment="same tools")
            self.assertEqual(restored["state"], original["state"])
            self.assertEqual(restored["state"]["version"], 2)
            for group in GROUPS:
                self.assertTrue(original["state"]["targets"][group])
                self.assertEqual(restored[group], [])


if __name__ == "__main__":
    unittest.main()
