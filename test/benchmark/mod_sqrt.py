import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def prime(n):
    if n < 2:
        return False
    for p in [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37]:
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d //= 2
        s += 1
    for a in [2, 325, 9375, 28178, 450775, 9780504, 1795265022]:
        if a % n == 0:
            continue
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(s - 1):
            x = x * x % n
            if x == n - 1:
                break
        else:
            return False
    return True


def workloads():
    rng = random.Random(2126432)
    primes = []
    while len(primes) < 256:
        p = rng.randrange(10**8, 10**9) | 1
        if prime(p):
            primes.append(p)
    wide = [4294967311, 9223372036854775783, 18446744069414584321, 18446744073709551557]
    assert all(prime(p) for p in wide)
    result = {}
    for mode in ['mixed31', 'mod4_3', 'ntt998', 'full32', 'mixed64', 'two_adic64']:
        queries = []
        size = 20000 if '64' in mode else 100000
        pool = [p for p in primes if p % 4 == 3] if mode == 'mod4_3' else primes
        for i in range(size):
            p = (998244353 if mode == 'ntt998' else 3221225473 if mode == 'full32' else
                 18446744069414584321 if mode == 'two_adic64' else rng.choice(wide if mode == 'mixed64' else pool))
            if i % 4 == 0:
                x = rng.randrange(p)
                a = x * x % p
            elif i % 4 == 1:
                a = p - 1
            else:
                a = rng.randrange(p)
            queries.append((a, p))
        data = (f'{size}\n' + ''.join(f'{a} {p}\n' for a, p in queries)).encode()
        exists = [a == 0 or pow(a, (p - 1) // 2, p) == 1 for a, p in queries]
        result[mode] = (data, queries, exists)
    return result


def validate(output, queries, exists):
    values = list(map(int, output.split()))
    assert len(values) == len(queries)
    normalized = []
    for x, (a, p), valid in zip(values, queries, exists):
        if x == -1:
            assert not valid, (a, p, x)
        else:
            assert valid and 0 <= x < p and x * x % p == a, (a, p, x)
            x = min(x, p - x)
        normalized.append(str(x))
    return hashlib.sha256(('\n'.join(normalized) + '\n').encode()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    if args.trials < 1:
        parser.error('--trials must be positive')
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    cache = ROOT / '.verify-helper/cache/mod-sqrt-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    programs = {name: (ROOT / 'test/benchmark/mod_sqrt.cpp', [f'-DMOD_SQRT_BENCH_VARIANT={variant}'])
                for name, variant in [('library', 0), ('tonelli64', 1), ('cipolla', 2)]}
    for identity in [405759, 215966, 331256]:
        programs[str(identity)] = (args.references.resolve() / f'{identity}.cpp', [])
    binaries = {}
    for name, (source, extra) in programs.items():
        binary = cache / name
        subprocess.run(['g++', *flags, '-DONLINE_JUDGE', *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    cases = workloads()
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'kernel': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'cpu_affinity': [args.cpu], 'trials': args.trials, 'warmup': 1,
              'input_seed': 2126432, 'schedule_seed': 28063,
              'metric': 'wall time including process startup, input, computation, output and exit',
              'input_transport': 'regular file for every program',
              'sources': {name: digest(source) for name, (source, _) in programs.items()},
              'headers': {name: digest(ROOT / name) for name in ['math/mod_sqrt.hpp', 'components/montgomery.hpp',
                          'components/montgomery32.hpp', 'components/splitmix64.hpp', 'components/fast_io.hpp',
                          'test/benchmark/mod_sqrt_baseline.hpp']},
              'workloads': {}}
    rng = random.Random(28063)
    for mode, (data, queries, exists) in cases.items():
        input_path = cache / (mode + '.in')
        input_path.write_bytes(data)
        selected = list(binaries) if mode in ['mixed31', 'mod4_3', 'ntt998'] else ['library', 'tonelli64', 'cipolla']
        samples, expected = {name: [] for name in selected}, None
        for trial in range(args.trials + 1):
            names = selected.copy()
            rng.shuffle(names)
            for name in names:
                with input_path.open('rb') as input_file:
                    start = time.perf_counter_ns()
                    run = subprocess.run([binaries[name]], stdin=input_file, capture_output=True, check=True, timeout=60)
                    elapsed = (time.perf_counter_ns() - start) / 1e6
                checksum = validate(run.stdout, queries, exists)
                if expected is None:
                    expected = checksum
                assert checksum == expected, (mode, name)
                if trial:
                    samples[name].append(elapsed)
        result['workloads'][mode] = {'queries': len(queries), 'input_sha256': hashlib.sha256(data).hexdigest(),
                                    'canonical_output_sha256': expected,
                                    'milliseconds': samples, 'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
        print(mode, {name: round(statistics.median(values), 3) for name, values in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
