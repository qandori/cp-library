import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    binary = args.work / 'bench'
    command = ['g++', '-std=c++23', '-O2', '-march=native', '-Wall', '-Wextra', '-Werror', '-I', '.',
               'test/benchmark/dsu_on_tree.cpp', '-o', str(binary)]
    subprocess.run(command, check=True)
    record = dict(utc=datetime.now(timezone.utc).isoformat(), compiler=subprocess.check_output(['g++', '--version'], text=True),
                  lscpu=subprocess.check_output(['lscpu'], text=True), cpu=args.cpu, command=command,
                  sources={p: sha(Path(p)) for p in ['tree/dsu_on_tree.hpp', 'components/undirected_csr.hpp',
                                                    'test/benchmark/dsu_on_tree.cpp', 'test/benchmark/dsu_on_tree.py']},
                  binary_sha256=sha(binary), timing='one warmup; shuffled process order; median process CPU; no input/output in build/run timers', cases=[])
    rng = random.Random(20261021)
    for n, shapes, repeats in [(200000, ['path', 'star', 'balanced', 'broom', 'random'], 5),
                               (2500000, ['path', 'star', 'random'], 3)]:
        for shape in shapes:
            values = {method: [] for method in ['linear', 'stack', 'clear']}
            warmups = {}
            for trial in range(repeats + 1):
                methods = list(values)
                rng.shuffle(methods)
                for method in methods:
                    start = time.perf_counter()
                    process = subprocess.run([str(binary), str(n), shape, method], capture_output=True, text=True, check=True, timeout=60)
                    row = json.loads(process.stdout)
                    row['process_wall'] = time.perf_counter() - start
                    if trial:
                        values[method].append(row)
                    else:
                        warmups[method] = row
            checksums = {r['checksum'] for items in values.values() for r in items}
            assert len(checksums) == 1
            assert len({r['additions'] for items in values.values() for r in items}) == 1
            summary = {method: {field: statistics.median(r[field] for r in rows)
                                for field in ['build_cpu', 'run_cpu', 'process_wall', 'rss_kib']} for method, rows in values.items()}
            record['cases'].append(dict(n=n, shape=shape, trials=values, warmups=warmups, summary=summary))
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(json.dumps(record, indent=2) + '\n')
            print(n, shape, {m: round(1000 * r['run_cpu'], 3) for m, r in summary.items()}, flush=True)


if __name__ == '__main__':
    main()
