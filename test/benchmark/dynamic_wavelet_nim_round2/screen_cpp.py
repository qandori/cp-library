import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

from variants import BASE, configure, save, sources


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, default=Path.cwd())
    parser.add_argument('--cases', type=Path, required=True)
    parser.add_argument('--small', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=3)
    parser.add_argument('--only', nargs='+')
    parser.add_argument('--generic', action='store_true')
    args = parser.parse_args()
    repo = args.repo.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    args.work = args.work.resolve()
    lock = open('/tmp/cp-wm-opt.lock', 'w')
    fcntl.flock(lock, fcntl.LOCK_EX)
    os.sched_setaffinity(0, {2})
    before, after = sources(repo, True), sources(repo)
    offsets = dict(before)
    offsets['components/dynamic_quad_wavelet.hpp'] = after['components/dynamic_quad_wavelet.hpp']
    variants = dict(before=before, offsets=configure(offsets), leaf=configure(after),
                    pair=configure(after, pair=True))
    for block in [2048, 4096, 8192, 16384]:
        variants[f'block{block}'] = configure(after, block=block)
        variants[f'quarter{block}'] = configure(after, block=block, quarter=True)
        variants[f'full{block}'] = configure(after, block=block, quarter=True, full=True)
    variants['legacy_full8192'] = configure(offsets, block=8192, quarter=True, full=True)
    variants['pair_full8192'] = configure(after, block=8192, quarter=True, full=True, pair=True)
    variants['half_full8192'] = configure(after, block=8192, full=True)
    for block in [4096, 8192]:
        variants[f'compact{block}'] = configure(after, block=block, quarter=True, compact=True)
        variants[f'pair_compact{block}'] = configure(after, block=block, quarter=True, compact=True, pair=True)
    packed = configure(after, block=8192, quarter=True, full=True)
    packed['components/dynamic_quad_vector.hpp'] = packed['components/dynamic_quad_vector.hpp'].replace('&& has_avx512()', '&& false').replace('&& has_avx2()', '&& false')
    variants['packed_full8192'] = packed
    for name in ['leaf_only', 'path_only']:
        headers = dict(after)
        patch = repo / 'test/benchmark/dynamic_wavelet_nim_round2/candidates' / (name + '.patch')
        directory = args.work / ('patch_' + name)
        save(directory, before)
        subprocess.run(['patch', '-p1', '-i', str(patch)], cwd=directory, check=True,
                       stdout=subprocess.DEVNULL)
        headers['components/dynamic_quad_vector.hpp'] = (directory / 'components/dynamic_quad_vector.hpp').read_text()
        variants[name + '_full8192'] = configure(headers, block=8192, quarter=True, full=True)
    if args.only:
        variants = {p: variants[p] for p in args.only}
    flags = ['-std=gnu++23', '-O2', '-UNDEBUG', '-ftrivial-auto-var-init=zero', '-fno-lto',
             '-DDWM_FAST_IO=1', '-DDWM_COUNT_DIRECT=1', '-DDWM_INSTRUMENT=1']
    if not args.generic:
        flags.append('-march=native')
    driver = repo / 'test/benchmark/dynamic_wavelet_quaternary/abc343f.cpp'
    result = dict(created_at=datetime.now(timezone.utc).isoformat(), baseline=BASE,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  cpu=next(x for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name')),
                  cpu_affinity=[2], flags=flags, trials=args.trials, driver_sha256=sha(driver),
                  sources={}, cases=[])
    for name, headers in variants.items():
        directory = args.work / name
        save(directory, headers)
        result['sources'][name] = {p: hashlib.sha256(s.encode()).hexdigest() for p, s in headers.items()}
        subprocess.run(['g++', *flags, '-I' + str(directory), '-I' + str(repo), str(driver), '-o', str(directory / 'solver')], check=True)
        for path in sorted(args.small.glob('*.in')):
            with path.open('rb') as inp:
                p = subprocess.run([str(directory / 'solver')], stdin=inp, capture_output=True, timeout=30, check=True)
            assert p.stdout.split() == path.with_suffix('.out').read_bytes().split(), (name, path)
        print(name, 'compiled and validated', flush=True)
    rng = random.Random(20261020)
    for path in sorted(args.cases.glob('*.in')):
        expected = path.with_suffix('.out').read_bytes().split()
        values = {name: [] for name in variants}
        for trial in range(args.trials + 1):
            names = list(variants)
            rng.shuffle(names)
            for name in names:
                with path.open('rb') as inp:
                    begin = time.perf_counter()
                    p = subprocess.run([str(args.work / name / 'solver')], stdin=inp, capture_output=True, check=True)
                    wall = time.perf_counter() - begin
                assert p.stdout.split() == expected, (path, name)
                row = json.loads(p.stderr)
                row['process_wall'] = wall
                if trial:
                    values[name].append(row)
        row = dict(name=path.stem, input_sha256=sha(path), output_sha256=sha(path.with_suffix('.out')),
                   trials=values, median={name: {key: statistics.median(x[key] for x in items)
                                               for key in ['build_cpu', 'operation_cpu', 'process_wall']}
                                          for name, items in values.items()})
        result['cases'].append(row)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')
        print(path.stem, {name: round(1000 * data['operation_cpu'], 3) for name, data in row['median'].items()}, flush=True)


if __name__ == '__main__':
    main()
