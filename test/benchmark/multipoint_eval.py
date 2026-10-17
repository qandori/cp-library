import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '117fda44639532534bc91fb02517a1cba09ee738'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--grid', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--pairs', type=int, default=8)
    parser.add_argument('--seconds', type=float, default=0.15)
    args = parser.parse_args()
    if args.pairs < 3 or args.seconds < 0.15:
        parser.error('use at least three pairs and 0.15 seconds per variant')
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    source = subprocess.check_output(['git', 'show', BASELINE + ':fps/multipoint_eval.hpp'], cwd=ROOT)
    baseline = source.decode().replace('CP_FPS_MULTIPOINT_EVAL_HPP', 'BASELINE_MULTIPOINT_EVAL_HPP')
    baseline = baseline.replace('multipoint_eval', 'baseline_multipoint_eval')
    baseline = baseline.replace('"fps.hpp"', '"fps/fps.hpp"')
    baseline = baseline.replace('"../components/fps_product_tree.hpp"', '"components/fps_product_tree.hpp"')
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-Wall', '-Wextra']
    if args.native:
        flags += ['-march=native']
    paths = ['fps/multipoint_eval.hpp', 'components/fps_direct_eval.hpp', 'components/ntt.hpp',
             'components/fps_product_tree.hpp', 'components/fps_operations.hpp', 'fps/fps.hpp',
             'math/mint.hpp', 'test/benchmark/multipoint_eval.cpp', 'test/benchmark/multipoint_eval.py']
    cpu_info = subprocess.run(['lscpu'], text=True, capture_output=True)
    environment = {'baseline': BASELINE, 'baseline_sha256': hashlib.sha256(source).hexdigest(),
                   'compiler': subprocess.check_output(['g++', '--version'], text=True),
                   'cpu': cpu_info.stdout, 'lscpu_error': cpu_info.stderr,
                   'proc_cpuinfo': Path('/proc/cpuinfo').read_text() if Path('/proc/cpuinfo').exists() else '',
                   'flags': flags, 'affinity': [args.cpu], 'pairs': args.pairs, 'seconds': args.seconds,
                   'seed': 731, 'grid': args.grid, 'sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths}}
    (args.output / 'environment.json').write_text(json.dumps(environment, indent=2) + '\n')
    with tempfile.TemporaryDirectory(prefix='cp-multipoint-') as directory:
        tmp = Path(directory)
        header = tmp / 'baseline.hpp'
        header.write_text(baseline)
        binary = tmp / 'benchmark'
        subprocess.run(['g++', *flags, '-I', str(ROOT), f'-DMULTIPOINT_BASELINE="{header}"',
                        str(ROOT / 'test/benchmark/multipoint_eval.cpp'), '-o', str(binary)], check=True)
        with (args.output / 'measurements.csv').open('w') as output:
            subprocess.run([str(binary), str(args.pairs), str(args.seconds), 'grid' if args.grid else 'final'], stdout=output, check=True)


if __name__ == '__main__':
    main()
