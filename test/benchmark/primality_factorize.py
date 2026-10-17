import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxx', default='g++-14')
    parser.add_argument('--cpu', type=int)
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--rounds', type=int, default=15)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror']
    if args.native:
        flags += ['-march=native', '-ftrivial-auto-var-init=zero']
    cases = [('small', 100000), ('random32', 100000), ('random64', 100000),
             ('odd32', 50000), ('odd64', 50000), ('prime32', 100000), ('prime64', 30000)]
    with tempfile.TemporaryDirectory() as directory:
        binary = str(Path(directory) / 'benchmark')
        subprocess.run([args.cxx, *flags, '-I', str(root),
                        str(root / 'test/benchmark/primality_factorize.cpp'), '-o', binary], check=True)
        prefix = [] if args.cpu is None else ['taskset', '-c', str(args.cpu)]
        print('case,count,before_ms,after_ms,before_checksum,after_checksum', flush=True)
        for case, count in cases:
            subprocess.run([*prefix, binary, case, str(count), str(args.rounds)], check=True)


if __name__ == '__main__':
    main()
