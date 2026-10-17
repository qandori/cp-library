import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'
    query = urllib.parse.urlencode({'problem': 'many_aplusb', 'status': 'AC', 'order': '+time', 'limit': 8})
    listing = get(api + '/submissions?' + query)
    metadata = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': []}
    identities = list(dict.fromkeys([item['id'] for item in listing['submissions']] + [193613, 156493]))
    for identity in identities:
        detail = get(api + f'/submissions/{identity}')
        (args.cache / f'{identity}.json').write_text(json.dumps(detail, indent=2) + '\n')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        metadata['references'].append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                                       'url': f'https://judge.yosupo.jp/submission/{identity}'})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(metadata, indent=2) + '\n')
    print(json.dumps(metadata, indent=2))


if __name__ == '__main__':
    main()
