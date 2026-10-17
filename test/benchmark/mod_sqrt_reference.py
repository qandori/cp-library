import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-mod-sqrt'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode({'problem': 'sqrt_mod', 'status': 'AC', 'order': '+time', 'limit': 20})
    listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
    selected = listing['submissions'][:1] + [item for item in listing['submissions'] if item['lang'].startswith('cpp')][:3]
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'listing': listing, 'references': []}
    for identity in dict.fromkeys([item['id'] for item in selected] + [215966, 331256]):
        detail = get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        result['references'].append({**detail['overview'], 'url': f'https://judge.yosupo.jp/submission/{identity}',
                                    'source_sha256': hashlib.sha256(source).hexdigest()})
        print(identity, detail['overview']['lang'], detail['overview']['time'], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
