import argparse
from datetime import datetime, timezone
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
BEFORE = 'b584f748d67d38faf22229cb2c971ccd98ba7d6e'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def measure(binaries, inp, answer, checker, work, trials, rng):
    samples = {name: [] for name in binaries}
    hashes = {}
    for trial in range(-1, trials):
        order = list(binaries)
        rng.shuffle(order)
        for name in order:
            out = work / 'output'
            with inp.open('rb') as a, out.open('wb') as b:
                start = time.perf_counter()
                subprocess.run([binaries[name]], stdin=a, stdout=b, check=True)
                ms = (time.perf_counter() - start) * 1000
            digest = sha(out)
            if name not in hashes or digest != hashes[name]:
                subprocess.run([str(checker), str(inp), str(out), str(answer)], check=True,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                hashes[name] = digest
            if trial >= 0:
                samples[name].append(ms)
    return dict(input_sha256=sha(inp), answer_sha256=sha(answer),
                variants={name: dict(samples_ms=values, median_ms=statistics.median(values), output_sha256=hashes[name]) for name, values in samples.items()})


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--references', type=Path, required=True)
    parser.add_argument('--official', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native', '-ftrivial-auto-var-init=zero']
    sources = ['components/predecessor_set.hpp', 'data_structure/sortable_segtree.hpp',
               'components/fast_io.hpp', 'test/predecessor_set.test.cpp', 'test/sortable_segtree.test.cpp',
               'test/benchmark/fast_set.cpp', 'test/benchmark/fast_set.py']
    environment = dict(date=datetime.now(timezone.utc).isoformat(), flags=flags,
                       compiler=subprocess.check_output(['g++', '--version'], text=True),
                       cpu=subprocess.check_output(['lscpu'], text=True), affinity=[args.cpu], before=BEFORE,
                       trials=args.trials, warmup=1, seed=403023, order_seed=403023,
                       sha256={p: sha(ROOT / p) for p in sources},
                       references={str(i): sha(args.references / (str(i) + '.cpp')) for i in [403023, 278227]},
                       official_commit=subprocess.check_output(['git', '-C', str(args.official.parents[1]), 'rev-parse', 'HEAD'], text=True).strip(),
                       method='Kernel excludes generation and I/O; judge uses fresh processes and regular files in Linux /tmp including I/O and exit; checker and SHA-256 outside timing')
    rng = random.Random(403023)
    results = dict(kernel={}, judge={}, sortable={})
    with tempfile.TemporaryDirectory(prefix='cp-fast-set-', dir='/tmp') as directory:
        work = Path(directory)
        old = subprocess.check_output(['git', 'show', BEFORE + ':components/predecessor_set.hpp'], cwd=ROOT, text=True)
        environment['before_sha256'] = hashlib.sha256(old.encode()).hexdigest()
        (work / 'old.hpp').write_text(old)
        (work / 'before.hpp').write_text(old.replace('PREDECESSOR_SET_HPP', 'BEFORE_PREDECESSOR_SET_HPP').replace('PredecessorSet', 'BeforePredecessorSet'))
        scalar = (ROOT / 'components/predecessor_set.hpp').read_text().replace('PREDECESSOR_SET_HPP', 'SCALAR_PREDECESSOR_SET_HPP')
        scalar = scalar.replace('predecessor_set_detail', 'predecessor_set_scalar_detail').replace('PredecessorSet', 'ScalarPredecessorSet')
        scalar = scalar.replace('__builtin_cpu_supports("avx2")', 'false')
        (work / 'scalar.hpp').write_text(scalar)
        driver = (ROOT / 'test/predecessor_set.test.cpp').read_text().replace('"../components/fast_io.hpp"', '"components/fast_io.hpp"')
        previous = driver.replace('"../components/predecessor_set.hpp"', '"old.hpp"').replace('PredecessorSet set(in.token());', '''auto bits = in.token();
\tPredecessorSet set(n);
\tfor (int i = 0; i < n; ++i) if (bits[i] == '1') set.insert(i);''')
        previous = previous.replace('set.set(p, type == 0)', '(type == 0 ? set.insert(p) : set.erase(p))')
        (work / 'old.cpp').write_text(previous)
        (work / 'scalar.cpp').write_text(driver.replace('"../components/predecessor_set.hpp"', '"scalar.hpp"\nusing PredecessorSet = ScalarPredecessorSet;'))
        source_map = dict(current=ROOT / 'test/predecessor_set.test.cpp', before=work / 'old.cpp', scalar=work / 'scalar.cpp')
        source_map.update({str(i): args.references / (str(i) + '.cpp') for i in [403023, 278227]})
        binaries = {}
        for name, source in source_map.items():
            binaries[name] = work / name
            subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(work), str(source), '-o', str(binaries[name])], check=True)
        subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(work), str(ROOT / 'test/benchmark/fast_set.cpp'), '-o', str(work / 'kernel')], check=True)
        sortable = args.official.parent / 'point_set_range_sort_range_composite'
        header = (ROOT / 'data_structure/sortable_segtree.hpp').read_text().replace('"../components/predecessor_set.hpp"', '"old.hpp"')
        (work / 'old_sortable.hpp').write_text(header)
        driver = (ROOT / 'test/sortable_segtree.test.cpp').read_text().replace('"../components/fast_io.hpp"', '"components/fast_io.hpp"')
        (work / 'old_sortable.cpp').write_text(driver.replace('"../data_structure/sortable_segtree.hpp"', '"old_sortable.hpp"'))
        sort_binaries = dict(current=work / 'sort_current', before=work / 'sort_before')
        for name, source in [('current', ROOT / 'test/sortable_segtree.test.cpp'), ('before', work / 'old_sortable.cpp')]:
            subprocess.run(['g++', *flags, '-I', str(ROOT), '-I', str(work), str(source), '-o', str(sort_binaries[name])], check=True)
        environment['generated_sha256'] = {p.name: sha(p) for p in work.iterdir() if p.suffix in ['.hpp', '.cpp']}
        for pattern, kind in enumerate(['random', 'all1', 'empty_search', 'small_search']):
            samples = {name: [] for name in ['before', 'current', 'scalar']}
            expected = None
            for trial in range(-1, args.trials):
                order = list(samples)
                rng.shuffle(order)
                for name in order:
                    row = json.loads(subprocess.check_output([work / 'kernel', str(list(samples).index(name)), str(pattern)], text=True))
                    digest = row['checksum'], row['count']
                    if expected is None:
                        expected = digest
                    if expected != digest:
                        raise RuntimeError((kind, name, expected, digest))
                    if trial >= 0:
                        samples[name].append(row)
            results['kernel'][kind] = dict(samples=samples, medians={name: {key: statistics.median(row[key] for row in rows) for key in ['build_ms', 'query_ms']} for name, rows in samples.items()})
            print('kernel', kind, results['kernel'][kind]['medians'], flush=True)
        for inp in sorted((args.official / 'in').glob('*.in')):
            target = work / 'input'
            shutil.copyfile(inp, target)
            row = measure(binaries, target, args.official / 'out' / (inp.stem + '.out'), args.official / 'checker', work, args.trials, rng)
            results['judge'][inp.stem] = row
            print('judge', inp.stem, {k: v['median_ms'] for k, v in row['variants'].items()}, flush=True)
        for stem in ['max_random_00', 'many_1_query_00', 'many_2_query_00']:
            target = work / 'input'
            shutil.copyfile(sortable / 'in' / (stem + '.in'), target)
            row = measure(sort_binaries, target, sortable / 'out' / (stem + '.out'), sortable / 'checker', work, args.trials, rng)
            results['sortable'][stem] = row
            print('sortable', stem, {k: v['median_ms'] for k, v in row['variants'].items()}, flush=True)
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')


if __name__ == '__main__':
    main()
