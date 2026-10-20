from collections import Counter
import hashlib
import itertools
import json
from pathlib import Path
import random
import subprocess
import tempfile

from binomial_prefix_incremental import ROOT, OUT, data, oracle


def brute(n, a, b):
    answer = Counter()
    for choices in itertools.product(range(4), repeat=n - 1):
        u = v = 1
        level = 1
        for length, choice in zip(range(n, 1, -1), choices):
            nu = u + (choice & 1)
            nv = v + (choice >> 1)
            if u <= a < u + length and v <= b < v + length and not (nu <= a < nu + length - 1 and nv <= b < nv + length - 1):
                level = length
            u, v = nu, nv
        answer[level] += 1
    return [answer[k] for k in range(1, n + 1)]


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    cases = []
    for n in range(1, 7):
        for a in range(1, n + 1):
            for b in range(1, n + 1):
                ks = list(range(1, n + 1))
                expected = brute(n, a, b)
                assert expected == oracle(n, a, b, ks)
                cases.append((data(n, a, b, ks), expected))
    randomizer = random.Random(20261020)
    for _ in range(100):
        n = randomizer.randrange(1, 501)
        a, b = randomizer.randrange(1, n + 1), randomizer.randrange(1, n + 1)
        ks = sorted(randomizer.sample(range(1, n + 1), min(n, randomizer.randrange(1, 31))))
        cases.append((data(n, a, b, ks), oracle(n, a, b, ks)))
    flags = ['-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT)]
    checks = []
    with tempfile.TemporaryDirectory(prefix='binomial-incremental-') as directory:
        binary = str(Path(directory) / 'check')
        for compiler in ['g++', 'clang++']:
            for source in ['test/benchmark/binomial_prefix_incremental_arc190_b.cpp', 'test/benchmark/arc190_b_sweeps.cpp']:
                subprocess.run([compiler, *flags, source, '-o', binary], cwd=ROOT, check=True)
                for payload, expected in cases:
                    output = subprocess.check_output([binary], input=payload)
                    assert list(map(int, output.split())) == expected, (compiler, source, payload)
                bundle = subprocess.check_output(['oj-bundle', source], cwd=ROOT, stderr=subprocess.DEVNULL)
                assert b'#include "' not in bundle
                subprocess.run([compiler, *flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, cwd=ROOT, check=True)
                checks.append({'compiler': compiler, 'source': source, 'cases': len(cases), 'bundle': 'passed'})
                print(compiler, source, len(cases), 'cases and bundle passed', flush=True)
    names = ['math/combinatorics/binomial_prefix_sum_incremental.hpp', 'test/unit/binomial_prefix_sum_incremental.cpp',
             'test/benchmark/binomial_prefix_incremental_arc190_b.cpp', 'test/benchmark/arc190_b_sweeps.cpp',
             'test/benchmark/binomial_prefix_incremental_check.py']
    report = {'result': 'passed', 'checks': checks, 'brute_grid_limit': 6, 'seed': 20261020,
              'sources': {name: hashlib.sha256((ROOT / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names}}
    (OUT / 'validation.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
