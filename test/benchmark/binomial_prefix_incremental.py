import argparse
import hashlib
import html
import json
import os
from pathlib import Path
import random
import re
import statistics
import subprocess
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / '.verify-helper/cache/arc190-b'
OUT = ROOT / 'test/benchmark/results/binomial_prefix_incremental'
BASE = '555b183'


def run(command, **kwargs):
    return subprocess.run(command, cwd=ROOT, check=True, **kwargs)


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def oracle(n, a, b, ks):
    from math import comb
    def cumulative(k):
        if k == 0:
            return 0
        row = n - k
        x = sum(comb(row, i) for i in range(max(0, a - k), min(a - 1, row) + 1))
        y = sum(comb(row, i) for i in range(max(0, b - k), min(b - 1, row) + 1))
        return x * y * pow(4, k - 1, 998244353) % 998244353
    return [(cumulative(k) - cumulative(k - 1)) % 998244353 for k in ks]


def data(n, a, b, ks):
    return (f'{n} {a} {b}\n{len(ks)}\n' + ' '.join(map(str, ks)) + '\n').encode()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--quick', action='store_true')
    parser.add_argument('--repeats', type=int, default=7)
    args = parser.parse_args()
    CACHE.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    for identifier in ['80215048', '61518705']:
        path = CACHE / (identifier + '.cpp')
        if not path.exists():
            page = urllib.request.urlopen('https://atcoder.jp/contests/arc190/submissions/' + identifier).read().decode()
            code = re.search(r'<pre[^>]*id="submission-code"[^>]*>(.*?)</pre>', page, re.S)[1]
            path.write_text(html.unescape(code))
    for name in ['components/binomial_prefix_core.hpp', 'components/fps_factorials.hpp', 'math/mint.hpp']:
        original_component = subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=ROOT)
        assert original_component.replace(b'\r\n', b'\n') == (ROOT / name).read_bytes().replace(b'\r\n', b'\n'), name
    old = subprocess.check_output(['git', 'show', BASE + ':math/combinatorics/binomial_prefix_sum_incremental.hpp'], cwd=ROOT, text=True)
    old = old.replace('BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP', 'BASELINE_BINOMIAL_PREFIX_SUM_INCREMENTAL_HPP')
    old = old.replace('IncrementalBinomialPrefixSum', 'BaselineIncrementalBinomialPrefixSum').replace('../../components/', 'components/')
    (CACHE / 'baseline.hpp').write_text(old)
    original = (CACHE / '80215048.cpp').read_text().replace('#include <atcoder/all>', '')
    (CACHE / 'submitted.cpp').write_text(original)
    report = {'base': BASE, 'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': list(os.uname()), 'cpu': subprocess.check_output(['lscpu'], text=True),
              'cpu_pin': 2, 'seed': 20261020, 'repeats': args.repeats,
              'original_source_sha256': sha(CACHE / '80215048.cpp'), 'micro': [], 'solution': []}
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-I', str(ROOT)]
    configurations = {'generic': [], 'native': ['-march=native', '-ftrivial-auto-var-init=zero']}
    if args.quick:
        configurations = {'native': configurations['native']}
    randomizer = random.Random(20261020)
    samples = [(3, 1, 2, [2], [6]), (5, 2, 5, [1, 3, 5], [4, 32, 128]),
               (100, 50, 50, [1, 10, 50, 100], [934228871, 758172260, 444239843, 0])]
    for name, extra in configurations.items():
        binaries = {}
        sources = {'micro': ROOT / 'test/benchmark/binomial_prefix_incremental.cpp',
                   'submitted': CACHE / 'submitted.cpp', 'sweeps': ROOT / 'test/benchmark/arc190_b_sweeps.cpp',
                   'cdf': ROOT / 'test/benchmark/binomial_prefix_incremental_arc190_b.cpp',
                   'official': CACHE / '61518705.cpp'}
        for mode, source in sources.items():
            binary = CACHE / (mode + '-' + name)
            run(['g++', *flags, *extra, str(source), '-o', str(binary)])
            binaries[mode] = str(binary)
        for shape in ['rows', 'diagonal', 'columns', 'walk', 'jumps', 'arc190']:
            n = 10000 if shape == 'jumps' else 1000000
            values = {mode: [] for mode in ['baseline', 'current', 'shared']}
            checksum = None
            for trial in range(-1, args.repeats):
                modes = list(values)
                randomizer.shuffle(modes)
                for mode in modes:
                    parts = subprocess.check_output(['taskset', '-c', '2', binaries['micro'], mode, shape, str(n)], text=True).split()
                    if checksum is None:
                        checksum = parts[2]
                    assert checksum == parts[2], (shape, mode, parts)
                    if trial >= 0:
                        values[mode].append({'build_ms': float(parts[0]), 'query_ms': float(parts[1]), 'peak_kib': int(parts[3])})
            report['micro'].append({'configuration': name, 'flags': flags + extra, 'shape': shape, 'n': n, 'values': values})
            print(name, shape, {m: round(statistics.median(x['query_ms'] for x in v), 3) for m, v in values.items()}, flush=True)
        cases = [(f'sample{i}', n, a, b, ks, expected) for i, (n, a, b, ks, expected) in enumerate(samples, 1)]
        if not args.quick:
            n = 10000000
            ks = sorted(randomizer.sample(range(1, n + 1), 200000))
            cases += [('large_center', n, n // 2, n // 2 + 1, ks, None), ('large_corner', n, 1, n, ks, None)]
        for label, n, a, b, ks, expected in cases:
            payload = data(n, a, b, ks)
            input_path = CACHE / (name + '-' + label + '.in')
            input_path.write_bytes(payload)
            values = {mode: [] for mode in ['submitted', 'sweeps', 'cdf', 'official']}
            reference = expected
            for trial in range(-1, args.repeats):
                modes = list(values)
                randomizer.shuffle(modes)
                for mode in modes:
                    memory = CACHE / 'memory.txt'
                    with input_path.open('rb') as stream:
                        start = time.perf_counter()
                        result = subprocess.check_output(['/usr/bin/time', '-f', '%M', '-o', str(memory), 'taskset', '-c', '2', binaries[mode]], stdin=stream)
                        elapsed = (time.perf_counter() - start) * 1000
                    actual = list(map(int, result.split()))
                    if reference is None:
                        reference = actual
                    assert actual == reference, (label, mode)
                    if trial >= 0:
                        values[mode].append({'wall_ms': elapsed, 'peak_kib': int(memory.read_text())})
            report['solution'].append({'configuration': name, 'case': label, 'n': n, 'a': a, 'b': b, 'q': len(ks),
                                       'input_sha256': hashlib.sha256(payload).hexdigest(), 'values': values})
            print(name, label, {m: round(statistics.median(x['wall_ms'] for x in v), 3) for m, v in values.items()}, flush=True)
        (OUT / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')
    sources = ['math/combinatorics/binomial_prefix_sum_incremental.hpp', 'components/binomial_prefix_core.hpp',
               'components/fps_factorials.hpp', 'math/mint.hpp', 'test/unit/binomial_prefix_sum_incremental.cpp',
               'test/benchmark/binomial_prefix_incremental.cpp', 'test/benchmark/binomial_prefix_incremental.py',
               'test/benchmark/binomial_prefix_incremental_arc190_b.cpp', 'test/benchmark/arc190_b_sweeps.cpp']
    report['sources'] = {name: sha(ROOT / name) for name in sources}
    (OUT / 'measurements.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
