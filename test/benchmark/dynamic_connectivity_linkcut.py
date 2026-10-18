import argparse
from contextlib import contextmanager
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
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '2be6469cbc4f7864e7920c98ac1a38450f0770eb'
HEADER = 'data_structure/dynamic_connectivity.hpp'
DRIVER = 'test/dynamic_connectivity.test.cpp'
KERNEL = 'test/benchmark/dynamic_connectivity_linkcut.cpp'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checksum(path):
    values = [int(x) for x in path.read_bytes().split()]
    value = len(values)
    for x in values:
        value = ((value ^ x) * 0x9e3779b97f4a7c15) & ((1 << 64) - 1)
    return str(value)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--problem', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/graph/dynamic_graph_vertex_add_component_sum')
    parser.add_argument('--reference-cache', type=Path, default=Path('.verify-helper/cache/dc-linkcut/reference'))
    parser.add_argument('--trials', type=int, default=9)
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--only', nargs='*')
    parser.add_argument('--generic', action='store_true')
    parser.add_argument('--kernel-only', action='store_true')
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    args.output.mkdir(parents=True, exist_ok=True)
    lock = open('/tmp/cp-dc-linkcut.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    flags = ['-std=gnu++23', '-O2', '-ftrivial-auto-var-init=zero', '-DATCODER', '-DONLINE_JUDGE']
    if not args.generic:
        flags.append('-march=native')
    frozen = [HEADER, DRIVER, KERNEL, 'components/fast_io.hpp', 'components/splitmix64.hpp', 'test/benchmark/dynamic_connectivity_linkcut.py']
    environment = dict(measured_at=datetime.now(timezone.utc).isoformat(), baseline=BASELINE, flags=flags,
        compiler=subprocess.check_output(['g++', '--version'], text=True), cpu=subprocess.check_output(['lscpu'], text=True),
        kernel=subprocess.check_output(['uname', '-a'], text=True), affinity=[args.cpu], trials=args.trials, warmup=1,
        order_seed=20261018, hash_seeds=[713 + i * 101 for i in range(args.trials)],
        sha256={path: sha(ROOT / path) for path in frozen},
        method='exclusive lock; fresh processes; regular-file I/O on Linux /tmp; kernel excludes parsing and checksum, includes construction/reserve/online API calls and answer collection; whole includes startup/I/O/exit; checker outside timing')
    results = dict(kernel={}, whole={})
    rng = random.Random(20261018)
    before = subprocess.check_output(['git', 'show', BASELINE + ':' + HEADER], cwd=ROOT, text=True)
    old_driver = subprocess.check_output(['git', 'show', BASELINE + ':' + DRIVER], cwd=ROOT, text=True)
    current = (ROOT / HEADER).read_text()
    fast_driver = (ROOT / DRIVER).read_text()
    cases = sorted((args.problem / 'in').glob('*.in'))
    if args.only:
        cases = [p for p in cases if p.stem in args.only]
    if not cases:
        raise RuntimeError('No official cases')
    with tempfile.TemporaryDirectory(prefix='cp-dc-linkcut-') as temporary:
        work = Path(temporary)
        binaries = dict(kernel={}, whole={})
        for name, header in [('before', before), ('current', current)]:
            header = header.replace('../components/splitmix64.hpp', 'components/splitmix64.hpp')
            header = header.replace('std::chrono::steady_clock::now().time_since_epoch().count()', 'dc_benchmark_seed')
            path = work / (name + '.hpp')
            path.write_text(header)
            with exclusive():
                binary = work / (name + '_kernel')
                subprocess.run(['g++', *flags, '-I', str(ROOT), '-DDC_BENCH_HEADER="' + str(path) + '"', str(ROOT / KERNEL), '-o', str(binary)], check=True)
                binaries['kernel'][name] = binary
            if args.kernel_only:
                continue
            for io, driver in [('stdio', old_driver), ('fastio', fast_driver)]:
                driver = driver.replace('../data_structure/dynamic_connectivity.hpp', str(path))
                driver = driver.replace('../components/fast_io.hpp', 'components/fast_io.hpp')
                driver = driver.replace('int main() {', 'int main(int argc, char **argv) {\n\tif (argc > 1) dc_benchmark_seed = std::stoull(argv[1]);')
                driver = '#include <cstdint>\n#include <string>\ninline std::uint64_t dc_benchmark_seed=713;\n' + driver
                source = work / (name + '_' + io + '.cpp')
                source.write_text(driver)
                with exclusive():
                    binary = work / (name + '_' + io)
                    subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
                    binaries['whole'][name + '_' + io] = binary
        if not args.kernel_only:
            reference = args.reference_cache / '402761.cpp'
            metadata = json.loads((ROOT / 'test/benchmark/results/dynamic_connectivity_linkcut_reference.json').read_text())
            row = next(r for r in metadata['references'] if r['id'] == 402761)
            assert sha(reference) == row['source_sha256']
            environment['reference'] = dict(id=402761, source_sha256=sha(reference), offline=True)
            with exclusive():
                binary = work / 'reference_offline'
                subprocess.run(['g++', *flags, str(reference), '-o', str(binary)], check=True)
                binaries['whole']['reference_offline'] = binary
        for case in cases:
            inp = work / 'input'
            shutil.copyfile(case, inp)
            answer = args.problem / 'out' / (case.stem + '.out')
            expected = checksum(answer)
            record = dict(input_sha256=sha(inp), answer_sha256=sha(answer), checksum=expected)
            for kind in ['kernel', 'whole']:
                if not binaries[kind]:
                    continue
                samples = {name: [] for name in binaries[kind]}
                hashes = {}
                with exclusive():
                    for trial in range(-1, args.trials):
                        order = list(samples)
                        rng.shuffle(order)
                        for name in order:
                            binary = binaries[kind][name]
                            seed = str(713 + 101 * trial)
                            if kind == 'kernel':
                                with inp.open('rb') as stream:
                                    result = json.loads(subprocess.check_output([binary, seed], stdin=stream))
                                assert result['checksum'] == expected, (case.name, name, result)
                                ms = result['ms']
                            else:
                                output = work / 'output'
                                with inp.open('rb') as stream, output.open('wb') as destination:
                                    start = time.perf_counter()
                                    subprocess.run([binary, seed], stdin=stream, stdout=destination, check=True)
                                    ms = (time.perf_counter() - start) * 1000
                                digest = sha(output)
                                if hashes.get(name) != digest:
                                    subprocess.run([args.problem / 'checker', inp, output, answer], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                                    hashes[name] = digest
                            if trial >= 0:
                                samples[name].append(ms)
                results[kind][case.stem] = dict(**record, samples_ms=samples, median_ms={name: statistics.median(values) for name, values in samples.items()}, output_sha256=hashes)
                print(kind, case.stem, results[kind][case.stem]['median_ms'], flush=True)
            (args.output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
        for path, digest in environment['sha256'].items():
            assert sha(ROOT / path) == digest, path
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    print('All measurements passed', flush=True)


if __name__ == '__main__':
    main()
