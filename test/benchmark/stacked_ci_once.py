import hashlib,json,os,shutil,subprocess,time
from pathlib import Path
root=Path.cwd().resolve()
out=Path(os.environ['RUNNER_TEMP'])/'stacked-tree-evidence'
out.mkdir(exist_ok=False)
allowed=sorted(os.sched_getaffinity(0)); cpu=0 if 0 in allowed else allowed[0]
cxx=shutil.which('g++')
meta={'commit':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'allowed_affinity':allowed,'selected_cpu':cpu,'compiler_path':cxx,'compiler':subprocess.check_output([cxx,'--version'],text=True),'lscpu':subprocess.check_output(['lscpu'],text=True),'uname':subprocess.check_output(['uname','-a'],text=True),'baseline':'31d238c62d0b297b1ffaf8b32a9b08461c2cadd9'}
meta['protocol_erratum'] = {'file':'test/benchmark/multipoint_tree_stacked/protocol.md','issue':'Proposed candidate header SHA256 prose line has a transcription error. The frozen protocol file remains unchanged.','canonical_candidate_sha256':'4cb93dd9d9d2b0bbbece729c7de93217a19cd4915cf9ace9aa797c0eefcb049e','authority':'Verified two-hunk patch, manifest, and frozen runner agree; no source, sampling, or acceptance threshold changes.'}
(out/'environment.json').write_text(json.dumps(meta,indent=2))
wrapper=out/'compiler-wrapper'; wrapper.mkdir()
(wrapper/'g++').write_text('''#!/usr/bin/env python3
import hashlib,json,os,subprocess,sys
from pathlib import Path
args=sys.argv[1:]
code=subprocess.call([os.environ['REAL_CXX'],*args])
if code==0 and '-o' in args:
 p=Path(args[args.index('-o')+1])
 entry={'compiler':os.environ['REAL_CXX'],'arguments':args,'binary':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
 with (Path(os.environ['BENCH_OUT'])/'binaries.jsonl').open('a') as f:f.write(json.dumps(entry)+'\\n')
sys.exit(code)
''')
(wrapper/'g++').chmod(0o755)
runner=root/'test/benchmark/multipoint_tree_stacked/run.py'
assert hashlib.sha256(runner.read_bytes()).hexdigest()=='5f3348a51f7a430a611aab236716068d6445c62a6c2d0bee7054ae4e463f43d9'
env=dict(os.environ,REAL_CXX=cxx,BENCH_OUT=str(out))
args=['python3',str(runner),'--output',str(out/'run'),'--cpu',str(cpu),'--cxx',str(wrapper/'g++')]
print('BEGIN_PROTOCOL',json.dumps(args),flush=True)
t=time.perf_counter()
try:subprocess.run(args,env=env,check=True)
finally:
 (out/'protocol-time.json').write_text(json.dumps({'command':args,'wall_seconds':time.perf_counter()-t},indent=2))
 for p in sorted(out.rglob('*')):
  if p.is_file() and p.suffix in {'.json','.jsonl','.csv','.txt','.log'}:
   print('BEGIN_EVIDENCE',str(p.relative_to(out)),flush=True); print(p.read_text(),flush=True); print('END_EVIDENCE',str(p.relative_to(out)),flush=True)
