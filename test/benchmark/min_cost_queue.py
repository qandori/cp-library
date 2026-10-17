import argparse
from datetime import datetime, timezone
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HEADERS = ['graph/min_cost_flow.hpp', 'components/flow_graph.hpp',
           'components/min_cost_circulation.hpp', 'components/d_ary_heap.hpp',
           'components/RadixHeap.hpp']
NAMES = ['random_positive', 'zero_90_percent', 'all_zero', 'cost_0_to_3', 'dag', 'parallel_two_vertices']


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-ref', default='6af78c247f5c9beba109116966f3a3d4e9dc5104')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--min-cpu-ms', type=float, default=200)
    parser.add_argument('--small-cpu-ms', type=float, default=500)
    args = parser.parse_args()
    if args.trials < 1 or args.min_cpu_ms <= 0 or args.small_cpu_ms <= 0:
        parser.error('Trials and CPU durations must be positive.')
    if args.cpu >= 0:
        if not hasattr(os, 'sched_setaffinity'):
            parser.error('CPU affinity is unavailable; use --cpu -1 to run unpinned.')
        os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    source = ROOT / 'test/benchmark/min_cost_queue.cpp'
    flags = {
        'generic': ['-std=c++2b', '-O2', '-DNDEBUG'],
        'native': ['-std=c++2b', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero'],
    }
    cases = [(config, api, mode) for config in flags for api in range(2) for mode in range(6)]
    result = {
        'started_at': datetime.now(timezone.utc).isoformat(),
        'baseline_ref': args.baseline_ref,
        'candidate_head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'compiler': subprocess.check_output([args.cxx, '--version'], text=True).splitlines()[0],
        'platform': platform.platform(), 'cpu_model': next((line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')), platform.processor()) if Path('/proc/cpuinfo').exists() else platform.processor(),
        'cpu_affinity': args.cpu, 'seed': 36683669,
        'trials': args.trials, 'min_cpu_ms': args.min_cpu_ms, 'small_cpu_ms': args.small_cpu_ms,
        'flags': flags, 'source_sha256': digest(source), 'runner_sha256': digest(Path(__file__)),
        'candidate_headers': {name: digest(ROOT / name) for name in HEADERS},
        'cases_declared': cases, 'workloads': NAMES,
        'metric': 'CPU and wall time for graph copy and solve; fixture generation excluded',
        'order': '(within_pair + trial + mode + api) % 2; baseline is 0',
        'results': {},
    }
    manifest = args.output / 'results.json'
    manifest.write_text(json.dumps(result, indent=2) + '\n')
    with tempfile.TemporaryDirectory(prefix='min-cost-queue-') as temporary:
        temporary = Path(temporary)
        baseline = temporary / 'baseline'
        baseline.mkdir()
        archive = subprocess.check_output(['git', 'archive', args.baseline_ref, *HEADERS], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as files:
            files.extractall(baseline, filter='data')
        result['baseline_headers'] = {name: digest(baseline / name) for name in HEADERS}
        binaries = {}
        for config, options in flags.items():
            binary = temporary / config
            command = [args.cxx, *options, '-I', str(ROOT),
                       '-DBASELINE_MIN_COST_FLOW_HEADER=' + json.dumps(str(baseline / 'graph/min_cost_flow.hpp')),
                       str(source), '-o', str(binary)]
            subprocess.run(command, check=True)
            binaries[config] = binary
        for config, api, mode in cases:
            name = f'{config}_{"public" if api else "scaling"}_{NAMES[mode]}'
            minimum = max(args.min_cpu_ms, args.small_cpu_ms) if mode == 5 else args.min_cpu_ms
            command = [str(binaries[config]), str(mode), str(api), str(args.trials), str(minimum)]
            process = subprocess.run(command, text=True, capture_output=True)
            (args.output / f'{name}.stdout').write_text(process.stdout)
            (args.output / f'{name}.stderr').write_text(process.stderr)
            process.check_returncode()
            rows = []
            for line in process.stdout.splitlines():
                trial, variant, repeats, checksum, wall, cpu = line.split()
                rows.append({'trial': int(trial), 'variant': int(variant), 'repeats': int(repeats),
                             'checksum_per_solve': checksum, 'wall_ms': float(wall), 'cpu_ms': float(cpu)})
            assert len(rows) == 2 * args.trials
            assert all(row['cpu_ms'] >= minimum for row in rows)
            speedups = []
            for trial in range(args.trials):
                before = next(row for row in rows if row['trial'] == trial and row['variant'] == 0)
                after = next(row for row in rows if row['trial'] == trial and row['variant'] == 1)
                assert before['checksum_per_solve'] == after['checksum_per_solve']
                speedups.append((before['cpu_ms'] / before['repeats']) / (after['cpu_ms'] / after['repeats']))
            medians = {variant: {metric: statistics.median(row[metric] / row['repeats'] for row in rows if row['variant'] == variant)
                                 for metric in ['cpu_ms', 'wall_ms']} for variant in [0, 1]}
            result['results'][name] = {'rows': rows, 'medians_ms_per_solve': medians,
                                       'paired_speedups': speedups, 'median_paired_speedup': statistics.median(speedups)}
            manifest.write_text(json.dumps(result, indent=2) + '\n')
            print(name, medians, 'SPEEDUP', statistics.median(speedups), flush=True)
        assert result['candidate_headers'] == {name: digest(ROOT / name) for name in HEADERS}
        assert result['source_sha256'] == digest(source)
    result['finished_at'] = datetime.now(timezone.utc).isoformat()
    manifest.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
