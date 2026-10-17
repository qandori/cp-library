import argparse
import io
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import tarfile

root = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('--base', default='6af78c247f5c9beba109116966f3a3d4e9dc5104')
p.add_argument('--output', type=Path, required=True)
p.add_argument('--cxx', default='g++')
p.add_argument('--trials', type=int, default=7)
p.add_argument('--cpu', type=int, default=0)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
base = a.output / 'baseline'
base.mkdir(exist_ok=True)
archive = subprocess.check_output(['git', 'archive', a.base], cwd=root)
with tarfile.open(fileobj=io.BytesIO(archive)) as t:
    t.extractall(base, filter='data')
seen = set()
def dependencies(path):
    for name in re.findall(r'^#include "([^"]+)"', path.read_text(), re.M):
        child = (path.parent / name).resolve()
        relative = child.relative_to(root.resolve())
        if relative in seen: continue
        seen.add(relative)
        if child.read_bytes() != (base / relative).read_bytes():
            raise RuntimeError(f'Baseline dependency differs: {relative}')
        dependencies(child)
dependencies(root / 'math/bigint.hpp')
os.sched_setaffinity(0, {a.cpu})
rows = []
cases = [(257,8),(257,9),(257,16),(257,32),(257,33),(1024,9),(1024,16),(1024,32),(10000,9),(10000,32),(100000,9),(100000,32),(32,32),(256,16),(1024,1024)]
for mode in ['generic','native']:
    binary = a.output / mode
    flags = ['-std=c++23','-O2','-DNDEBUG']
    if mode == 'native': flags += ['-march=native','-ftrivial-auto-var-init=zero']
    subprocess.run([a.cxx,*flags,'-I',str(root),f'-DBIGINT_BASELINE_HEADER="{base.resolve()}/math/bigint.hpp"',str(root/'test/benchmark/bigint_unbalanced.cpp'),'-o',str(binary)],check=True)
    for n,m in cases:
        result = subprocess.run([str(binary.resolve()),str(n),str(m),str(a.trials)],text=True,capture_output=True,check=True)
        (a.output/f'{mode}-{n}-{m}.log').write_text(result.stdout+result.stderr)
        records = [list(map(float,s.split())) for s in result.stdout.splitlines()]
        if len(records) != a.trials or any(min(r[5], r[6]) < .20 - 1e-9 for r in records):
            raise RuntimeError('Invalid measured duration or trial count')
        row = {'mode':mode,'flags':flags,'n':n,'m':m,'ratio':statistics.median(r[7] for r in records),'records':records}
        rows.append(row)
        print(mode,n,m,row['ratio'],flush=True)
        (a.output/'results.json').write_text(json.dumps({'base':a.base,'compiler':subprocess.check_output([a.cxx,'--version'],text=True),'platform':platform.platform(),'flags':flags,'cpu':a.cpu,'header_sha256':hashlib.sha256((root/'math/bigint.hpp').read_bytes()).hexdigest(),'benchmark_sha256':hashlib.sha256((root/'test/benchmark/bigint_unbalanced.cpp').read_bytes()).hexdigest(),'rows':rows},indent=2))
