#!/usr/bin/env python3
"""Freeze C++ controls, then compare them with separately compiled Nim controls."""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import shutil
import statistics
import subprocess
import time

HEADERS = ['data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp',
           'components/dynamic_bit_vector.hpp', 'components/dynamic_quad_vector.hpp',
           'components/dynamic_quad_wavelet.hpp', 'components/fast_io.hpp']
FLAGS = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero',
         '-UNDEBUG', '-fno-lto', '-Wall', '-Wextra', '-Werror',
         '-DDWM_FAST_IO=1', '-DDWM_COUNT_DIRECT=1']


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.next')
    data = json.dumps(value, indent=2) + '\n'
    for attempt in range(6):
        try:
            temporary.write_text(data)
            temporary.replace(path)
            return
        except OSError:
            if attempt == 5:
                raise
            time.sleep(0.05 * (1 << attempt))


def cpu_now():
    usage = resource.getrusage(resource.RUSAGE_CHILDREN)
    return usage.ru_utime + usage.ru_stime


def prepare(args):
    args.work.mkdir(parents=True, exist_ok=True)
    target = args.work / 'cpp.json'
    if target.exists():
        raise RuntimeError('Refusing to replace existing frozen controls')
    repo = args.repo.resolve()
    driver = args.work / 'abc343f.cpp'
    shutil.copyfile(repo / 'test/benchmark/dynamic_wavelet_quaternary/abc343f.cpp', driver)
    compiler = shutil.which(args.cxx)
    record = {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'helper_sha256': sha(__file__), 'baseline': args.baseline,
              'working_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip(),
              'flags': FLAGS, 'compiler_path': compiler, 'compiler_sha256': sha(compiler),
              'compiler': subprocess.check_output([compiler, '--version'], text=True),
              'driver_sha256': sha(driver), 'sources': {}, 'binaries': {}}
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for arm in ['before', 'after']:
            directory = args.work / arm
            record['sources'][arm] = {}
            for relative in HEADERS:
                path = directory / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(subprocess.check_output(['git', 'show', args.baseline + ':' + relative], cwd=repo)
                                 if arm == 'before' else (repo / relative).read_bytes())
                record['sources'][arm][relative] = sha(path)
            for domain in args.domains:
                extra = [] if domain == 'i32' else ['-DDWM_VALUE_TYPE=std::uint32_t', '-DDWM_BITS=30']
                for family in ['stream', 'api']:
                    name = f'cpp_{arm}_{domain}_{family}'
                    binary = args.work / name
                    command = [compiler, *FLAGS, *extra, f'-DDWM_INSTRUMENT={int(family == "api")}',
                               '-I', str(directory), str(driver), '-o', str(binary)]
                    subprocess.run(command, check=True)
                    record['binaries'][name] = {'path': str(binary), 'sha256': sha(binary),
                                                'command': command, 'family': family, 'source': arm,
                                                'semantics': 'signed int32 / 32 bits' if domain == 'i32' else 'unsigned uint32 / 30 bits',
                                                'caller': 'FastInput/FastOutput + kth_largest(0), prev_value, count'}
                    print('compiled', name, flush=True)
    save(target, record)


def measure(args):
    if args.output.exists():
        raise RuntimeError('Refusing to replace existing measurements')
    cpp = json.loads((args.work / 'cpp.json').read_text())
    nim = json.loads((args.reference / 'preparation.json').read_text())
    for arm, headers in cpp['sources'].items():
        for relative, expected in headers.items():
            assert sha(args.work / arm / relative) == expected, (arm, relative)
    assert sha(args.reference / 'Supplied.nim') == nim['source_sha256']
    assert sha(args.reference / 'Instrumented.nim') == nim['api_source_sha256']
    binaries = dict(cpp['binaries'])
    for name, value in nim['binaries'].items():
        binaries['nim_' + name] = {**value, 'family': 'api' if name.endswith('_api') else 'stream',
                                   'semantics': nim['semantics']}
    if args.only_variants:
        unknown = set(args.only_variants) - set(binaries)
        if unknown:
            raise RuntimeError('Unknown variants: ' + ', '.join(sorted(unknown)))
        binaries = {name: binary for name, binary in binaries.items() if name in args.only_variants}
    for binary in binaries.values():
        assert sha(binary['path']) == binary['sha256']
    cases = [json.loads(path.read_text()) for path in sorted(args.cases.glob('*.json'))
             if path.name != 'manifest.json' and (not args.only_cases or path.stem in args.only_cases)]
    small = [json.loads(path.read_text()) for path in sorted(args.small.glob('*.json')) if path.name != 'manifest.json']
    for case in cases + small:
        assert sha(case['input']['path']) == case['input']['sha256']
        assert sha(case['expected']['path']) == case['expected']['sha256']
    result = {'metadata': {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                           'helper_sha256': sha(__file__), 'cpp': cpp, 'nim': nim, 'binaries': binaries,
                           'lscpu': subprocess.check_output(['lscpu'], text=True), 'uname': list(os.uname()),
                           'cpu': args.cpu, 'repeats': args.repeats, 'seed': args.seed,
                           'timing': 'exclusive flock, one warmup, shuffled order, blocking wait, regular input/output files; process wall plus RUSAGE_CHILDREN CPU; API timers exclude input/loading/output'},
              'cases': cases, 'validation': [], 'validation_reused': [], 'warmups': [], 'samples': [], 'summary': {}}
    validated = {}
    if args.validated_from:
        prior = json.loads(args.validated_from.read_text())
        for row in prior['validation']:
            binary = prior['metadata']['binaries'][row['variant']]
            key = (row['case'], row['variant'], row.get('input_sha256'), binary['sha256'])
            validated[key] = row
        result['metadata']['validation_reused_from'] = {'path': str(args.validated_from), 'sha256': sha(args.validated_from)}
    temporary = args.work / 'measured-output.txt'

    def run(case, name, trial, sequence):
        binary = binaries[name]
        with open(case['input']['path'], 'rb') as source, temporary.open('wb') as destination:
            begin_wall, begin_cpu = time.perf_counter_ns(), cpu_now()
            process = subprocess.run([binary['path']], stdin=source, stdout=destination,
                                     stderr=subprocess.PIPE, check=True)
            cpu, wall = cpu_now() - begin_cpu, (time.perf_counter_ns() - begin_wall) / 1e9
        assert sha(temporary) == case['expected']['sha256'], (case['name'], name, 'wrong answer')
        row = {'case': case['name'], 'variant': name, 'trial': trial, 'sequence': sequence,
               'input_sha256': case['input']['sha256'], 'wall_seconds': wall, 'cpu_seconds': cpu,
               'answer_sha256': sha(temporary)}
        if binary['family'] == 'api':
            row['api'] = json.loads(process.stderr)
            assert row['api']['answers'] == case['stats']['query']
            row['api']['total_cpu'] = row['api']['build_cpu'] + row['api']['operation_cpu']
        return row

    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        for case in small:
            for name in binaries:
                key = (case['name'], name, case['input']['sha256'], binaries[name]['sha256'])
                if key in validated:
                    assert validated[key]['answer_sha256'] == case['expected']['sha256']
                    result['validation_reused'].append(validated[key])
                    continue
                result['validation'].append(run(case, name, -2, -1))
            print('validated', case['name'], flush=True)
        rng = random.Random(args.seed)
        rng.shuffle(cases)
        sequence = 0
        for case in cases:
            for trial in range(-1, args.repeats):
                names = list(binaries)
                rng.shuffle(names)
                for name in names:
                    row = run(case, name, trial, sequence)
                    result['warmups' if trial == -1 else 'samples'].append(row)
                    sequence += 1
                save(args.output, result)
            summary = {}
            for name in binaries:
                rows = [r for r in result['samples'] if r['case'] == case['name'] and r['variant'] == name]
                summary[name] = {field: statistics.median(r[field] for r in rows)
                                 for field in ['wall_seconds', 'cpu_seconds']}
                if binaries[name]['family'] == 'api':
                    summary[name]['api'] = {field: statistics.median(r['api'][field] for r in rows)
                                            for field in ['build_cpu', 'operation_cpu', 'total_cpu']}
            result['summary'][case['name']] = summary
            save(args.output, result)
            print('measured', case['name'], {n: round(v['wall_seconds'] * 1000, 3) for n, v in summary.items()}, flush=True)


def main():
    parser = argparse.ArgumentParser(__doc__)
    sub = parser.add_subparsers(dest='mode', required=True)
    p = sub.add_parser('prepare')
    p.add_argument('--repo', type=Path, default=Path.cwd())
    p.add_argument('--work', type=Path, required=True)
    p.add_argument('--baseline', default='5012542')
    p.add_argument('--domains', nargs='+', choices=['i32', 'u30'], default=['i32', 'u30'])
    p.add_argument('--cxx', default='g++')
    p = sub.add_parser('measure')
    p.add_argument('--work', type=Path, required=True)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--cases', type=Path, required=True)
    p.add_argument('--small', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--cpu', type=int, default=2)
    p.add_argument('--repeats', type=int, default=7)
    p.add_argument('--seed', type=int, default=20261020)
    p.add_argument('--only-cases', nargs='+')
    p.add_argument('--only-variants', nargs='+')
    p.add_argument('--validated-from', type=Path)
    args = parser.parse_args()
    args.work = args.work.resolve()
    if args.mode == 'measure' and args.repeats < 1:
        parser.error('--repeats must be positive')
    if args.mode == 'prepare':
        prepare(args)
    else:
        measure(args)


if __name__ == '__main__':
    main()
