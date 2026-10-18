import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def get(url):
    with urllib.request.urlopen(urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'}), timeout=90) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    api = 'https://v3.api.judge.yosupo.jp'
    query = urllib.parse.urlencode(dict(problem='dynamic_graph_vertex_add_component_sum', status='AC', order='+time', limit=100))
    listing = get(api + '/submissions?' + query)
    ids = [row['id'] for row in listing['submissions'][:8]]
    ids += [289400, 214504, 390701, 377433, 147929, 213416, 320835]

    def fetch(identity):
        detail = get(api + '/submissions/' + str(identity))
        source = detail['source'].encode()
        (args.cache / (str(identity) + '.cpp')).write_bytes(source)
        return dict(**detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(), url='https://judge.yosupo.jp/submission/' + str(identity))

    references = list(ThreadPoolExecutor(max_workers=4).map(fetch, dict.fromkeys(ids)))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(dict(retrieved_at=datetime.now(timezone.utc).isoformat(), listing=listing, references=references), indent=2) + '\n')
    for row in references:
        print(row['id'], row.get('user_name', '(Anonymous)'), row['time'])


if __name__ == '__main__':
    main()
