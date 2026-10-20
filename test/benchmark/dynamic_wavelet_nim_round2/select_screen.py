#!/usr/bin/env python3
import argparse
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
import sys
import time

from variants import BASE, configure, save as save_headers, sources


FLAGS = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero',
         '-UNDEBUG', '-fno-lto', '-Wall', '-Wextra', '-Werror']
DOMAINS = [('u7', 'std::uint32_t', 7), ('i64', 'std::int64_t', 64),
           ('u64', 'std::uint64_t', 64)]
WORKLOADS = [('u7', 'select', 'uniform', 1000000),
             ('i64', 'select', 'uniform', 200000),
             ('u64', 'select', 'uniform', 200000),
             ('u7', 'select-valid-dup', 'duplicates', 200000),
             ('u7', 'select-lastbit-miss', 'uniform', 200000),
             ('u7', 'select-high-k', 'uniform', 200000)]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save_json(path, value):
    temporary = path.with_suffix(path.suffix + '.next')
    for attempt in range(6):
        try:
            temporary.write_text(json.dumps(value, indent=2) + '\n')
            temporary.replace(path)
            return
        except OSError:
            if attempt == 5:
                raise
            time.sleep(0.05 * (1 << attempt))


def selector(source):
    begin = source.rindex('\n\tint select(int digit, int k) const {')
    return begin, source.index('\n};', begin)


def prepare(repo, work):
    baseline = sources(repo, baseline=True)
    binary = configure(sources(repo), block=8192, quarter=True, full=True, pair=True)
    scan = dict(binary)
    path = 'components/dynamic_quad_vector.hpp'
    first, last = selector(scan[path])
    old_first, old_last = selector(baseline[path])
    scan[path] = scan[path][:first] + baseline[path][old_first:old_last] + scan[path][last:]
    headers = {}
    for name, values in [('base', baseline), ('scan', scan), ('binary', binary)]:
        save_headers(work / name, values)
        headers[name] = {p: sha(work / name / p) for p in values}
    benchmark = repo / 'test/benchmark/dwm_avx_final'
    source = (benchmark / 'bench.cpp').read_text()
    source = source.replace('using WM=DynamicWaveletMatrix<T,BITS,DynamicBitVector<BLOCK,BRANCH>>;',
                            'using WM=DynamicWaveletMatrix<T,BITS>;')
    assert source.count('if(kind=="access")t=6;') == 1
    source = source.replace('if(kind=="access")t=6;',
                            'if(kind=="set-noop")t=2;else if(kind=="access")t=6;')
    needle = '  if(t==7)x=a.empty()?0:a[rng()%a.size()];'
    assert source.count(needle) == 1
    extra = ('  if(kind=="set-noop")x=a[p];\n'
             '  if(kind=="count"&&!(i&1)&&l<r)x=a[l+rng()%(r-l)];\n'
             '  if(kind=="rank"&&!(i&1)&&r)x=a[rng()%r];\n')
    source = source.replace(needle, extra + needle)
    source = source.replace(';if(kind==', ';\n  if(kind==')
    source = source.replace(';return decode_key(x);', ';\n  return decode_key(x);')
    (work / 'bench.cpp').write_text(source)
    for name in ['escape.cpp', 'vector_oracle.cpp']:
        shutil.copyfile(benchmark / name, work / name)
    shutil.copyfile(Path(__file__).with_name('select_oracle.cpp'), work / 'select_oracle.cpp')
    save_json(work / 'controls.json', headers)
    return headers


def main():
    parser = argparse.ArgumentParser(description='Broad API controls and extended select controls.')
    parser.add_argument('--repo', type=Path, default=Path.cwd())
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--repeats', type=int, default=5)
    parser.add_argument('--api', action='store_true', help='Reuse the existing 52-workload API runner.')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--check-recorded', type=Path, help='Compare generated header hashes with select_screen.json.')
    args = parser.parse_args()
    repo, work = args.repo.resolve(), args.work.resolve()
    if work == repo or any((work / p).exists() for p in ['results.json', 'controls.json']):
        parser.error('--work must be a fresh private directory')
    work.mkdir(parents=True, exist_ok=True)
    headers = prepare(repo, work)
    if args.check_recorded:
        expected = json.loads(args.check_recorded.read_text())['manifest']['headers']
        assert headers == expected, 'Generated controls differ from the recorded sources'
    if args.prepare_only:
        print(json.dumps(headers, indent=2))
        return
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        if args.api:
            original = repo / 'test/benchmark/dwm_avx_final'
            copied = work / 'scan/test/benchmark/dwm_avx_final'
            copied.mkdir(parents=True, exist_ok=True)
            for name in ['bench.cpp', 'escape.cpp', 'vector_oracle.cpp']:
                shutil.copyfile(original / name, copied / name)
            expected = {p: headers['scan'][p] for p in [
                'components/dynamic_quad_vector.hpp', 'components/dynamic_quad_wavelet.hpp',
                'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp']}
            save_json(work / 'api-header-hashes.json', expected)
            env = dict(os.environ)
            env['GIT_DIR'] = subprocess.check_output(
                ['git', 'rev-parse', '--absolute-git-dir'], cwd=repo, text=True).strip()
            subprocess.run([sys.executable, repo / 'test/benchmark/dynamic_wavelet_quaternary/run_api.py',
                            '--repo', work / 'scan', '--work', work / 'api-output',
                            '--baseline', BASE, '--header-hashes', work / 'api-header-hashes.json'],
                           env=env, check=True)
            if args.output:
                save_json(args.output, json.loads((work / 'api-output/results.json').read_text()))
            return
        manifest = {'utc': datetime.now(timezone.utc).isoformat(), 'base_commit': BASE,
                    'headers': headers, 'flags': FLAGS, 'cpu': args.cpu,
                    'lock': '/tmp/cp-wm-opt.lock', 'repeats': args.repeats,
                    'compiler': subprocess.check_output(['g++', '--version'], text=True),
                    'compiler_sha256': sha(Path(shutil.which('g++')).resolve()),
                    'helper_sha256': sha(__file__), 'helper_oracle_sha256': sha(work / 'select_oracle.cpp'),
                    'sources': {p: sha(work / p) for p in ['bench.cpp', 'escape.cpp']},
                    'commands': [], 'binaries': {}}
        result = {'manifest': manifest, 'checks': [], 'cases': [], 'complete': False}

        def record():
            save_json(work / 'results.json', result)

        def run(command):
            command = list(map(str, command))
            begin = time.monotonic()
            process = subprocess.run(command, capture_output=True, text=True, timeout=180)
            manifest['commands'].append({'command': command, 'returncode': process.returncode,
                                         'seconds': time.monotonic() - begin})
            if process.returncode:
                (work / 'failure.log').write_text(process.stdout + process.stderr)
                record()
                process.check_returncode()
            return process.stdout.strip()

        for label, compiler, extra in [
                ('native', 'g++', ['-O2', '-march=native']),
                ('scalar-sanitize', 'clang++', ['-O1', '-g', '-DDYNAMIC_QUAD_DISABLE_SIMD',
                                              '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
            binary = work / ('oracle-' + label)
            run([compiler, '-std=c++2b', '-Wall', '-Wextra', '-Werror', *extra,
                 '-I' + str(work / 'binary'), work / 'select_oracle.cpp', '-o', binary])
            run([binary])
            result['checks'].append({'name': label, 'status': 'pass', 'binary_sha256': sha(binary)})
            record()
        run(['g++', *FLAGS, '-c', work / 'escape.cpp', '-o', work / 'escape.o'])
        for domain, value_type, bits in DOMAINS:
            for arm in headers:
                binary = work / (arm + '-' + domain)
                run(['g++', *FLAGS, '-DVALUE_TYPE=' + value_type, '-DBITS=' + str(bits),
                     '-I' + str(work / arm), work / 'bench.cpp', work / 'escape.o', '-o', binary])
                manifest['binaries'][arm + '-' + domain] = sha(binary)
        rng = random.Random(20261021)
        for domain, kind, distribution, q in WORKLOADS:
            case = {'domain': domain, 'kind': kind, 'distribution': distribution,
                    'n': 100000, 'q': q, 'samples': {p: [] for p in headers}}
            truth = None
            for trial in range(args.repeats):
                arms = list(headers)
                rng.shuffle(arms)
                for arm in arms:
                    dump = work / ('dump-' + domain + '-' + kind + '-' + arm + '-' + str(trial))
                    answer = json.loads(run([work / (arm + '-' + domain), kind, 100000, q,
                                             20261020, distribution, 'random', 1, 'warm', dump, 'bulk']))
                    input_path = Path(str(dump) + '.input')
                    hashes = (sha(input_path), sha(dump), answer['hash'], answer['finalhash'])
                    if truth is not None:
                        assert hashes == truth, (domain, kind, arm, trial)
                    truth = hashes
                    case['samples'][arm].append({'trial': trial, **answer})
                    if not (trial == 0 and arm == 'base'):
                        input_path.unlink()
                        dump.unlink()
            case['hashes'] = truth
            case['summary'] = {}
            for metric in ['cpu', 'wall', 'buildcpu', 'memory']:
                case['summary'][metric] = {
                    arm: {'median': statistics.median(s[metric] for s in samples),
                          'range': [min(s[metric] for s in samples), max(s[metric] for s in samples)]}
                    for arm, samples in case['samples'].items()}
            values = case['summary']['cpu']
            case['binary_over_base'] = values['binary']['median'] / values['base']['median']
            case['binary_over_scan'] = values['binary']['median'] / values['scan']['median']
            result['cases'].append(case)
            record()
            print(json.dumps({'domain': domain, 'kind': kind, 'cpu': values}), flush=True)
        result['complete'] = True
        result['interpretation'] = ('Operation-only CPU/wall; construction recorded separately. '
                                    'Full input/output/final-state hashes match across every arm and trial. '
                                    'Extended select controls use full bulk build, not the final compact build.')
        record()
        if args.output:
            save_json(args.output, result)


if __name__ == '__main__':
    main()
