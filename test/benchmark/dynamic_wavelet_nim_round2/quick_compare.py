#!/usr/bin/env python3
"""Compare frozen API candidates with both supplied Nim compiler controls."""
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


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def cpu_now():
    usage = resource.getrusage(resource.RUSAGE_CHILDREN)
    return usage.ru_utime + usage.ru_stime


def save(path, value):
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


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--repo', type=Path, default=Path.cwd())
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=3)
    parser.add_argument('--resume', action='store_true')
    args = parser.parse_args()
    repo, cache = args.repo.resolve(), args.cache.resolve()
    if args.output.exists() and not args.resume:
        raise RuntimeError('Refusing to replace an existing record')
    screen_path = repo / 'test/benchmark/results/dynamic_wavelet_nim_round2/screen1.json'
    screen = json.loads(screen_path.read_text())
    nim_path = cache / 'reference/preparation.json'
    nim = json.loads(nim_path.read_text())
    binaries, sources = {}, {}
    for arm in ['before', 'full8192']:
        root = cache / 'screen1' / arm
        sources[arm] = {str(p.relative_to(root)): sha(p) for p in root.rglob('*.hpp')}
        assert sources[arm] == screen['sources'][arm], (arm, 'frozen source mismatch')
        binaries['cpp_' + arm] = {'path': str(root / 'solver'), 'sha256': sha(root / 'solver'),
                                 'source': arm, 'semantics': 'signed int32 / 32 bits, FastInput/FastOutput, kth_largest(0), prev_value, count'}
    for variant in ['official_api', 'optimized_api']:
        binary = nim['binaries'][variant]
        assert sha(binary['path']) == binary['sha256']
        binaries['nim_' + variant] = {'path': binary['path'], 'sha256': binary['sha256'],
                                      'command': nim['commands'][variant], 'backend_build': binary['backend_build'],
                                      'semantics': nim['semantics']}
    for name, expected in [('Supplied.nim', nim['source_sha256']), ('Instrumented.nim', nim['api_source_sha256'])]:
        assert sha(cache / 'reference' / name) == expected
    cases = json.loads((cache / 'cases-large/manifest.json').read_text())
    for case in cases:
        assert sha(case['input']['path']) == case['input']['sha256']
        assert sha(case['expected']['path']) == case['expected']['sha256']
    result = {'metadata': {'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                           'helper_sha256': sha(__file__), 'cpp_screen_manifest_sha256': sha(screen_path),
                           'cpp_flags': screen['flags'], 'cpp_driver_sha256': screen['driver_sha256'],
                           'cpp_baseline': screen['baseline'], 'cpp_candidate': 'full8192 selection candidate, not applied to production',
                           'sources': sources, 'binaries': binaries, 'nim_preparation_sha256': sha(nim_path),
                           'nim_source_sha256': nim['source_sha256'], 'nim_api_source_sha256': nim['api_source_sha256'],
                           'nim_library_commit': nim['library_commit'], 'nim_license': nim['license'],
                           'nim_version': nim['nim_version'], 'nim_sha256': nim['nim_sha256'],
                           'nim_optimization_difference': {'official': nim['official'], 'optimized': nim['optimized']},
                           'compiler': subprocess.check_output(['g++', '--version'], text=True),
                           'compiler_sha256': sha(shutil.which('g++')), 'uname': list(os.uname()),
                           'lscpu': subprocess.check_output(['lscpu'], text=True), 'cpu': 2, 'seed': 20261020,
                           'repeats': args.repeats, 'warmups': 1,
                           'timing': 'exclusive shared task lock, CPU2, shuffled order, blocking subprocess wait, regular file stdin/stdout, process CPU RUSAGE_CHILDREN; API timers exclude input/loading/output'},
              'cases': cases, 'warmups': [], 'samples': [], 'summary': {}}
    if args.resume:
        result = json.loads(args.output.read_text())
        assert result['metadata']['repeats'] == args.repeats
        assert result['metadata']['sources'] == sources
        assert result['metadata']['binaries'] == binaries
        result['metadata'].setdefault('resumed', []).append({
            'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'helper_sha256': sha(__file__)})
    complete = {(r['case'], r['variant'], r['trial']) for r in result['warmups'] + result['samples']}
    output_file = cache / 'quick-output.txt'
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        rng = random.Random(20261020)
        rng.shuffle(cases)
        sequence = 0
        for case in cases:
            for trial in range(-1, args.repeats):
                order = list(binaries)
                rng.shuffle(order)
                for name in order:
                    if (case['name'], name, trial) in complete:
                        sequence += 1
                        continue
                    with open(case['input']['path'], 'rb') as source, output_file.open('wb') as destination:
                        start_wall, start_cpu = time.perf_counter_ns(), cpu_now()
                        process = subprocess.run([binaries[name]['path']], stdin=source, stdout=destination,
                                                 stderr=subprocess.PIPE, check=True)
                        cpu, wall = cpu_now() - start_cpu, (time.perf_counter_ns() - start_wall) / 1e9
                    assert sha(output_file) == case['expected']['sha256'], (case['name'], name, 'wrong answer')
                    timers = json.loads(process.stderr)
                    assert timers['answers'] == case['stats']['query']
                    timers['total_cpu'] = timers['build_cpu'] + timers['operation_cpu']
                    row = {'case': case['name'], 'variant': name, 'trial': trial, 'sequence': sequence,
                           'wall_seconds': wall, 'cpu_seconds': cpu, 'api': timers,
                           'answer_sha256': case['expected']['sha256']}
                    result['warmups' if trial == -1 else 'samples'].append(row)
                    sequence += 1
                save(args.output, result)
            summary = {}
            for name in binaries:
                rows = [r for r in result['samples'] if r['case'] == case['name'] and r['variant'] == name]
                summary[name] = {field: statistics.median(r['api'][field] for r in rows)
                                 for field in ['build_cpu', 'operation_cpu', 'total_cpu']}
                summary[name]['process_wall'] = statistics.median(r['wall_seconds'] for r in rows)
            result['summary'][case['name']] = summary
            save(args.output, result)
            print(case['name'], {name: round(v['operation_cpu'] * 1000, 3) for name, v in summary.items()}, flush=True)


if __name__ == '__main__':
    main()
