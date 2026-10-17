import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = '117fda44639532534bc91fb02517a1cba09ee738'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--baseline', default=BASELINE)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--cpu', type=int, default=0)
    parser.add_argument('--minimum-cpu', type=float, default=0.2)
    parser.add_argument('--trials', type=int, default=6)
    parser.add_argument('--native', action='store_true')
    args = parser.parse_args()
    if args.minimum_cpu < 0.15 or args.trials < 6 or args.trials % 2:
        parser.error('Require at least 0.15 CPU seconds and an even trial count of at least six')
    args.output.mkdir(parents=True, exist_ok=True)
    old = subprocess.check_output(['git', 'show', f'{args.baseline}:fps/compose.hpp'], cwd=ROOT, text=True)
    old = old.replace('CP_FPS_COMPOSE_HPP', 'CP_FPS_COMPOSE_BASELINE_HPP').replace('#include "fps.hpp"', '#include "fps/fps.hpp"')
    if old.count('FormalPowerSeries<T> compose(') != 1:
        raise RuntimeError('Unexpected baseline entry point')
    old = old.replace('FormalPowerSeries<T> compose(', 'FormalPowerSeries<T> baseline_compose(')
    flags = ['-std=c++23', '-O2', '-DNDEBUG'] + (['-march=native'] if args.native else [])
    paths = ['fps/compose.hpp', 'test/benchmark/compose_valuation.cpp', 'test/benchmark/compose_valuation.py']
    include_pattern = r'^\s*#\s*include\s*"([^"\n]+)"'
    original_compose = subprocess.check_output(['git', 'show', f'{args.baseline}:fps/compose.hpp'], cwd=ROOT, text=True)
    if re.findall(include_pattern, original_compose, re.M) != re.findall(include_pattern, (ROOT / paths[0]).read_text(), re.M):
        raise RuntimeError('Composition include graph changed; baseline isolation needs review')
    seen = set()
    pending = [paths[0]]
    while pending:
        path = pending.pop()
        if path in seen:
            continue
        seen.add(path)
        for include in re.findall(include_pattern, (ROOT / path).read_text(), re.M):
            candidates = [(ROOT / path).parent / include, ROOT / include]
            resolved = next((p.resolve() for p in candidates if p.is_file()), None)
            if resolved is None or not resolved.is_relative_to(ROOT):
                raise RuntimeError(f'Unresolved production dependency: {path}: {include}')
            pending.append(str(resolved.relative_to(ROOT)))
    dependencies = sorted(seen - {paths[0]})
    for path in dependencies:
        original = subprocess.check_output(['git', 'show', f'{args.baseline}:{path}'], cwd=ROOT)
        if original != (ROOT / path).read_bytes():
            raise RuntimeError(f'Shared dependency differs from baseline: {path}')
    env = dict(baseline=args.baseline, flags=flags, cpu=args.cpu, minimum_cpu=args.minimum_cpu, trials=args.trials,
               compiler=subprocess.check_output([args.cxx, '--version'], text=True), platform=platform.platform(),
               hashes={p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in paths + dependencies})
    (args.output / 'environment.json').write_text(json.dumps(env, indent=2))
    cases = [(n, v, 0) for n in [64, 256, 4096] for v in [1, 2, n // 16, n // 8, n // 4]]
    cases += [(32768, v, 0) for v in [1, 4096, 8192]]
    cases += [(4096, 512, k) for k in [1, 2, 3]]
    cases += [(n, v, 0) for n, v in [(8, 2), (32, 4), (33, 4), (65, 8), (65, 9), (129, 16), (129, 17)]]
    rows = []
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        (tmp / 'compose_baseline.hpp').write_text(old)
        subprocess.run([args.cxx, *flags, '-I', str(ROOT), '-I', str(tmp), str(ROOT / paths[1]), '-o', str(tmp / 'bench')], check=True)
        if hasattr(os, 'sched_setaffinity'):
            os.sched_setaffinity(0, {args.cpu})
        for n, valuation, kind in cases:
            result = subprocess.run([str(tmp / 'bench'), str(n), str(valuation), str(kind), str(args.minimum_cpu), str(args.trials)], capture_output=True, text=True, check=True)
            records = [list(map(float, line.split())) for line in result.stdout.splitlines()]
            if len(records) != args.trials or any(min(r[1], r[3]) < args.minimum_cpu for r in records):
                raise RuntimeError('Incomplete measurement or measured CPU floor violation')
            base = statistics.median(r[1] / r[2] for r in records)
            cur = statistics.median(r[3] / r[4] for r in records)
            rows.append(dict(n=n, valuation=valuation, kind=kind, records=records, baseline_seconds=base,
                             candidate_seconds=cur, paired_speedup=statistics.median(r[5] for r in records), ratio_of_medians=base / cur, checksum=result.stderr.strip()))
            (args.output / 'results.json').write_text(json.dumps(rows, indent=2))
            print(n, valuation, kind, rows[-1]['paired_speedup'], flush=True)

if __name__ == '__main__':
    main()
