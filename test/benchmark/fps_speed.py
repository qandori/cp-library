import argparse
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '82fe5f034fca2805aeb59aee1be56216493ea5de'
PROBLEMS = dict(inv='inv_of_formal_power_series', exp='exp_of_formal_power_series',
                log='log_of_formal_power_series', sqrt='sqrt_of_formal_power_series',
                pow='pow_of_formal_power_series')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--target', choices=['haswell', 'native'], default='haswell')
    parser.add_argument('--sizes', nargs='+', type=int, default=[32768, 131073, 500000, 524288])
    parser.add_argument('--operations', nargs='+', default=['inv', 'log', 'exp', 'sqrt', 'pow', 'div_series', 'divmod', 'square', 'multiply',
                                                         'inv:1:1', 'inv:12:1', 'inv:32:1', 'exp:1:1', 'exp:24:1', 'exp:64:1', 'inv:32:1024', 'exp:64:1024'])
    parser.add_argument('--candidates', action='store_true')
    parser.add_argument('--official-root', type=Path)
    parser.add_argument('--references', type=Path)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=' + args.target]
    baseline = subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip()
    result = dict(environment=dict(baseline=baseline, flags=flags, trials=args.trials, cpu=args.cpu,
                                   compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                                   hardware=subprocess.check_output(['lscpu'], text=True), seed=20261018,
                                   harness_sha256=sha(Path(__file__)), driver_sha256=sha(ROOT / 'test/benchmark/fps_tuning_basic.cpp')),
                  sources={}, kernel=[], judge=[])
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def save():
        args.output.write_text(json.dumps(result, indent=2) + '\n')

    with open('/tmp/cp-fps-tuning.lock', 'w') as lock, tempfile.TemporaryDirectory(prefix='fps-speed-') as temp:
        fcntl.flock(lock, fcntl.LOCK_EX)
        work = Path(temp)
        before, current = work / 'before', work / 'current'
        data = subprocess.check_output(['git', 'archive', baseline, 'fps', 'components', 'math', 'convolution'], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(data)) as archive:
            archive.extractall(before, filter='data')
        for folder in ['fps', 'components', 'math', 'convolution']:
            shutil.copytree(ROOT / folder, current / folder)
        trees = dict(before=before, current=current)
        if args.candidates:
            for count in [1, 2, 4]:
                tree = work / f'group{count}'
                shutil.copytree(current, tree)
                header = tree / 'components/ntt.hpp'
                s = header.read_text()
                begin = s.index('void inverse_newton8')
                end = s.index('\n\ttemplate <int n>', begin)
                body = s[begin:end].replace('i += 64', f'i += {8 * count}').replace('j < 8;', f'j < {count};')
                body = body.replace('inverse_leaf_products8<8>', f'inverse_leaf_products8<{count}>')
                body = body.replace('inverse_leaf_products8<8, true>', f'inverse_leaf_products8<{count}, true>')
                header.write_text(s[:begin] + body + s[end:])
                trees[f'group{count}'] = tree
        binaries = {}
        for name, tree in trees.items():
            result['sources'][name] = {p.relative_to(tree).as_posix(): sha(p) for p in tree.rglob('*.hpp')}
            binary = work / name
            binary = binary.with_suffix('.out')
            subprocess.run(['g++', *flags, '-I', str(tree), str(ROOT / 'test/benchmark/fps_tuning_basic.cpp'), '-o', str(binary)], check=True)
            binaries[name] = binary
            print('built', name, flush=True)
        rng = random.Random(20261018)
        for n in args.sizes:
            for op in args.operations:
                samples = {name: [] for name in binaries}
                checksums = set()
                for trial in range(args.trials + 1):
                    order = list(binaries)
                    rng.shuffle(order)
                    for name in order:
                        raw = subprocess.check_output([str(binaries[name]), op, str(n)], text=True).split()
                        checksums.add(raw[1])
                        if trial:
                            samples[name].append(float(raw[0]) * 1000)
                if len(checksums) != 1:
                    raise RuntimeError((op, n, checksums))
                row = dict(operation=op, size=n, checksum=next(iter(checksums)), samples_ms=samples,
                           median_ms={name: statistics.median(values) for name, values in samples.items()})
                result['kernel'].append(row)
                print('kernel', op, n, row['median_ms'], flush=True)
                save()
        if args.official_root:
            official = args.official_root.resolve()
            folders = {p.parent.name: p.parent for p in official.glob('*/*/info.toml')}
            metadata = json.loads((ROOT / 'test/benchmark/results/fps_speed_reference.json').read_text())
            old_io = work / 'current_old_io'
            shutil.copytree(current, old_io)
            shutil.copy2(before / 'components/fast_io.hpp', old_io / 'components/fast_io.hpp')
            result['sources']['current_old_io'] = {p.relative_to(old_io).as_posix(): sha(p) for p in old_io.rglob('*.hpp')}
            result['environment']['official_commit'] = subprocess.check_output(['git', '-C', str(official), 'rev-parse', 'HEAD'], text=True).strip()
            for op, problem in PROBLEMS.items():
                if op not in args.operations:
                    continue
                folder = folders[problem]
                inputs = [p for p in (folder / 'in').glob('*.in') if p.stem.startswith('max_random')]
                if op == 'sqrt':
                    inputs = [p for p in inputs if not (folder / 'out' / (p.stem + '.out')).read_text().startswith('-1')]
                if op == 'pow':
                    inputs = list((folder / 'in').glob('binary_exp_max*.in'))
                inp = max(inputs, key=lambda p: p.stat().st_size)
                answer = folder / 'out' / (inp.stem + '.out')
                drivers = {}
                for name, tree in [('before', before), ('current_old_io', old_io), ('current', current)]:
                    (tree / 'test').mkdir(exist_ok=True)
                    for f in [f'fps_{op}.test.cpp', 'fps_io.hpp']:
                        shutil.copy2(ROOT / 'test' / f, tree / 'test' / f)
                    binary = work / f'judge_{name}_{op}'
                    subprocess.run(['g++', *flags, '-I', str(tree), str(tree / f'test/fps_{op}.test.cpp'), '-o', str(binary)], check=True)
                    drivers[name] = binary
                if args.references:
                    for reference in metadata['problems'][problem]['references'][:2]:
                        source = args.references.resolve() / problem / (str(reference['id']) + '.cpp')
                        if sha(source) != reference['source_sha256']:
                            raise RuntimeError(f'Reference source changed: {source}')
                        name = 'reference_' + str(reference['id'])
                        binary = work / name
                        subprocess.run(['g++', *flags, str(source), '-o', str(binary)], check=True)
                        drivers[name] = binary
                samples = {name: [] for name in drivers}
                for trial in range(args.trials + 1):
                    order = list(drivers)
                    rng.shuffle(order)
                    for name in order:
                        out = work / 'actual.txt'
                        with inp.open('rb') as stdin, out.open('wb') as stdout:
                            begin = time.perf_counter()
                            subprocess.run([str(drivers[name])], stdin=stdin, stdout=stdout, check=True)
                            elapsed = (time.perf_counter() - begin) * 1000
                        subprocess.run([str(folder / 'checker'), str(inp), str(out), str(answer)], check=True,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        if trial:
                            samples[name].append(elapsed)
                row = dict(operation=op, input=inp.name, input_sha256=sha(inp), samples_ms=samples,
                           median_ms={name: statistics.median(values) for name, values in samples.items()})
                result['judge'].append(row)
                print('judge', op, inp.name, row['median_ms'], flush=True)
                save()
    save()


if __name__ == '__main__':
    main()
