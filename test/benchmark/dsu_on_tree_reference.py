import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': 'cp-library-dsu-on-tree'}), timeout=60) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode(dict(problem='vertex_add_subtree_sum', status='AC', order='+time', limit=10))
    listing = json.loads(get('https://v3.api.judge.yosupo.jp/submissions?' + query))
    record = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing=listing, references=[])
    for entry in listing['submissions'][:2]:
        detail = json.loads(get('https://v3.api.judge.yosupo.jp/submissions/' + str(entry['id'])))
        source = detail['source'].encode()
        (args.cache / (str(entry['id']) + '.cpp')).write_bytes(source)
        record['references'].append(dict(overview=detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(),
                                         url='https://judge.yosupo.jp/submission/' + str(entry['id'])))
    commits = json.loads(get('https://api.github.com/repos/NyaanNyaan/library/commits?path=tree/dsu-on-tree.hpp&per_page=1'))
    pin = commits[0]['sha']
    url = f'https://raw.githubusercontent.com/NyaanNyaan/library/{pin}/tree/dsu-on-tree.hpp'
    source = get(url)
    (args.cache / 'nyaan-dsu-on-tree.hpp').write_bytes(source)
    record['dsu_reference'] = dict(url=url, commit=pin, source_sha256=hashlib.sha256(source).hexdigest())
    record['editorial'] = 'https://atcoder.jp/contests/abc454/editorial/19112'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(record, indent=2) + '\n')
    print([(r['overview']['id'], r['overview']['time']) for r in record['references']])


if __name__ == '__main__':
    main()
