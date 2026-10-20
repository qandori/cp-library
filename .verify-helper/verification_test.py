import contextlib
import datetime
import hashlib
from importlib.util import find_spec
import json
import os
from pathlib import Path
import tempfile
import subprocess
import time
import unittest
from unittest.mock import Mock, patch

from verification import get_marker, prepare_samples


class AtCoderSamples(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.url = "https://atcoder.jp/contests/abc454/tasks/abc454_g"
        self.paths = ["test/a.test.cpp", "test/b.test.cpp"]
        for name in self.paths:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(f'#define PROBLEM "{self.url}"\n')
        self.cases = self.root / ".verify-helper/cache" / hashlib.md5(self.url.encode()).hexdigest() / "test"
        environment = patch.dict(os.environ, {}, clear=True)
        environment.start()
        self.addCleanup(environment.stop)

    def download(self, command, **kwargs):
        self.assertEqual(command[:3], ["oj", "download", "-d"])
        self.assertEqual(command[4:], ["--silent", self.url])
        self.assertEqual(kwargs, {"check": True})
        cases = Path(command[3])
        cases.mkdir()
        (cases / "sample-1.in").write_text("1\n")
        (cases / "sample-1.out").write_text("2\n")

    def test_downloads_samples_once_before_verification(self):
        self.cases.mkdir(parents=True)
        with patch("verification.subprocess.run", side_effect=self.download) as run:
            prepare_samples(self.root, self.paths)
            prepare_samples(self.root, self.paths)
        run.assert_called_once()
        self.assertEqual((self.cases / "sample-1.in").read_text(), "1\n")
        self.assertEqual((self.cases / "sample-1.out").read_text(), "2\n")

    def test_preserves_system_tests_and_other_judges(self):
        self.cases.mkdir(parents=True)
        (self.cases / "system.in").write_text("3\n")
        (self.cases / "system.out").write_text("4\n")
        with patch("verification.subprocess.run") as run:
            prepare_samples(self.root, self.paths)
            for url in ("https://judge.yosupo.jp/problem/aplusb", "https://yukicoder.me/problems/no/2362"):
                (self.root / self.paths[0]).write_text(f'#define PROBLEM "{url}"\n')
                prepare_samples(self.root, self.paths[:1])
        run.assert_not_called()
        self.assertEqual((self.cases / "system.in").read_text(), "3\n")

    def test_token_keeps_system_download_available(self):
        with patch.dict(os.environ, {"DROPBOX_TOKEN": "test-token"}), patch("verification.subprocess.run") as run:
            prepare_samples(self.root, self.paths)
        run.assert_not_called()
        self.assertFalse(self.cases.exists())

    def test_failed_download_is_not_cached(self):
        def failed(command, **kwargs):
            self.download(command, **kwargs)
            raise subprocess.CalledProcessError(1, command)
        with patch("verification.subprocess.run", side_effect=failed):
            with self.assertRaises(subprocess.CalledProcessError):
                prepare_samples(self.root, self.paths)
        self.assertFalse(self.cases.exists())
        with patch("verification.subprocess.run", side_effect=self.download):
            prepare_samples(self.root, self.paths)
        self.assertTrue((self.cases / "sample-1.out").is_file())

    def test_incomplete_samples_fail(self):
        def incomplete(command, **kwargs):
            self.download(command, **kwargs)
            (Path(command[3]) / "sample-1.out").unlink()
        with patch("verification.subprocess.run", side_effect=incomplete):
            with self.assertRaisesRegex(RuntimeError, "Incomplete AtCoder samples"):
                prepare_samples(self.root, self.paths)
        self.assertFalse(self.cases.exists())

    @unittest.skipUnless(find_spec("onlinejudge_verify"), "Requires online-judge-verify-helper")
    def test_helper_tests_prepared_samples_without_system_download(self):
        from onlinejudge_verify import verify

        with patch("verification.subprocess.run", side_effect=self.download):
            prepare_samples(self.root, self.paths)
        language, environment = Mock(), Mock()
        language.list_attributes.return_value = {"PROBLEM": self.url}
        language.list_environments.return_value = [environment]
        environment.get_execute_command.return_value = ["sample-program"]
        previous = Path.cwd()
        try:
            os.chdir(self.root)
            with patch.object(verify.onlinejudge_verify.languages.list, "get", return_value=language), \
                    patch.object(verify, "exec_command") as command:
                self.assertTrue(verify.verify_file(Path(self.paths[0]), compilers=[], tle=10, jobs=2))
            command.assert_called_once()
            self.assertEqual(command.call_args.args[0][:2], ["oj", "test"])
            self.assertNotIn("--system", command.call_args.args[0])
            environment.compile.assert_called_once()
        finally:
            os.chdir(previous)


@unittest.skipUnless(find_spec("onlinejudge_verify"), "Requires online-judge-verify-helper")
class VerificationStatus(unittest.TestCase):
    def test_parallel_reload_preserves_successes_during_lazy_language_initialization(self):
        from onlinejudge_verify import config, marker
        from onlinejudge_verify.languages import list as languages

        with tempfile.TemporaryDirectory() as directory, contextlib.ExitStack() as stack:
            root = Path(directory)
            names = ["a.test.cpp", "b.test.cpp", "c.test.cpp", "d.test.cpp"]
            for name in names:
                (root / name).write_text("int main() {}\n")
            timestamp = (datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(days=1)).strftime("%Y-%m-%d %H:%M:%S %z")
            path = root / "timestamps.json"
            path.write_text(json.dumps(dict.fromkeys(names, timestamp)))
            previous = Path.cwd()
            os.chdir(root)
            stack.callback(os.chdir, previous)

            initialize = languages.CPlusPlusLanguage

            def slow_initialization():
                time.sleep(0.05)
                return initialize()

            def load(**kwargs):
                return marker.VerificationMarker(json_path=path, use_git_timestamp=False, **kwargs)

            stack.enter_context(patch.object(config, "_loaded_config", None))
            stack.enter_context(patch.object(languages, "_dict", None))
            stack.enter_context(patch.object(languages, "CPlusPlusLanguage", slow_initialization))
            stack.enter_context(patch.object(marker, "get_verification_marker", side_effect=load))
            verification = get_marker()
            for name in names:
                self.assertTrue(verification.is_verified(Path(name)), name)


if __name__ == "__main__":
    unittest.main()
