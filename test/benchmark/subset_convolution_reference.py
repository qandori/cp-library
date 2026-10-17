import argparse
import hashlib
import json
from pathlib import Path
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument('directory', type=Path)
parser.add_argument('--metadata', type=Path, required=True)
args = parser.parse_args()
args.directory.mkdir(parents=True, exist_ok=True)
metadata = []
for identifier in [406006, 352953]:
    url = f'https://v3.api.judge.yosupo.jp/submissions/{identifier}'
    with urllib.request.urlopen(url, timeout=60) as response:
        submission = json.load(response)
    if submission['overview']['status'] != 'AC':
        raise RuntimeError(f'{identifier} is no longer AC')
    source = submission['source']
    (args.directory / f'{identifier}.cpp').write_bytes(source.encode())
    metadata.append({'overview': submission['overview'],
                     'sha256': hashlib.sha256(source.encode()).hexdigest(),
                     'url': f'https://judge.yosupo.jp/submission/{identifier}'})
args.metadata.parent.mkdir(parents=True, exist_ok=True)
args.metadata.write_text(json.dumps(metadata, indent=2) + '\n')
