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
import time

ROOT = Path(__file__).resolve().parents[2]


def canonical(values):
    row = {}
    for x in values:
        while x:
            p = x.bit_length() - 1
            if p not in row:
                row[p] = x
                break
            x ^= row[p]
    for p in sorted(row):
        for q in row:
            if q > p and (row[q] >> p) & 1:
                row[q] ^= row[p]
    return tuple(row[p] for p in sorted(row))


def change_basis(values, rng):
    rng.shuffle(values)
    if len(values) > 1:
        for _ in range(len(values) * 3):
            i, j = rng.sample(range(len(values)), 2)
            values[i] ^= values[j]
    return values


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--references', type=Path, default=ROOT / 'test/benchmark/results/xor_basis_reference.json')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=7)
    parser.add_argument('--cases', type=int, default=24000)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    metadata = json.loads(args.references.read_text())
    reference = metadata['problems']['intersection_of_f2_vector_spaces']['references'][0]
    reference_source = args.cache / (str(reference['id']) + '.cpp')
    assert hashlib.sha256(reference_source.read_bytes()).hexdigest() == reference['source_sha256']
    paths = ['math/xor_basis.hpp', 'components/xor_basis_core.hpp', 'components/fast_io.hpp',
             'test/xor_basis_intersection.test.cpp', 'test/benchmark/xor_basis_judge.py']
    env = {'compiler': subprocess.check_output(['g++', '--version'], text=True), 'flags': flags, 'reference': reference,
           'cpu': subprocess.check_output(['lscpu'], text=True), 'uname': subprocess.check_output(['uname', '-a'], text=True),
           'seed': 713, 'affinity': [args.cpu], 'trials': args.trials, 'cases': args.cases,
           'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    rng = random.Random(713)
    lines = [str(args.cases)]
    expected = []
    groups = {name: 0 for name in ['empty', 'full', 'near_full', 'random', 'sparse', 'equal']}
    for t in range(args.cases):
        name = list(groups)[t % len(groups)]
        groups[name] += 1
        columns = [(1 << i) | rng.getrandbits(i) for i in range(30)]
        permutation = list(range(30))
        rng.shuffle(permutation)
        columns = [sum(((x >> i) & 1) << permutation[i] for i in range(30)) for x in columns]
        n, m = {'empty': (0, 17), 'full': (30, 22), 'near_full': (29, 29), 'random': (rng.randrange(31), rng.randrange(31)),
                'sparse': (4, 7), 'equal': (15, 15)}[name]
        a = set(rng.sample(range(30), n))
        b = a.copy() if name == 'equal' else set(rng.sample(range(30), m))
        x = change_basis([columns[i] for i in sorted(a)], rng)
        y = change_basis([columns[i] for i in sorted(b)], rng)
        lines.append(' '.join(map(str, [len(x), *x])))
        lines.append(' '.join(map(str, [len(y), *y])))
        expected.append(canonical([columns[i] for i in sorted(a & b)]))
    data = ('\n'.join(lines) + '\n').encode()
    env['groups'] = groups
    env['input_sha256'] = hashlib.sha256(data).hexdigest()
    env['canonical_output_sha256'] = hashlib.sha256(repr(expected).encode()).hexdigest()
    rows = []
    with tempfile.TemporaryDirectory(prefix='cp-xor-basis-judge-') as directory:
        tmp = Path(directory)
        input_path = tmp / 'input.txt'
        input_path.write_bytes(data)
        binaries = {}
        for variant, source in [('library', ROOT / 'test/xor_basis_intersection.test.cpp'), ('reference', reference_source)]:
            binary = tmp / variant
            subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
            binaries[variant] = binary
        for trial in range(args.trials):
            order = ['library', 'reference']
            rng.shuffle(order)
            for variant in order:
                with input_path.open('rb') as file:
                    start = time.perf_counter()
                    output = subprocess.check_output([str(binaries[variant])], stdin=file)
                    elapsed = time.perf_counter() - start
                answers = output.splitlines()
                assert len(answers) == args.cases
                for line, answer in zip(answers, expected):
                    values = list(map(int, line.split()))
                    assert values[0] == len(values) - 1 == len(answer)
                    assert canonical(values[1:]) == answer
                rows.append({'variant': variant, 'trial': trial, 'seconds': elapsed, 'output_sha256': hashlib.sha256(output).hexdigest()})
                print(trial, variant, f'{elapsed:.6f}', 'all subspaces match', flush=True)
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2) + '\n')
    summary = [{'variant': variant, 'seconds': statistics.median(r['seconds'] for r in rows if r['variant'] == variant)}
               for variant in ['library', 'reference']]
    for name, data in [('samples.csv', rows), ('summary.csv', summary)]:
        with (args.output / name).open('w', newline='') as file:
            writer = csv.DictWriter(file, fieldnames=data[0].keys())
            writer.writeheader()
            writer.writerows(data)


if __name__ == '__main__':
    main()
