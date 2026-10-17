import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SYMBOLS = ['PolynomialHalfGCD', 'PolynomialExtendedGCD', 'polynomial_gcd', 'polynomial_extended_gcd', 'polynomial_inv_mod']
BASELINE = '6af78c247f5c9beba109116966f3a3d4e9dc5104'
OLD_BASE = '\t\t\twhile (int(b.size()) > m) {\n\t\t\t\tauto [q, r] = a.divmod(b);\n\t\t\t\tstep(result, q), a = std::move(b), b = std::move(r);\n\t\t\t}\n'
FUSED_BASE = '\t\t\twhile (int(b.size()) > m) {\n\t\t\t\tint length = int(b.size()), degree = int(a.size()) - length;\n\t\t\t\tT inverse = b.back().inv();\n\t\t\t\tfor (int column = 0; column < 2; ++column) {\n\t\t\t\t\tauto& top = result.a[column];\n\t\t\t\t\tconst auto& bottom = result.a[column + 2];\n\t\t\t\t\tif (!bottom.empty()) top.resize(std::max(top.size(), bottom.size() + degree));\n\t\t\t\t}\n\t\t\t\tfor (int i = degree; i >= 0; --i) {\n\t\t\t\t\tT q = a[i + length - 1] * inverse;\n\t\t\t\t\tfor (int j = 0; j + 1 < length; ++j) a[i + j] -= q * b[j];\n\t\t\t\t\tfor (int column = 0; column < 2; ++column) {\n\t\t\t\t\t\tauto& top = result.a[column];\n\t\t\t\t\t\tconst auto& bottom = result.a[column + 2];\n\t\t\t\t\t\tfor (int j = 0; j < int(bottom.size()); ++j) top[i + j] -= q * bottom[j];\n\t\t\t\t\t}\n\t\t\t\t}\n\t\t\t\ta.resize(length - 1), O::trim(a), a.swap(b);\n\t\t\t\tO::trim(result.a[0]), O::trim(result.a[1]);\n\t\t\t\tresult.a[0].swap(result.a[2]), result.a[1].swap(result.a[3]);\n\t\t\t\tresult.shift += degree, result.identity = false;\n\t\t\t}\n'


def sha(source):
    return hashlib.sha256(source.encode()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--trials', type=int, default=6)
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--minimum-cpu', type=float, default=0.15)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    if args.minimum_cpu < 0.15:
        parser.error('--minimum-cpu must be at least 0.15')
    args.output.mkdir(parents=True, exist_ok=True)
    paths = ['components/polynomial_half_gcd.hpp', 'fps/polynomial_gcd.hpp']
    baseline = [subprocess.check_output(['git', 'show', f'{args.baseline}:{p}'], cwd=ROOT, text=True) for p in paths]
    candidate = [(ROOT / p).read_text() for p in paths]
    if candidate[0].count(OLD_BASE) != 1:
        raise RuntimeError('Expected original Euclid base in candidate; fused experiment must remain attributable')
    fused = [candidate[0].replace(OLD_BASE, FUSED_BASE), candidate[1]]
    variants = [('Base', baseline), ('Column', candidate), ('FusedColumn', fused)]
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    sources = paths + ['fps/fps.hpp', 'components/fps_operations.hpp', 'components/ntt.hpp', 'math/mint.hpp',
                       'test/benchmark/polynomial_inv_mod.cpp', 'test/benchmark/polynomial_inv_mod.py']
    cpu_info = subprocess.run(['lscpu'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    environment = {'baseline': args.baseline, 'flags': flags, 'cpu': args.cpu, 'minimum_cpu': args.minimum_cpu,
                   'compiler': subprocess.check_output([args.cxx, '--version'], text=True), 'platform': platform.platform(),
                   'lscpu': {'stdout': cpu_info.stdout, 'stderr': cpu_info.stderr, 'returncode': cpu_info.returncode},
                   'proc_cpuinfo': Path('/proc/cpuinfo').read_text() if Path('/proc/cpuinfo').exists() else None,
                   'available_affinity': sorted(os.sched_getaffinity(0)), 'seed': 294384, 'trials': args.trials,
                   'source_sha256': {p: sha((ROOT / p).read_text()) for p in sources},
                   'variants': {name: dict(zip(paths, map(sha, texts))) for name, texts in variants},
                   'order': 'rotate [0,1,2] by floor(trial/2)%3, reverse odd trials', 'samples': []}
    with tempfile.TemporaryDirectory(prefix='polynomial-inverse-') as directory:
        temporary = Path(directory)
        includes = ''
        for name, texts in variants:
            half = temporary / f'{name}_half.hpp'
            public = temporary / f'{name}_gcd.hpp'
            half.write_text(texts[0].replace('"../fps/fps.hpp"', f'"{ROOT / "fps/fps.hpp"}"'))
            public.write_text(texts[1].replace('"fps.hpp"', f'"{ROOT / "fps/fps.hpp"}"')
                              .replace('"../components/polynomial_half_gcd.hpp"', f'"{half}"'))
            includes += '\n#undef CP_COMPONENTS_POLYNOMIAL_HALF_GCD_HPP\n#undef CP_FPS_POLYNOMIAL_GCD_HPP\n'
            includes += ''.join(f'#define {symbol} {name}_{symbol}\n' for symbol in SYMBOLS)
            includes += f'#include "{public}"\n'
            includes += ''.join(f'#undef {symbol}\n' for symbol in SYMBOLS)
        header = temporary / 'variants.hpp'
        header.write_text(includes)
        binary = temporary / 'benchmark'
        command = [args.cxx, *flags, f'-DCP_POLYNOMIAL_GCD_VARIANTS="{header}"',
                   str(ROOT / 'test/benchmark/polynomial_inv_mod.cpp'), '-o', str(binary)]
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (args.output / 'build.log').write_text(result.stdout)
        result.check_returncode()
        cases = [(n, 2, 0) for n in [1, 32, 128, 129, 256, 1024, 4096, 50000, 65537]]
        cases += [(4096, 2, 4), (4096, 2, 5)] + [(50000, 2, mode) for mode in [1, 2, 3]]
        cases += [(n, op, 0) for n in [4096, 50000] for op in [0, 1]]
        for n, op, mode in cases:
            raw = subprocess.check_output([str(binary), str(n), str(op), str(mode), str(args.trials),
                                           str(args.cpu), str(args.minimum_cpu)], text=True)
            samples = []
            for line in raw.splitlines():
                values = line.split(',')
                row = dict(zip(['n', 'op', 'mode', 'trial', 'variant', 'count'], map(int, values[:6])))
                row.update(cpu_seconds=float(values[6]), milliseconds=float(values[7]), checksum=values[8])
                if row['cpu_seconds'] < args.minimum_cpu:
                    raise RuntimeError('CPU measurement floor not met')
                samples.append(row)
            if len(samples) != 3 * args.trials or len({r['checksum'] for r in samples}) != 1:
                raise RuntimeError('Incomplete or inconsistent samples')
            environment['samples'] += samples
            (args.output / 'results.json').write_text(json.dumps(environment, indent=2) + '\n')
            median = [statistics.median(r['milliseconds'] for r in samples if r['variant'] == v) for v in range(3)]
            print(n, op, mode, 'median milliseconds', median, flush=True)
    summary = []
    for n, op, mode in cases:
        selected = [r for r in environment['samples'] if (r['n'], r['op'], r['mode']) == (n, op, mode)]
        median = [statistics.median(r['milliseconds'] for r in selected if r['variant'] == v) for v in range(3)]
        for variant in [1, 2]:
            paired = []
            for trial in range(args.trials):
                pair = {r['variant']: r['milliseconds'] for r in selected if r['trial'] == trial}
                paired.append(pair[0] / pair[variant])
            summary.append({'n': n, 'op': op, 'mode': mode, 'variant': variant,
                            'baseline_ms': median[0], 'candidate_ms': median[variant],
                            'ratio_of_medians': median[0] / median[variant],
                            'paired_speedup_median': statistics.median(paired),
                            'paired_speedup_min': min(paired), 'paired_speedup_max': max(paired)})
    with (args.output / 'summary.csv').open('w') as file:
        writer = csv.DictWriter(file, fieldnames=summary[0].keys())
        writer.writeheader()
        writer.writerows(summary)


if __name__ == '__main__':
    main()
