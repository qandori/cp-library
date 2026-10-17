import argparse
import csv
import hashlib
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    sources = {
        'Baseline': root / 'test/benchmark/rolling_hash_powers_baseline.hpp',
        'Candidate': root / 'string/rolling_hash.hpp',
    }
    manifest = []
    for name, path in sources.items():
        raw = path.read_text()
        manifest.append(f'{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.relative_to(root)}')
        raw = raw.replace('ROLLING_HASH_HPP', name.upper() + '_ROLLING_HASH_HPP').replace('RollingHash', name)
        raw = raw.replace('#include "../components/splitmix64.hpp"', '#include "' + str(root / 'components/splitmix64.hpp') + '"')
        raw = raw.replace('  private:', '  public:')
        raw, count = re.subn(r'\tstatic Hash base\(\) \{.*?\n\t\}',
                            '\tstatic Hash base() {\n\t\tstatic const Hash value = rolling_benchmark_base;\n\t\treturn value;\n\t}',
                            raw, count=1, flags=re.S)
        if count != 1:
            raise RuntimeError('Could not replace exactly one base function')
        raw = raw.replace('class ' + name, 'extern std::uint64_t rolling_benchmark_base;\n\nclass ' + name, 1)
        (output / (name + '.hpp')).write_text(raw)
    shutil.copyfile(root / 'test/benchmark/rolling_hash_powers.cpp', output / 'screen.cpp')
    (output / 'source-sha256.txt').write_text('\n'.join(manifest) + '\n')
    if args.prepare_only:
        return
    cpu = args.cpu if args.cpu is not None else min(os.sched_getaffinity(0))
    prefix = ['taskset', '-c', str(cpu)]
    environment = subprocess.check_output([args.cxx, '--version'], text=True)
    environment += '\n' + str(os.uname()) + f'\nPinned CPU: {cpu}\n'
    environment += Path('/proc/cpuinfo').read_text().split('\n\n', 1)[0] + '\n'
    (output / 'environment.txt').write_text(environment)
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror']
    subprocess.run([args.cxx, *flags, 'screen.cpp', '-o', 'verify'], cwd=output, check=True)
    with (output / 'correctness.log').open('w') as log:
        for base in [256, 257, 12345678901234567, 2305843009213693694]:
            subprocess.run([*prefix, './verify', 'verify', '0', '0', '0', '0', str(base)], cwd=output, stdout=log, check=True)
    cases = [(1000000, 0), (8000000, 0), (16, 1), (64, 1), (10000, 1),
             (1000000, 1), (8000000, 1), (64, 2), (1000000, 2), (1000000, 3)]
    summary = []
    for arch in ['generic', 'native']:
        extra = ['-march=native'] if arch == 'native' else []
        executable = 'screen-' + arch
        subprocess.run([args.cxx, *flags, '-DNDEBUG', *extra, 'screen.cpp', '-o', executable], cwd=output, check=True)
        for case_index, (n, mode) in enumerate(cases):
            path = output / f'{arch}-{n}-{mode}.csv'
            with path.open('w') as log:
                subprocess.run([*prefix, './' + executable, str(n), str(mode), '10', '.50', str(case_index % 2),
                                '12345678901234567'], cwd=output, stdout=log, check=True)
            rows = list(csv.DictReader(path.open()))
            if len(rows) != 10:
                raise RuntimeError('Incomplete sample set: ' + str(path))
            ratios = [float(row['ratio']) for row in rows]
            minimum = min(float(row[variant + '_cpu']) for row in rows for variant in ['baseline', 'candidate'])
            summary.append([path.name, n, mode, len(rows),
                            statistics.median(float(row['baseline_cpu']) / int(row['reps']) * 1000 for row in rows),
                            statistics.median(float(row['candidate_cpu']) / int(row['reps']) * 1000 for row in rows),
                            statistics.median(ratios), min(ratios), max(ratios), minimum,
                            'valid-duration' if minimum >= .10 else 'INCONCLUSIVE-short-sample'])
    with (output / 'summary.csv').open('w') as log:
        writer = csv.writer(log)
        writer.writerow(['file', 'n', 'mode', 'pairs', 'baseline_ms', 'candidate_ms', 'median_ratio', 'min_ratio',
                         'max_ratio', 'min_cpu_seconds', 'duration_status'])
        writer.writerows(summary)


if __name__ == '__main__':
    main()
