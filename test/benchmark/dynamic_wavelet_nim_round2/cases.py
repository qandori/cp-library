#!/usr/bin/env python3
"""Reuse the independent ABC343 F oracle and add update/query controls."""
import argparse
import fcntl
import hashlib
import importlib.util
import json
from pathlib import Path
import random

PREVIOUS = Path(__file__).resolve().parents[1] / 'dynamic_wavelet_quaternary/cases.py'
spec = importlib.util.spec_from_file_location('abc343f_cases', PREVIOUS)
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)


def boundaries(directory):
    rng = random.Random(20261026)
    n = 32770
    initial = [rng.choice([1, 7, 1_000_000_000, rng.randrange(1, oracle.LIMIT + 1)]) for _ in range(n)]
    operations = []
    positions = [0, 1, 31, 32, 1023, 1024, 4095, 4096, 8191, 8192, 16383, 16384, 32767, n - 1]
    for repeat in range(8):
        for p in positions:
            for x in [1, 7, 1_000_000_000, rng.randrange(1, oracle.LIMIT + 1)]:
                operations.append((1, p, x))
                for l, r in [(0, n), (p, p + 1), (max(0, p - 1), min(n, p + 2)),
                             (max(0, p - 33), min(n, p + 33)), (0, p + 1), (p, n)]:
                    operations.append((2, l, r))
    return oracle.write_case(directory, 'small_leaf8192', initial, operations, 20261026, cross_check=True)


def extra(directory, shape, n, q, seed):
    rng = random.Random(seed)
    initial = [rng.randrange(1, oracle.LIMIT + 1) for _ in range(n)]
    operations = []
    for i in range(q):
        if shape == 'update_heavy' and i % 100 != 99:
            operations.append((1, rng.randrange(n), rng.randrange(1, oracle.LIMIT + 1)))
        else:
            l = rng.randrange(n)
            r = rng.randrange(l + 1, n + 1)
            if i % 1000 == 999:
                l, r = 0, n
            operations.append((2, l, r))
    return oracle.write_case(directory, shape, initial, operations, seed)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--small', action='store_true')
    parser.add_argument('--large', action='store_true')
    parser.add_argument('--n', type=int, default=200000)
    parser.add_argument('--q', type=int, default=200000)
    args = parser.parse_args()
    assert 1 <= args.n <= 200000 and 1 <= args.q <= 200000
    args.root.mkdir(parents=True, exist_ok=True)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if args.small:
            directory = args.root / 'cases-small'
            rows = oracle.small_cases(directory)
            rows.append(boundaries(directory))
            (directory / 'manifest.json').write_text(json.dumps(rows, indent=2) + '\n')
        if args.large:
            directory = args.root / 'cases-large'
            rows = [oracle.large_case(directory, shape, args.n, args.q, 20261020 + i)
                    for i, shape in enumerate(oracle.SHAPES)]
            rows += [extra(directory, shape, args.n, args.q, 20261027 + i)
                     for i, shape in enumerate(['update_heavy', 'query_only'])]
            for row in rows:
                row['round2_generator_sha256'] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
                (directory / (row['name'] + '.json')).write_text(json.dumps(row, indent=2) + '\n')
            (directory / 'manifest.json').write_text(json.dumps(rows, indent=2) + '\n')


if __name__ == '__main__':
    main()
