import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import re
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
MOD = 998244353


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--tune', action='store_true')
    args = parser.parse_args()
    os.chdir(ROOT)
    os.sched_setaffinity(0, {2})
    cache = ROOT / '.verify-helper/cache/online-convolution-benchmark'
    cache.mkdir(parents=True, exist_ok=True)
    args.output.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++2b', '-O2', '-DNDEBUG']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    source = ROOT / 'test/benchmark/online_convolution.cpp'
    original = (ROOT / 'convolution/online_convolution.hpp').read_text()
    threshold = int(re.search(r'd < (\d+)', original)[1])
    definitions = {'online': [], 'hitona': ['-DBENCH_HITONA']}
    if args.tune:
        for candidate in [2, 4, 16, 32, 64]:
            header = cache / f'threshold_{candidate}.hpp'
            header.write_text(re.sub(r'd < \d+', f'd < {candidate}', original).replace(
                '"../components/ntt.hpp"', '"' + str(ROOT / 'components/ntt.hpp') + '"'))
            definitions[f'threshold_{candidate}'] = [f'-DBENCH_ONLINE_HEADER="{header}"']
    binaries = {}
    for name, extra in definitions.items():
        binary = cache / name
        subprocess.run(['g++', *flags, *extra, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        binaries[name] = str(binary)
    reference = ROOT / '.verify-helper/cache/online_convolution_reference/409662.cpp'
    if not args.tune:
        binary = cache / 'reference_409662'
        extra = [] if args.native else ['-march=haswell']
        subprocess.run(['g++', *flags, *extra, str(reference), '-o', str(binary)], check=True)
        binaries['reference_409662'] = str(binary)
    rng = random.Random(682341)
    result = {'platform': platform.platform(),
              'cpu': Path('/proc/cpuinfo').read_text().split('model name')[1].split('\n')[0].strip(': \t'),
              'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              'flags': flags, 'reference_extra_flags': [] if args.native else ['-march=haswell'],
              'direct_threshold': threshold, 'cpu_affinity': [2], 'trials': 7, 'seed': 682341,
              'source_hash_normalization': 'CRLF to LF',
              'metric': 'wall time including startup, input, construction, computation, output and exit',
              'sources': {name: sha(ROOT / name) for name in ['test/benchmark/online_convolution.cpp',
                  'convolution/online_convolution.hpp', 'components/ntt.hpp', 'math/mint.hpp',
                  'components/fast_io.hpp']}, 'workloads': {}}
    for n in ([131072] if args.tune else [8192, 131072, 500000]):
        a = [rng.randrange(MOD) for _ in range(n)]
        b = [rng.randrange(MOD) for _ in range(n)]
        for mode in ['pairs', 'feedback', 'exp']:
            values = [0] + a[1:] if mode == 'exp' else a
            data = (str(n) + '\n' + ' '.join(map(str, values)) + '\n' +
                    ('' if mode == 'exp' else ' '.join(map(str, b)) + '\n')).encode()
            commands = {name: [binary, mode] for name, binary in binaries.items()}
            if mode != 'exp':
                commands.pop('reference_409662', None)
            samples = {name: [] for name in commands}
            expected = None
            for name, command in commands.items():
                output = b' '.join(subprocess.check_output(command, input=data, timeout=60).split())
                if expected is None:
                    expected = output
                assert output == expected, (n, mode, name)
            schedule = list(commands) * 7
            rng.shuffle(schedule)
            for name in schedule:
                start = time.perf_counter()
                output = subprocess.check_output(commands[name], input=data, timeout=60)
                samples[name].append((time.perf_counter() - start) * 1000)
                assert b' '.join(output.split()) == expected, (n, mode, name)
            item = {'n': n, 'input_sha256': hashlib.sha256(data).hexdigest(),
                    'output_tokens_sha256': hashlib.sha256(expected).hexdigest(),
                    'programs': {name: {'milliseconds': times, 'median_ms': statistics.median(times)}
                                 for name, times in samples.items()}}
            result['workloads'][f'{mode}_{n}'] = item
            print(n, mode, {name: round(value['median_ms'], 3) for name, value in item['programs'].items()}, flush=True)
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
