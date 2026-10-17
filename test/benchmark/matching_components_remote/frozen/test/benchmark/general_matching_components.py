import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('out', type=Path)
parser.add_argument('--stage', choices=['pilot', 'final'], default='final')
args = parser.parse_args()
out = args.out.resolve()
out.mkdir(parents=True, exist_ok=True)
root = Path(__file__).resolve().parents[2]
base = '8c38d74c07610dfd1a5fb13b1beeb18313ce1e04'
source = subprocess.check_output(['git', 'show', base + ':graph/general_matching.hpp'], cwd=root)
assert hashlib.sha256(source).hexdigest() == '62c215ab4bf9d24bcffc8da5defb9e5b187e69975276e78413d05f965e0007be'
(out / 'baseline.hpp').write_bytes(source.replace(b'GENERAL_MATCHING_HPP', b'MATCHING_COMPONENT_BASELINE_HPP').replace(b'GeneralMatching', b'BaselineMatching'))
metadata = {'baseline_commit': base, 'stage': args.stage, 'cpu': 0, 'compiler': subprocess.check_output(['g++', '--version'], text=True), 'cpuinfo': Path('/proc/cpuinfo').read_text(), 'hashes': {}}
for file in [root / 'graph/general_matching.hpp', root / 'test/benchmark/general_matching_components.cpp', out / 'baseline.hpp']:
    metadata['hashes'][file.name] = hashlib.sha256(file.read_bytes()).hexdigest()
for mode, flags in [('generic', []), ('native', ['-march=native'])]:
    binary = out / (args.stage + '-' + mode + '.bin')
    command = ['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-fmax-errors=3', *flags, '-iquote', str(root), '-iquote', str(out), str(root / 'test/benchmark/general_matching_components.cpp'), '-o', str(binary)]
    metadata[mode] = command
    subprocess.run(command, check=True)
    with (out / (args.stage + '-' + mode + '.csv')).open('w') as stdout, (out / (args.stage + '-' + mode + '.log')).open('w') as stderr:
        subprocess.run(['taskset', '-c', '0', str(binary), args.stage], stdout=stdout, stderr=stderr, check=True)
(out / (args.stage + '-environment.json')).write_text(json.dumps(metadata, indent=2))
