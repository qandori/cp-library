import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import resource
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
MOD = 998244353


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    stack_limit = resource.getrlimit(resource.RLIMIT_STACK)[1]
    resource.setrlimit(resource.RLIMIT_STACK, (stack_limit, stack_limit))
    cache = ROOT / '.verify-helper/cache/polynomial-gcd-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    sources = {'gcd': ROOT / 'test/benchmark/polynomial_gcd.cpp',
               'inverse': ROOT / 'test/polynomial_inv_mod.test.cpp', 'reference_409589': args.reference.resolve()}
    binaries = {}
    for name, source in sources.items():
        binary = cache / name
        extra = ['-march=haswell'] if name.startswith('reference_') and not args.native else []
        subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    rng = random.Random(294384)
    def polynomial(n):
        return [rng.randrange(MOD) for _ in range(n-1)] + [rng.randrange(1, MOD)]
    def encode(a, b):
        return (f'{len(a)} {len(b)}\n' + ' '.join(map(str, a)) + '\n' + ' '.join(map(str, b)) + '\n').encode()
    cases = []
    for n in [256, 4096]:
        data = encode(polynomial(n), polynomial(n-1))
        cases.append((f'gcd_random_{n}', data, {'half_gcd': [binaries['gcd']], 'euclid': [binaries['gcd'], 'euclid']}))
    n = 50000
    for shape in ['random', 'sparse', 'equal', 'constant']:
        a, b = polynomial(n), polynomial(n-1)
        if shape == 'sparse':
            a, b = [MOD-1] + [0]*(n-2) + [1], [MOD-1] + [0]*(n//2-2) + [1]
        if shape == 'equal':
            b = a[:]
        if shape == 'constant':
            b = [7]
        data = encode(a, b)
        programs = {'half_gcd': [binaries['gcd']]}
        if shape != 'random':
            programs['euclid'] = [binaries['gcd'], 'euclid']
        cases.append((f'gcd_{shape}_{n}', data, programs))
        cases.append((f'inverse_{shape}_{n}', data, {'library': [binaries['inverse']],
            'reference_409589': [binaries['reference_409589']]}))
    result = {'platform': platform.platform(),
              'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              'flags': flags, 'reference_extra_flags': [] if args.native else ['-march=haswell'],
              'cpu_affinity': [2], 'stack_limit': stack_limit, 'trials': 7, 'seed': 294384,
              'metric': 'wall time including startup, input, construction, computation, output and exit',
              'sources': {name: sha(path) for name, path in sources.items()},
              'headers': {name: sha(ROOT/name) for name in ['fps/polynomial_gcd.hpp', 'components/polynomial_half_gcd.hpp',
                  'fps/fps.hpp', 'components/fps_operations.hpp', 'components/ntt.hpp', 'math/mint.hpp', 'components/fast_io.hpp']},
              'workloads': {}}
    for name, data, programs in cases:
        expected = None
        samples = {p: [] for p in programs}
        for command in programs.values():
            output = b' '.join(subprocess.check_output(command, input=data, timeout=30).split())
            if expected is None:
                expected = output
            assert output == expected, name
        schedule = list(programs)*7
        rng.shuffle(schedule)
        for program in schedule:
            start = time.perf_counter()
            output = subprocess.check_output(programs[program], input=data, timeout=30)
            samples[program].append((time.perf_counter()-start)*1000)
            assert b' '.join(output.split()) == expected, (name, program)
        item = {'input_sha256': hashlib.sha256(data).hexdigest(), 'input_bytes': len(data),
                'output_tokens_sha256': hashlib.sha256(expected).hexdigest(),
                'programs': {p: {'milliseconds': values, 'median_ms': statistics.median(values)} for p, values in samples.items()}}
        result['workloads'][name] = item
        print(name, {p: round(v['median_ms'], 3) for p, v in item['programs'].items()}, flush=True)
    (args.output/'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
