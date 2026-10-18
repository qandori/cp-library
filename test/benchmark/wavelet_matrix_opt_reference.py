import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


PROBLEMS = ('range_kth_smallest', 'static_range_frequency', 'static_range_sum_with_upper_bound')
API = 'https://v3.api.judge.yosupo.jp'
EXTRA = (303545, 158109, 208756, 332152, 117672, 393938, 401531, 258498, 205664)


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-benchmark'})
    with urllib.request.urlopen(request, timeout=90) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--top', type=int, default=10)
    parser.add_argument('--extra', type=int, nargs='*', default=list(EXTRA))
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    retrieved_at = datetime.now(timezone.utc).isoformat()
    listings = {}
    identities = list(args.extra)
    for problem in PROBLEMS:
        query = urllib.parse.urlencode(dict(problem=problem, status='AC', order='+time', limit=100))
        listings[problem] = get(API + '/submissions?' + query)
        identities.extend(row['id'] for row in listings[problem]['submissions'][:args.top])

    def fetch(identity):
        detail = get(API + '/submissions/' + str(identity))
        source = detail['source'].encode()
        (args.cache / (str(identity) + '.cpp')).write_bytes(source)
        row = dict(**detail['overview'], source_sha256=hashlib.sha256(source).hexdigest(),
                    source_bytes=len(source), url='https://judge.yosupo.jp/submission/' + str(identity),
                    api_url=API + '/submissions/' + str(identity))
        if identity == 402999:
            core = detail['source'].split('// ===== include/canard/kernel/wavelet_rank.hpp =====\n', 1)[1]
            core = core.rsplit('\n#include <cstdint>\n#include <ranges>\n#include <vector>\n\nint main()', 1)[0]
            prefix = '#define CANARD_ALWAYS_INLINE [[gnu::always_inline]]\nnamespace canard::execution { struct scalar {}; struct avx2 {}; }\n'
            core = (prefix + core + '\n#undef CANARD_ALWAYS_INLINE\n').encode()
            (args.cache / '402999_core.hpp').write_bytes(core)
            row['kernel_sha256'] = hashlib.sha256(core).hexdigest()
            row['kernel_file'] = '402999_core.hpp'
            row['kernel_extraction'] = 'Unchanged layout, scalar/AVX2 kernel, radix encoder and wavelet_matrix definitions; execution tags and inline macro supplied.'
        return row

    with ThreadPoolExecutor(max_workers=4) as executor:
        references = list(executor.map(fetch, dict.fromkeys(identities)))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    document = dict(retrieved_at=retrieved_at, completed_at=datetime.now(timezone.utc).isoformat(),
                    api=API, order='+time', listings=listings, references=references)
    args.output.write_text(json.dumps(document, indent=2) + '\n', encoding='utf-8')
    for row in references:
        print(row['problem_name'], row['id'], row.get('user_name', '(Anonymous)'), row['time'])


if __name__ == '__main__':
    main()
