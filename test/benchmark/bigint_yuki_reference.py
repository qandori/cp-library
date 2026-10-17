import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0 cp-library-bigint'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'yukicoder': [], 'library_checker': {}}
    for identity, integer, elapsed in [(1182839, 'uint512_t', 197), (1185228, 'cpp_int', 248)]:
        url = f'https://yukicoder.me/submissions/{identity}'
        source = get(url + '/source')
        (args.cache / f'{identity}.cpp').write_bytes(source)
        (args.cache / f'{identity}.html').write_bytes(get(url))
        result['yukicoder'].append({'id': identity, 'url': url, 'type': integer, 'judge_ms': elapsed,
                                    'source_sha256': hashlib.sha256(source).hexdigest()})
    for problem in ['addition_of_big_integers', 'multiplication_of_big_integers', 'division_of_big_integers', 'shortest_path']:
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 10})
        listing = json.loads(get('https://v3.api.judge.yosupo.jp/submissions?' + query))
        selected = listing['submissions'][:1] + [item for item in listing['submissions'] if item['lang'].startswith('cpp')][:2]
        references = []
        for identity in dict.fromkeys(item['id'] for item in selected):
            detail = json.loads(get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}'))
            source = detail['source'].encode()
            (args.cache / f'lc-{identity}.txt').write_bytes(source)
            references.append({**detail['overview'], 'url': f'https://judge.yosupo.jp/submission/{identity}',
                               'source_sha256': hashlib.sha256(source).hexdigest()})
        result['library_checker'][problem] = {'listing': listing, 'references': references}
        print(problem, [(item['id'], item['time'], item['lang']) for item in references], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
