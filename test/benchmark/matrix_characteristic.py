import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--trials', type=int, default=5)
    parser.add_argument('--seconds', type=float, default=0.15)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    paths = ['math/matrix.hpp', 'components/matrix_arithmetic.hpp', 'math/mint.hpp',
             'test/benchmark/matrix_characteristic.cpp', 'test/benchmark/matrix_characteristic.py']
    metadata = {'baseline': str(args.baseline.resolve()), 'compiler': subprocess.check_output(['g++', '--version'], text=True),
                'cpu': Path('/proc/cpuinfo').read_text(), 'affinity': [args.cpu], 'trials': args.trials,
                'seconds': args.seconds, 'seed': 914771,
                'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    metadata['baseline_sha256'] = {p: hashlib.sha256((args.baseline / p).read_bytes()).hexdigest()
                                  for p in paths[:2]}
    cases = [('mint', n, 'dense') for n in (0, 1, 8, 15, 16, 17, 64, 128, 256, 512)]
    cases += [('mint', n, 'zero') for n in (64, 512)]
    cases += [('mint', 128, kind) for kind in ('triangular', 'hessenberg', 'sparse', 'cycle', 'max')]
    cases += [('mod31', 128, 'dense'), ('mod31', 512, 'dense'), ('mod2', 128, 'dense')]
    cases += [('generic', n, 'dense') for n in (16, 64)]
    rows, summaries = [], []
    for mode in ('generic', 'native'):
        binary = args.output / mode
        flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
        if mode == 'native':
            flags += ['-march=native', '-ftrivial-auto-var-init=zero']
        command = ['g++', *flags, '-I', str(ROOT),
                   '-DMATRIX_BASELINE_HEADER="' + str(args.baseline.resolve() / 'math/matrix.hpp') + '"',
                   str(ROOT / 'test/benchmark/matrix_characteristic.cpp'), '-o', str(binary)]
        subprocess.run(command, check=True)
        metadata[mode + '_flags'] = flags

        def run(case, repetitions, trial):
            field, n, kind = case
            lines = subprocess.check_output([str(binary), field, str(n), kind, str(repetitions), '914771', str(trial)], text=True).splitlines()
            samples = {}
            for line in lines:
                variant, cpu, wall, checksum = line.split()
                samples[variant] = {'cpu': float(cpu), 'wall': float(wall), 'checksum': checksum}
            assert samples['baseline']['checksum'] == samples['candidate']['checksum'], case
            return samples

        for case in cases:
            repetitions = 1
            while True:
                sample = run(case, repetitions, 0)
                fastest = min(v['cpu'] for v in sample.values())
                if fastest >= args.seconds * 1.2:
                    break
                repetitions *= max(2, math.ceil(args.seconds * 1.4 / max(fastest, 0.000001)))
            cpu_ratios, wall_ratios = [], []
            valid = True
            for trial in range(args.trials):
                sample = run(case, repetitions, trial)
                cpu_ratios.append(sample['baseline']['cpu'] / sample['candidate']['cpu'])
                wall_ratios.append(sample['baseline']['wall'] / sample['candidate']['wall'])
                valid &= all(v['cpu'] >= args.seconds for v in sample.values())
                rows.append({'mode': mode, 'case': case, 'trial': trial, 'repetitions': repetitions, 'samples': sample})
            row = {'mode': mode, 'case': case, 'cpu_ratio': statistics.median(cpu_ratios),
                   'wall_ratio': statistics.median(wall_ratios), 'duration_valid': valid, 'repetitions': repetitions}
            summaries.append(row)
            print(json.dumps(row), flush=True)
            (args.output / 'samples.json').write_text(json.dumps(rows, indent=2) + '\n')
    (args.output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    (args.output / 'summary.json').write_text(json.dumps(summaries, indent=2) + '\n')


if __name__ == '__main__':
    main()
