import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


def download(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-online-convolution'})
    with urllib.request.urlopen(request, timeout=60) as response:
        return response.read()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    result = {'retrieved_at': datetime.now(timezone.utc).isoformat(), 'problems': {}, 'libraries': []}
    for problem in ('convolution_mod', 'exp_of_formal_power_series'):
        query = urllib.parse.urlencode({'problem': problem, 'status': 'AC', 'order': '+time', 'limit': 5})
        listing = json.loads(download('https://v3.api.judge.yosupo.jp/submissions?' + query))
        references = []
        baseline = 409465 if problem == 'convolution_mod' else 409662
        identities = list(dict.fromkeys([item['id'] for item in listing['submissions'][:3]] + [baseline]))
        for identity in identities:
            detail = json.loads(download(f'https://v3.api.judge.yosupo.jp/submissions/{identity}'))
            source = detail['source'].encode()
            (args.cache / f'{identity}.cpp').write_bytes(source)
            references.append({**detail['overview'], 'source_sha256': hashlib.sha256(source).hexdigest(),
                               'url': f'https://judge.yosupo.jp/submission/{identity}'})
            print(problem, identity, detail['overview'].get('time'), flush=True)
        result['problems'][problem] = references
    for repository, paths in {
        'hitonanode/cplib-cpp': ['convolution/relaxed_multiplication.hpp', 'convolution/ntt.hpp', 'modint.hpp'],
        'NyaanNyaan/library': ['ntt/relaxed-convolution.hpp'],
    }.items():
        commit = json.loads(download(f'https://api.github.com/repos/{repository}/commits/master'))['sha']
        for path in paths:
            url = f'https://raw.githubusercontent.com/{repository}/{commit}/{path}'
            source = download(url)
            destination = args.cache / repository.split('/')[0] / path
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(source)
            result['libraries'].append({'repository': repository, 'commit': commit, 'path': path,
                                        'url': url, 'source_sha256': hashlib.sha256(source).hexdigest()})
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
