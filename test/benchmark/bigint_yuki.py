import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

from bigint_history import BASE_REVISION, materialize


ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def generate(shape, n=30000, m=50000):
    rng = random.Random(36691182839)
    edges, used = [], set()
    for current_shape in ['chain', 'random', 'star']:
        edges, used = [], set()
        for v in range(1, n):
            u = v - 1 if current_shape == 'chain' else 0 if current_shape == 'star' else rng.randrange(v)
            used.add((u, v))
            a, b = rng.randrange(1, 301), rng.randrange(1, 301)
            g = math.gcd(a, b)
            edges.append((u + 1, v + 1, a // g, b // g))
        while len(edges) < m:
            u, v = sorted(rng.sample(range(n), 2))
            if (u, v) in used:
                continue
            used.add((u, v))
            a, b = rng.randrange(1, 301), rng.randrange(1, 301)
            g = math.gcd(a, b)
            edges.append((u + 1, v + 1, a // g, b // g))
        if current_shape == shape:
            return (f'{n} {m}\n' + ''.join(' '.join(map(str, edge)) + '\n' for edge in edges)).encode()
    raise ValueError(shape)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--boost-include', type=Path)
    parser.add_argument('--reference-dir', type=Path, required=True)
    parser.add_argument('--acl-include', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    if args.trials < 1:
        parser.error('--trials must be positive')
    os.sched_setaffinity(0, {args.cpu})
    cache = ROOT / '.verify-helper/cache/bigint-comparison'
    baseline = materialize(ROOT, cache / 'headers-baseline')
    candidate = materialize(ROOT, cache / 'headers-candidate')
    patch = ROOT / 'test/benchmark/bigint_yuki_candidate.patch'
    normalized_patch = cache / 'candidate.patch'
    normalized_patch.write_bytes(patch.read_bytes().replace(b'\r\n', b'\n'))
    subprocess.run(['git', 'apply', '--unidiff-zero', '--directory=' + candidate.relative_to(ROOT).as_posix(), str(normalized_patch)], cwd=ROOT, check=True)
    args.output.mkdir(parents=True, exist_ok=True)
    (cache / 'bin').mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    includes = ['-I', str(baseline)]
    if args.boost_include:
        includes += ['-I', str(args.boost_include.resolve())]
    common = ROOT / 'test/benchmark/bigint_yuki.cpp'
    programs = {
        'baseline': (baseline, baseline / 'graph/dijkstra.hpp', []),
        'candidate': (candidate, ROOT / 'graph/dijkstra.hpp', ['-DBENCH_HAS_GCD']),
        'boost_fixed_old': (baseline, baseline / 'graph/dijkstra.hpp', ['-DBENCH_BOOST_FIXED']),
        'boost_dynamic_old': (baseline, baseline / 'graph/dijkstra.hpp', ['-DBENCH_BOOST_DYNAMIC']),
        'boost_fixed': (baseline, ROOT / 'graph/dijkstra.hpp', ['-DBENCH_BOOST_FIXED']),
        'boost_dynamic': (baseline, ROOT / 'graph/dijkstra.hpp', ['-DBENCH_BOOST_DYNAMIC']),
    }
    binaries = {}
    for name, (headers, dijkstra, defines) in programs.items():
        binary = cache / 'bin' / name
        subprocess.run([args.cxx, *flags, '-I', str(headers), *includes,
                        '-DBENCH_DIJKSTRA_HEADER="' + str(dijkstra) + '"', *defines,
                        str(common), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    for identity in [1182839, 1185228]:
        source = args.reference_dir / f'{identity}.cpp'
        binary = cache / 'bin' / str(identity)
        subprocess.run([args.cxx, '-std=c++23', '-O2', '-DONLINE_JUDGE',
                        *(['-march=native', '-ftrivial-auto-var-init=zero'] if args.native else []),
                        *includes, '-I', str(args.acl_include.resolve()), str(source), '-o', str(binary)], check=True)
        binaries[str(identity)] = str(binary)
    result = {'environment': {
        'compiler': subprocess.check_output([args.cxx, '--version'], text=True),
        'kernel': subprocess.check_output(['uname', '-a'], text=True),
        'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
        'affinity': [args.cpu], 'flags': flags,
        'boost_version_header': next(line for line in ((args.boost_include or Path('/usr/include')) / 'boost/version.hpp').read_text().splitlines()
                                     if line.startswith('#define BOOST_LIB_VERSION')),
        'trials': args.trials, 'warmup': 1, 'input_seed': 36691182839, 'schedule_seed': 1185228,
        'metric': 'wall time including startup, input, computation, output and exit',
    }, 'baseline_revision': BASE_REVISION,
        'sources': {str(p.relative_to(ROOT)): digest(p) for p in [common, patch, ROOT / 'graph/dijkstra.hpp']},
        'reference_sources': {str(identity): digest(args.reference_dir / f'{identity}.cpp') for identity in [1182839, 1185228]},
        'candidate_sha256': digest(candidate / 'math/bigint.hpp'), 'workloads': {}}
    rng = random.Random(1185228)
    print('Compilation complete', flush=True)
    for shape in ['chain', 'random', 'star']:
        data = generate(shape)
        samples = {name: [] for name in binaries}
        expected = None
        for trial in range(args.trials + 1):
            names = list(binaries)
            rng.shuffle(names)
            for name in names:
                start = time.perf_counter_ns()
                run = subprocess.run([binaries[name]], input=data, capture_output=True, check=True, timeout=30)
                elapsed = (time.perf_counter_ns() - start) / 1e6
                checksum = hashlib.sha256(run.stdout).hexdigest()
                if expected is None:
                    expected = checksum
                assert expected == checksum, (shape, name, trial)
                if trial:
                    samples[name].append({'wall_ms': elapsed, 'phase_ms': json.loads(run.stderr) if run.stderr else None})
        result['workloads'][shape] = {'input_sha256': hashlib.sha256(data).hexdigest(), 'output_sha256': expected,
                                      'n': 30000, 'm': 50000, 'runs': samples}
        print(shape, {name: round(statistics.median(run['wall_ms'] for run in runs), 3) for name, runs in samples.items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
