import os,sys,json,math,hashlib,subprocess,time,statistics,shutil
from pathlib import Path
source=Path(__file__).resolve().parent;repo=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]).resolve();target=sys.argv[3]
work.mkdir(parents=True,exist_ok=True);out=work/'evidence';out.mkdir(exist_ok=True);bins=work/'bins';bins.mkdir(exist_ok=True)
plan=json.loads((source/'plan.json').read_text());cpu=min(os.sched_getaffinity(0));flags=['-O2','-std=gnu++23','-fmodules','-ftrivial-auto-var-init=zero','-DONLINE_JUDGE','-DATCODER']+(['-march=native'] if target=='native' else [])
command_number=0
def command(args,**kw):
 global command_number
 command_number+=1;prefix=out/f'command-{command_number:05d}';started=time.time()
 metadata=dict(args=list(map(str,args)),started=started,finished=None,returncode=None)
 prefix.with_suffix('.json').write_text(json.dumps(metadata,indent=2))
 try:
  with prefix.with_suffix('.stdout').open('w') as stdout,prefix.with_suffix('.stderr').open('w') as stderr:
   p=subprocess.run(args,text=True,stdout=stdout,stderr=stderr,timeout=240,**kw)
  metadata.update(finished=time.time(),returncode=p.returncode)
  prefix.with_suffix('.json').write_text(json.dumps(metadata,indent=2))
  if p.returncode:raise RuntimeError(f'command failed {args}; see {prefix.name}')
  return prefix.with_suffix('.stdout').read_text()
 except Exception as error:
  metadata.update(finished=time.time(),error=str(error));prefix.with_suffix('.json').write_text(json.dumps(metadata,indent=2));raise
basic=dict(started=time.time(),base=plan['base'],target=target,flags=flags,cpu_affinity=cpu,sources={str(p.relative_to(source)):hashlib.sha256(p.read_bytes()).hexdigest() for p in source.rglob('*') if p.is_file()})
(out/'started.json').write_text(json.dumps(basic,indent=2))
version=command(['g++-15','-dumpfullversion']).strip();(out/'compiler-version.txt').write_text(version);assert version=='15.2.0',version
provenance=dict(base=plan['base'],head=command(['git','-C',str(repo),'rev-parse','HEAD']).strip(),compiler=command(['g++-15','--version']),flags=flags,cpu_affinity=cpu,uname=command(['uname','-a']),lscpu=command(['lscpu']),target=target,sources={str(p.relative_to(source)):hashlib.sha256(p.read_bytes()).hexdigest() for p in source.rglob('*') if p.is_file()})
observed_headers={h:hashlib.sha256((repo/h).read_bytes()).hexdigest() for h in ['components/dynamic_bit_vector.hpp','data_structure/dynamic_wavelet_matrix.hpp']};(out/'base-headers.json').write_text(json.dumps(observed_headers,indent=2))
assert hashlib.sha256((repo/'components/dynamic_bit_vector.hpp').read_bytes()).hexdigest()=='6ccac3add8d0c35649ec0a2b50e03e1142502473c8e2b1fd453ffe77c32689e7'
assert hashlib.sha256((repo/'data_structure/dynamic_wavelet_matrix.hpp').read_bytes()).hexdigest()=='a2af5a1addcd64377a290b1f5dd6ec7587f100d0132c8f6551649af4468ae33a'
compiler=Path(shutil.which('g++-15')).resolve();cc1=Path(command(['g++-15','-print-prog-name=cc1plus']).strip()).resolve();provenance.update(compiler_path=str(compiler),compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),cc1plus_path=str(cc1),cc1plus_sha256=hashlib.sha256(cc1.read_bytes()).hexdigest());(out/'provenance.json').write_text(json.dumps(provenance,indent=2));variants={};built=set()
for name in ['baseline']+list(plan['plans']):
 dest=work/name;dest.mkdir(exist_ok=True)
 for directory in ['components','data_structure']:shutil.copytree(repo/directory,dest/directory,dirs_exist_ok=True)
 if name!='baseline':command(['patch','--fuzz=0','-d',str(dest),'-p1','-i',str(source/'patches'/f'{name}.patch')])
 variants[name]=dest

def binary(name,case):
 key=(name,case['bits'],case['block'],case['branch']);path=bins/f'{name}-{key[1]}-{key[2]}-{key[3]}'
 if key not in built:
  extra=[f'-DBITS={key[1]}',f'-DBLOCK={key[2]}',f'-DBRANCH={key[3]}']+(['-DVALUE_TYPE=std::uint64_t'] if key[1]==64 else [])
  cmd=['g++-15',*flags,*extra,'-I',str(variants[name]),str(source/'bench.cpp'),str(source/'escape.cpp'),'-o',str(path)]
  command(cmd);built.add(key)
  (out/(path.name+'.compile.json')).write_text(json.dumps(dict(command=cmd,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),size=command(['size',str(path)]),headers={h:hashlib.sha256((variants[name]/h).read_bytes()).hexdigest() for h in ['components/dynamic_bit_vector.hpp','data_structure/dynamic_wavelet_matrix.hpp']}),indent=2))
  (out/(path.name+'.asm')).write_text(command(['objdump','-Cd',str(path)]))
 return path
rows=[];preflights={};preflight_dir=out/'preflight';preflight_dir.mkdir(exist_ok=True)
def sample(name,c,reps,filename=None,validate=False,expected=None):
 path=binary(name,c);cmd=['taskset','-c',str(cpu),str(path),c['kind'],str(c['n']),str(c['q']),str(c['seed']),c['dist'],c['loc'],str(reps),c['cache'],str(filename) if filename else '-']
 env=os.environ.copy()
 if validate:env['DWM_VALIDATE_REPLAYS']='1'
 if expected:env.update(DWM_EXPECTED_HASH=expected['hash'],DWM_EXPECTED_FINAL_HASH=expected['finalhash'])
 start=time.time();r=json.loads(command(cmd,env=env));r.update(variant=name,case=c,target=target,command=cmd,validation=('full_exact_each_replay' if validate else 'all_output_digests_and_first_final_state'),total_wall=time.time()-start)
 if filename:r.update(input_sha256=hashlib.sha256(Path(str(filename)+'.input').read_bytes()).hexdigest(),full_output_sha256=hashlib.sha256(Path(filename).read_bytes()).hexdigest())
 return r

def record(r):
 rows.append(r)
 with (out/'samples.jsonl').open('a') as f:f.write(json.dumps(r)+'\n')

def preflight(name,case_name):
 key=(name,case_name)
 if key not in preflights:
  file=preflight_dir/f'{name}-{case_name}.out';r=sample(name,plan['cases'][case_name],3,file,True);r.update(stage='exact_case_preflight',case_name=case_name);record(r);preflights[key]=(r,file)
 return preflights[key]

def compare(name,case_name,aa=False):
 c=plan['cases'][case_name];first='baseline';second='baseline' if aa else name;target_cpu=plan['aa_target_cpu'] if aa else plan['target_cpu'];label=('aa' if aa else name)+'-'+case_name
 checks=[preflight(v,case_name) for v in [first,second]]
 assert checks[0][1].read_bytes()==checks[1][1].read_bytes(),label+' preflight mismatch'
 assert Path(str(checks[0][1])+'.input').read_bytes()==Path(str(checks[1][1])+'.input').read_bytes(),label+' input mismatch'
 pilots=[]
 for i,v in enumerate([first,second]):
  r=sample(v,c,1,expected=checks[i][0]);r.update(stage='pilot',comparison=label,leg=i);record(r);pilots.append(r)
 repeats=max(1,math.ceil(target_cpu/max(min(r['cpu'] for r in pilots),1e-6)))
 if repeats>5000:raise RuntimeError(f'bad pilot/repeat estimate: {label} {pilots}')
 cell=[]
 for trial in range(plan['trials']):
  for leg in ([0,1] if trial%2==0 else [1,0]):
   v=[first,second][leg];dump=out/f'{label}-{leg}.out' if trial==0 else None;r=sample(v,c,repeats,dump,expected=checks[leg][0])
   r.update(stage='measurement',comparison=label,trial=trial,leg=leg);record(r);cell.append(r)
 assert len({(r['hash'],r['finalhash']) for r in cell})==1,label
 assert (out/f'{label}-0.out').read_bytes()==(out/f'{label}-1.out').read_bytes(),label
 assert (out/f'{label}-0.out.input').read_bytes()==(out/f'{label}-1.out.input').read_bytes(),label
 ratios=[next(r['cpu'] for r in cell if r['trial']==t and r['leg']==0)/next(r['cpu'] for r in cell if r['trial']==t and r['leg']==1) for t in range(plan['trials'])]
 summary=dict(comparison=label,case=case_name,aa=aa,median=statistics.median(ratios),minimum=min(ratios),maximum=max(ratios),ratios=ratios,all_samples_long_enough=all(r['cpu']>=plan['minimum_cpu'] for r in cell),baseline_memory=cell[0]['memory'],ratio_direction='baseline/candidate; above1 means candidate faster',interpretation='descriptive screening only; floor miss, unstable or unmatched AA family means inconclusive')
 with (out/'summary.jsonl').open('a') as f:f.write(json.dumps(summary)+'\n')
 print(json.dumps(summary),flush=True)
 # Full output remains verified but is not needed in the retained bounded artifact.
 for leg in [0,1]:
  (out/f'{label}-{leg}.out').unlink();(out/f'{label}-{leg}.out.input').unlink()

# Per-replay reset correctness and independent vector oracle are separate from performance.
valid=set();failures=[]
def fail(name,stage,error):
 entry=dict(variant=name,stage=stage,error=str(error));failures.append(entry)
 with (out/'failures.jsonl').open('a') as f:f.write(json.dumps(entry)+'\n')
 print('FAILED',json.dumps(entry),flush=True)
 if name=='baseline':raise RuntimeError(entry)
for name,dest in variants.items():
 try:
  c=dict(plan['cases']['mixed'],n=2000,q=2000);dump=out/f'preflight-{name}.out';r=sample(name,c,3,dump,True);r.update(stage='preflight');record(r)
  result=command([sys.executable,str(source/'verify_oracle.py'),str(dump)]);print(result.strip(),flush=True)
  tests=['dynamic_architecture']
  if name in ['neighbors','uniformneighbors','selectprefix']:tests.append('dynamic_wavelet_neighbors')
  if name in ['leaf','noop','noopearly','height']:tests.append('review_edge_cases')
  if name=='leaf':tests.append('dynamic_leaf')
  for test in tests:
   executable=bins/f'{name}-{test}'
   cmd=['g++-15',*flags,'-I',str(dest),str(source/'tests'/f'{test}.cpp'),'-o',str(executable)]
   command(cmd);(out/(executable.name+'.sha256')).write_text(hashlib.sha256(executable.read_bytes()).hexdigest()+'\n');command(['taskset','-c',str(cpu),str(executable)])
   with (out/'tests.log').open('a') as f:f.write(name+' '+test+' PASS\n')
  valid.add(name)
 except Exception as error:fail(name,'preflight',error)
for case in plan['calibration']:compare('baseline',case,True)
for name,cases in plan['plans'].items():
 if name not in valid:continue
 for case in cases:
  try:compare(name,case)
  except Exception as error:
   fail(name,'comparison-'+case,error);break
(out/'completed.json').write_text(json.dumps(dict(complete=not failures,candidate_count=len(plan['plans']),failures=failures,measurements=sum(r.get('stage')=='measurement' for r in rows))))
print('FIXED PROTOCOL FINISHED',flush=True)
