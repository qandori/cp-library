import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
metadata = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'references': []}
for identifier in [391971, 362494, 362211]:
    with urllib.request.urlopen(f'https://v3.api.judge.yosupo.jp/submissions/{identifier}', timeout=60) as response:
        detail = json.load(response)
    source = detail['source'].encode()
    (args.output / f'{identifier}.cpp').write_bytes(source)
    metadata['references'].append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                                   'url': f'https://judge.yosupo.jp/submission/{identifier}'})
(args.output / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
