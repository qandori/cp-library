import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    paths = ['math/xor_basis.hpp', 'components/xor_basis_core.hpp', 'test/benchmark/xor_basis.cpp', 'test/benchmark/xor_basis.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'seed': 713, 'trials': args.trials, 'affinity': [args.cpu],
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    rows = []
    rng = random.Random(7919)
    with tempfile.TemporaryDirectory(prefix='cp-xor-basis-') as directory:
        binary = Path(directory) / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(ROOT / paths[2]), '-o', str(binary)], check=True)
        for operation, count in [('build', 10000), ('query', 200000)]:
            for rank in [8, 32, 63, 64]:
                expected = None
                for trial in range(args.trials):
                    order = ['scanning', 'compact', 'library']
                    rng.shuffle(order)
                    for variant in order:
                        elapsed, result = subprocess.check_output([str(binary), variant, operation, str(rank), str(count)]).splitlines()
                        if expected is None:
                            expected = result
                        assert result == expected, (operation, rank, variant)
                        rows.append({'operation': operation, 'rank': rank, 'count': count, 'variant': variant,
                                     'trial': trial, 'seconds': float(elapsed), 'checksum': result.decode()})
                print(operation, rank, 'checksums match', flush=True)
    summary = [{'operation': op, 'rank': rank, 'variant': variant,
                'seconds': statistics.median(r['seconds'] for r in rows if (r['operation'], r['rank'], r['variant']) == (op, rank, variant))}
               for op in ['build', 'query'] for rank in [8, 32, 63, 64] for variant in ['scanning', 'compact', 'library']]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
