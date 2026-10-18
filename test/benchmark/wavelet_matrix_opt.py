import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '523956fbb612ed975d73d1fcac414238f8ec2d16'
HEADER = 'data_structure/wavelet_matrix.hpp'
PROBLEMS = {'kth': ('range_kth_smallest', 'test/wavelet_matrix_kth.test.cpp', 402999),
            'freq': ('static_range_frequency', 'test/wavelet_matrix_freq.test.cpp', 205664),
            'sum': ('static_range_sum_with_upper_bound', 'test/wavelet_matrix_sum.test.cpp', 402379)}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    data = json.dumps(value, indent=2) + '\n'
    for attempt in range(6):
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(mode='w', dir=path.parent, prefix='.' + path.name, suffix='.tmp', delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(data)
            temporary.replace(path)
            return
        except OSError:
            if attempt == 5:
                raise
            time.sleep(0.05)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--suite', choices=['synthetic', 'official'], default='synthetic')
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--quick', action='store_true')
    parser.add_argument('--problem-root', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/data_structure')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {args.cpu})
    lock = open('/tmp/cp-wm-opt.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    flags = ['-std=gnu++23', '-O2', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE']
    if not args.generic:
        flags.append('-march=native')
    frozen = [HEADER, 'components/quad_bit_vector.hpp', 'components/oct_bit_vector.hpp', 'components/fast_io.hpp', 'components/splitmix64.hpp',
              'test/benchmark/wavelet_matrix_opt.cpp', 'test/benchmark/wavelet_matrix_opt.py', 'test/benchmark/wavelet_matrix_opt_reference.cpp']
    frozen += [row[1] for row in PROBLEMS.values()]
    environment = dict(measured_at=datetime.now(timezone.utc).isoformat(), baseline=BASELINE, flags=flags,
        compiler=subprocess.check_output(['g++', '--version'], text=True), cpu=subprocess.check_output(['lscpu'], text=True),
        kernel=subprocess.check_output(['uname', '-a'], text=True), affinity=[args.cpu], trials=args.trials,
        warmup=1, order_seed=20261019, suite=args.suite, quick=args.quick,
        sha256={path: sha(ROOT / path) for path in frozen}, reference_sha256={})
    metadata = json.loads((ROOT / 'test/benchmark/results/wavelet_matrix_opt_reference.json').read_text())
    args.output.mkdir(parents=True, exist_ok=True)
    environment['completed'] = False
    write_json(args.output / 'environment.json', environment)
    results = []
    rng = random.Random(20261019)
    with tempfile.TemporaryDirectory(prefix='cp-wm-opt-') as temporary:
        work = Path(temporary)
        headers = {}
        for name in ['before', 'current']:
            folder = work / name
            for path in [HEADER, 'components/quad_bit_vector.hpp'] + ([] if name == 'before' else ['components/oct_bit_vector.hpp', 'components/splitmix64.hpp']):
                destination = folder / path
                destination.parent.mkdir(parents=True, exist_ok=True)
                code = subprocess.check_output(['git', 'show', BASELINE + ':' + path], text=True) if name == 'before' else (ROOT / path).read_text()
                destination.write_text(code)
            headers[name] = folder / HEADER

        def compile(source, binary, extra=()):
            with exclusive():
                subprocess.run(['g++', *flags, '-I', str(ROOT), *extra, str(source), '-o', str(binary)], check=True)
            return binary

        def reference(identity):
            source = ROOT / '.verify-helper/cache/wm-opt/reference' / (str(identity) + '.cpp')
            row = next(row for row in metadata['references'] if row['id'] == identity)
            assert sha(source) == row['source_sha256']
            environment['reference_sha256'][str(identity)] = sha(source)
            return compile(source, work / ('reference_' + str(identity)))

        def save():
            write_json(args.output / 'results.json', results)

        if args.suite == 'synthetic':
            binaries = {name: compile(ROOT / 'test/benchmark/wavelet_matrix_opt.cpp', work / (name + '_synthetic'), ['-DWM_OPT_HEADER="' + str(header) + '"']) for name, header in headers.items()}
            cases = [(200000, mode, op, 0, 64) for mode in [0, 1, 2, 7] for op in ['build', 'kth', 'freq', 'count', 'prev', 'next', 'prepare', 'small', 'large', 'eager']]
            cases += [(200000, mode, op, 0, 32) for mode in [0, 1, 2] for op in ['build', 'kth', 'freq', 'count', 'small']]
            cases += [(200000, mode, op, 0, 32) for mode in [9, 10, 11, 12] for op in ['count']]
            cases += [(1000000, mode, op, 0, 64) for mode in [0, 1, 2] for op in ['build', 'kth', 'freq', 'count']]
            cases += [(200000, mode, op, 0, 64) for mode in [3, 4, 5, 6] for op in ['build', 'kth', 'freq', 'count', 'small']]
            cases += [(200000, 0, op, 8, 64) for op in ['kth', 'freq', 'count', 'prev', 'next', 'small', 'large']]
            cases += [(200000, 0, op, 0, 64) for op in ['between', 'access', 'select', 'sum']]
            cases += [(500000, mode, op, 0, 32) for mode in [0, 1, 7, 13] for op in ['count', 'prepared_count', 'prepare_count', 'prepared_select']]
            if args.quick:
                cases = [(200000, m, op, 0, 64) for m in [0, 1, 2, 7] for op in ['build', 'kth', 'freq', 'count', 'small', 'eager']]
                cases += [(1000000, 0, 'kth', 0, 64), (200000, 0, 'kth', 8, 64)]
                cases += [(500000, mode, op, 0, 32) for mode in [1, 13] for op in ['count', 'prepared_count', 'prepare_count']]
            for n, mode, op, length, width in cases:
                samples = {name: [] for name in binaries}
                checks = set()
                with exclusive():
                    for trial in range(-1, args.trials):
                        order = list(binaries)
                        rng.shuffle(order)
                        for name in order:
                            row = json.loads(subprocess.check_output([binaries[name], str(n), '50000', str(mode), str(length), op, str(width), '81917']))
                            checks.add(row['checksum'])
                            if trial >= 0:
                                samples[name].append(row['ms'])
                assert len(checks) == 1, (n, mode, op, length, width, checks)
                result = dict(n=n, mode=mode, op=op, length=length, width=width, q=50000, seed=81917,
                    samples_ms=samples, median_ms={name: statistics.median(values) for name, values in samples.items()}, checksum=checks.pop())
                if op in ['build', 'prepare', 'eager']:
                    result['peak_rss_kib'] = {}
                    with exclusive():
                        for name, binary in binaries.items():
                            rss = work / 'rss'
                            row = json.loads(subprocess.check_output(['/usr/bin/time', '-f', '%M', '-o', str(rss), binary, str(n), '50000', str(mode), str(length), op, str(width), '81917']))
                            assert row['checksum'] == result['checksum']
                            result['peak_rss_kib'][name] = int(rss.read_text())
                results.append(result)
                print(n, mode, op, length, width, result['median_ms'], flush=True)
                save()
        else:
            for kind, (problem, driver, identity) in PROBLEMS.items():
                binaries = {}
                for name, header in headers.items():
                    for io in ['stdio', 'fastio']:
                        code = subprocess.check_output(['git', 'show', BASELINE + ':' + driver], text=True) if io == 'stdio' else (ROOT / driver).read_text()
                        if name == 'before':
                            code = code.replace('\twm.prepare_count();\n', '')
                        elif kind == 'freq' and io == 'stdio':
                            code = code.replace('\tWaveletMatrix wm(a);', '\tWaveletMatrix wm(a);\n\twm.prepare_count();')
                        code = code.replace('../data_structure/wavelet_matrix.hpp', str(header)).replace('../components/fast_io.hpp', 'components/fast_io.hpp')
                        source = work / (kind + '_' + name + '_' + io + '.cpp')
                        source.write_text(code)
                        binaries[name + '_' + io] = compile(source, source.with_suffix(''))
                binaries['reference_' + str(identity)] = reference(identity)
                kernels = {}
                if kind == 'kth':
                    source = ROOT / 'test/benchmark/wavelet_matrix_opt_reference.cpp'
                    for name, header in headers.items():
                        kernels[name] = compile(source, work / (name + '_kernel'), ['-DWM_OPT_HEADER="' + str(header) + '"'])
                    kernels['reference_402999'] = compile(source, work / 'reference_kernel', [] if not args.generic else ['-mavx2', '-mpopcnt'])
                folder = args.problem_root / problem
                cases = sorted((folder / 'in').glob('*.in'))
                if args.quick:
                    cases = [p for p in cases if p.stem in ['max_random_00', 'dense_small_a_00', 'random_00', 'random_03', 'random_04', 'small_a_x_00']]
                for case in cases:
                    inp = work / 'input'
                    shutil.copyfile(case, inp)
                    answer = folder / 'out' / (case.stem + '.out')
                    for phase, programs in [('whole', binaries), ('kernel', kernels)]:
                        if not programs:
                            continue
                        samples = {name: [] for name in programs}
                        build_samples = {name: [] for name in programs}
                        query_samples = {name: [] for name in programs}
                        output_hashes = {}
                        expected = 0
                        if phase == 'kernel':
                            for x in map(int, answer.read_bytes().split()):
                                expected = ((expected ^ x) * 0x9e3779b97f4a7c15) & ((1 << 64) - 1)
                        with exclusive():
                            for trial in range(-1, args.trials):
                                order = list(programs)
                                rng.shuffle(order)
                                for name in order:
                                    with inp.open('rb') as stream:
                                        if phase == 'kernel':
                                            row = json.loads(subprocess.check_output([programs[name]], stdin=stream))
                                            assert row['checksum'] == str(expected), (case, name, row)
                                            ms = row['ms']
                                        else:
                                            output = work / 'output'
                                            with output.open('wb') as destination:
                                                start = time.perf_counter()
                                                subprocess.run([programs[name]], stdin=stream, stdout=destination, check=True)
                                                ms = (time.perf_counter() - start) * 1000
                                            digest = sha(output)
                                            if output_hashes.get(name) != digest:
                                                subprocess.run([folder / 'checker', inp, output, answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                                                output_hashes[name] = digest
                                    if trial >= 0:
                                        samples[name].append(ms)
                                        if phase == 'kernel':
                                            build_samples[name].append(row['build_ms'])
                                            query_samples[name].append(row['query_ms'])
                        result = dict(problem=problem, case=case.stem, phase=phase, input_sha256=sha(inp), answer_sha256=sha(answer), output_sha256=output_hashes,
                            samples_ms=samples, median_ms={name: statistics.median(values) for name, values in samples.items()})
                        if phase == 'kernel':
                            result.update(checksum=str(expected), build_samples_ms=build_samples, query_samples_ms=query_samples,
                                build_median_ms={name: statistics.median(values) for name, values in build_samples.items()},
                                query_median_ms={name: statistics.median(values) for name, values in query_samples.items()})
                        results.append(result)
                        print(problem, case.stem, phase, result['median_ms'], flush=True)
                        save()
        for path, digest in environment['sha256'].items():
            assert sha(ROOT / path) == digest, path
    environment['completed'] = True
    environment['completed_at'] = datetime.now(timezone.utc).isoformat()
    write_json(args.output / 'environment.json', environment)
    print('All measurements passed', flush=True)


if __name__ == '__main__':
    main()
