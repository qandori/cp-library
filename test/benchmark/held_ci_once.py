import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time

ROOT = Path.cwd().resolve()
OUT = ROOT / 'benchmark-results'
OUT.mkdir(exist_ok=True)
BASE = '117fda44639532534bc91fb02517a1cba09ee738'
EXPECTED = json.loads((ROOT / 'test/benchmark/held_ci_hashes.json').read_text())
for p, sha in EXPECTED.items():
    assert hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == sha, p
# Isolate original functions against byte-identical shared production dependencies.
seen = set()
pending = ['fps/multipoint_eval.hpp', 'fps/compose.hpp']
while pending:
    p = pending.pop()
    if p in seen:
        continue
    seen.add(p)
    for name in re.findall(r'^\s*#\s*include\s*"([^"\n]+)"', (ROOT / p).read_text(), re.M):
        q = next((x.resolve() for x in [(ROOT / p).parent / name, ROOT / name] if x.is_file()), None)
        assert q and q.is_relative_to(ROOT), (p, name)
        pending.append(str(q.relative_to(ROOT)))
for p in sorted(seen - {'fps/multipoint_eval.hpp', 'fps/compose.hpp', 'components/fps_direct_eval.hpp'}):
    assert subprocess.check_output(['git', 'show', BASE + ':' + p]) == (ROOT / p).read_bytes(), p
allowed = sorted(os.sched_getaffinity(0))
cpu = allowed[0]
real_cxx = shutil.which('g++')
metadata = {'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(), 'baseline': BASE,
            'allowed_affinity': allowed, 'selected_cpu': cpu, 'compiler_path': real_cxx,
            'compiler': subprocess.check_output([real_cxx, '--version'], text=True),
            'lscpu': subprocess.check_output(['lscpu'], text=True),
            'uname': subprocess.check_output(['uname', '-a'], text=True),
            'source_sha256': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in sorted(seen | set(EXPECTED))}}
(OUT / 'environment.json').write_text(json.dumps(metadata, indent=2))
wrapper = OUT / 'compiler-wrapper'
wrapper.mkdir(exist_ok=True)
(wrapper / 'g++').write_text('''#!/usr/bin/env python3
import hashlib,json,os,shutil,subprocess,sys
from pathlib import Path
args=sys.argv[1:]
code=subprocess.call([os.environ['REAL_CXX'], *args])
if code == 0 and '-o' in args:
    source=Path(args[args.index('-o')+1])
    dest=Path(os.environ['BENCH_OUT']) / (os.environ['BENCH_LABEL']+'.bin')
    shutil.copy2(source,dest)
    entry={'label':os.environ['BENCH_LABEL'],'compiler':os.environ['REAL_CXX'],'arguments':args,'sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
    with (Path(os.environ['BENCH_OUT'])/'binaries.jsonl').open('a') as f: f.write(json.dumps(entry)+'\\n')
sys.exit(code)
''')
(wrapper / 'g++').chmod(0o755)
env = dict(os.environ, PATH=str(wrapper) + os.pathsep + os.environ['PATH'], REAL_CXX=real_cxx, BENCH_OUT=str(OUT))
for suite in ['multipoint', 'composition']:
    for native in [False, True]:
        label = suite + ('-native' if native else '-generic')
        script = 'multipoint_eval.py' if suite == 'multipoint' else 'compose_valuation.py'
        args = ['python3', 'test/benchmark/' + script, '--output', str(OUT / label), '--cpu', str(cpu)]
        args += ['--pairs', '8', '--seconds', '0.15'] if suite == 'multipoint' else ['--trials', '6', '--minimum-cpu', '0.2']
        if native:
            args += ['--native']
        print('BEGIN_PROTOCOL', label, json.dumps(args), flush=True)
        t = time.perf_counter()
        subprocess.run(args, env=dict(env, BENCH_LABEL=label), check=True)
        record = {'label': label, 'wall_seconds': time.perf_counter() - t, 'command': args}
        with (OUT / 'protocol-times.jsonl').open('a') as f:
            f.write(json.dumps(record) + '\n')
        print('END_PROTOCOL', json.dumps(record), flush=True)
