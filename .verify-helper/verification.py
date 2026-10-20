import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
from urllib.parse import urlparse


def prepare_samples(root, paths):
    from judge_data import urls_for

    if os.environ.get("DROPBOX_TOKEN"):
        return
    for url in urls_for(root, {"verification": paths}):
        if urlparse(url).hostname != "atcoder.jp":
            continue
        directory = root / ".verify-helper/cache" / hashlib.md5(url.encode()).hexdigest() / "test"
        if directory.exists() and any(directory.iterdir()):
            continue
        directory.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=directory.parent) as temporary:
            cases = Path(temporary) / "test"
            subprocess.run(["oj", "download", "-d", str(cases), "--silent", url], check=True)
            inputs = list(cases.glob("*.in"))
            if not inputs or any(not path.with_suffix(".out").is_file() for path in inputs):
                raise RuntimeError("Incomplete AtCoder samples: " + url)
            if directory.exists():
                directory.rmdir()
            cases.rename(directory)


def get_marker():
    from onlinejudge_verify import marker
    from onlinejudge_verify.languages import list as languages

    languages.get(Path("source.cpp"))
    return marker.get_verification_marker(jobs=2)
