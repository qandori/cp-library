import argparse
import json
import hashlib
import pathlib
import statistics
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--baseline', default='e476d2233f097421c14ef2d768559d5d8e2b1837')
parser.add_argument('--compiler', default='g++')
parser.add_argument('--seconds', type=float, default=0.1)
parser.add_argument('--pairs', type=int, default=9)
parser.add_argument('--native', action='store_true')
parser.add_argument('--uniform-bounds', action='store_true')
parser.add_argument('--candidate-first', action='store_true')
args = parser.parse_args()
if args.seconds <= 0 or args.pairs < 1:
    parser.error('seconds and pairs must be positive')
root = pathlib.Path(__file__).resolve().parents[2]
cases = [(n, m, o, 0) for n in (1024, 200000, 1000000)
         for m in (0, 1, 2) for o in (1, 4, 5)]
cases += [(200000, m, o, 0) for m in (0, 1, 2) for o in (7, 8)]
cases += [(200000, m, o, 0) for m in (6, 7, 8) for o in (1, 7, 8)]
cases += [(200000, 0, o, 0) for o in (0, 2, 3, 6, 9, 10, 11, 12, 13)]
cases += [(200000, m, o, length)
          for m, length in ((0, 8), (1, 8), (2, 8), (3, 0), (4, 0), (5, 0))
          for o in (4, 5)]
with tempfile.TemporaryDirectory(prefix='wavelet-neighbors-') as directory:
    temporary = pathlib.Path(directory)
    for name in ('data_structure/wavelet_matrix.hpp', 'components/quad_bit_vector.hpp'):
        destination = temporary / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(subprocess.check_output(
            ['git', 'show', f'{args.baseline}:{name}'], cwd=root))
    if (temporary / 'components/quad_bit_vector.hpp').read_bytes() != (root / 'components/quad_bit_vector.hpp').read_bytes():
        raise RuntimeError('this paired benchmark requires an unchanged QuadBitVector')
    binary = temporary / 'benchmark'
    command = [args.compiler, '-std=c++23', '-O2', f'-I{root}',
               f'-DWAVELET_BASELINE_HEADER="{temporary / "data_structure/wavelet_matrix.hpp"}"']
    if args.native:
        command.append('-march=native')
    if args.candidate_first:
        command.append('-DWAVELET_CANDIDATE_FIRST')
    command += [str(root / 'test/benchmark/wavelet_matrix_neighbors.cpp'), '-o', str(binary)]
    subprocess.run(command, check=True)
    print(json.dumps({'metadata': {
        'baseline': subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=root, text=True).strip(),
        'candidate_sha256': hashlib.sha256((root / 'data_structure/wavelet_matrix.hpp').read_bytes()).hexdigest(),
        'compiler': subprocess.check_output([args.compiler, '--version'], text=True).splitlines()[0],
        'native': args.native, 'candidate_first': args.candidate_first,
        'seconds': args.seconds, 'pairs': args.pairs, 'uniform_bounds': args.uniform_bounds}}), flush=True)
    for n, mode, operation, length in cases:
        raw = subprocess.check_output(
            [str(binary), str(n), str(mode), str(operation), str(length),
             str(args.seconds), str(args.pairs)] + (['uniform'] if args.uniform_bounds else []), text=True)
        pairs = {}
        for line in raw.splitlines():
            pair, variant, cpu, wall, checksum, repetitions = line.split()
            pairs.setdefault(int(pair), {})[int(variant)] = {
                'cpu': float(cpu), 'wall': float(wall),
                'checksum': checksum, 'repetitions': int(repetitions)}
        for pair in pairs.values():
            if pair[0]['checksum'] != pair[1]['checksum']:
                raise RuntimeError('baseline and candidate checksums differ')
        ratios = [pair[0]['cpu'] / pair[1]['cpu'] for pair in pairs.values()]
        print(json.dumps({'n': n, 'mode': mode, 'operation': operation, 'length': length,
                          'median': statistics.median(ratios),
                          'minimum': min(ratios), 'maximum': max(ratios),
                          'pairs': pairs}), flush=True)
