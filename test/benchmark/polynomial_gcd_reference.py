import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': 'cp-library-polynomial-gcd-audit'}), timeout=60) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problems': {}}
    for problem in ('gcd_of_polynomials', 'inv_of_polynomials'):
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 50})
        listing = get('https://v3.api.judge.yosupo.jp/submissions?' + query)
        references = []
        selected = listing['submissions'][:4]
        selected += [item for item in listing['submissions'] if item['lang'].startswith('cpp')][:3]
        for item in {item['id']: item for item in selected}.values():
            identity = item['id']
            cached = args.cache / f'{identity}.json'
            detail = json.loads(cached.read_text()) if cached.exists() else get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}')
            cached.write_text(json.dumps(detail) + '\n')
            source = detail['source'].encode()
            extension = 'cpp' if detail['overview']['lang'].startswith('cpp') else 'rs'
            (args.cache / f'{identity}.{extension}').write_bytes(source)
            references.append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(), 'url': f'https://judge.yosupo.jp/submission/{identity}'})
        result['problems'][problem] = {'listing': listing, 'references': references}
        print(problem, [(r.get('id'), r.get('time'), r.get('lang')) for r in references], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
