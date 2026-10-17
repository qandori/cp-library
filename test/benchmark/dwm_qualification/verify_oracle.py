import struct,sys,bisect,json
from collections import Counter
from pathlib import Path
p=Path(sys.argv[1]);f=Path(str(p)+'.input').read_text().splitlines();n,q=map(int,f[0].split());a=list(map(int,f[1].split()));out=[]
assert len(a)==n and len(f[2:])==q
kind=sys.argv[2] if len(sys.argv)>2 else "operations"
if kind in ("build","rebuild"):out=[0]*(2*q)
initial=a.copy();observed=Counter();cycle_p=0;cycle_x=0
if kind in ("select-lastbit-miss","select-churn"):assert initial and len({v&1 for v in initial})==1
for op_index,line in enumerate([] if kind in ("build","rebuild") else f[2:]):
 t,p0,l,r,k,x,y=map(int,line.split());z=0;tag=0
 observed[f"operation_{t}"]+=1
 if kind=="select-lastbit-miss":assert t==7 and k==0 and x not in a and (x-1) in a and (x&1)!=(initial[0]&1)
 if kind=="select-high-k":assert t==7 and k==n and x in a
 if kind=="select-valid-dup":assert t==7 and 0<=k<a.count(x)
 if kind=="select-churn":
  step=op_index%10
  if step==0:
   assert a==initial and 0<=p0<n-1 and x==initial[p0]+1 and (x&1)!=(initial[0]&1)
   cycle_p=p0;cycle_x=x
  if step in (1,3,5,8):
   expected_p=cycle_p+(step in (3,5));expected_x=initial[expected_p] if step in (5,8) else cycle_x
   assert t==2 and p0==expected_p and x==expected_x
  else:
   assert t==7 and x==cycle_x and k==(1 if step in (4,7) else 0)
   assert a.count(x)=={0:0,2:1,4:2,6:1,7:1,9:0}[step]
   observed[f"churn_query_step_{step}"]+=1
 if t==0:a.insert(p0,x)
 elif t==1:z=a.pop(p0)
 elif t==2:a[p0]=x
 elif t==3:z=sorted(a[l:r])[k]
 elif t==4:z=sum(v<x for v in a[l:r])
 elif t==5:z=a[l:r].count(x)
 elif t in (6,12):z=a[p0]
 elif t==7:
  positions=[i for i,v in enumerate(a) if v==x];z=positions[k] if 0<=k<len(positions) else -1
  observed['select_hit' if z>=0 else 'select_miss']+=1
  if z>=0:observed['select_valid_singleton' if len(positions)==1 else 'select_valid_duplicate']+=1
  if z>=0 and k>0:observed['select_valid_later_occurrence']+=1
 elif t==8:
  v=[v for v in a[l:r] if v<x];tag=bool(v);z=max(v) if v else 0
 elif t==9:
  v=[v for v in a[l:r] if v>=x];tag=bool(v);z=min(v) if v else 0
 elif t==10:z=sum(x<=v<y for v in a[l:r])
 elif t==11:z=sorted(a[l:r],reverse=True)[k]
 elif t==13:z=a[:r].count(x)
 elif t==14:a.append(x)
 elif t==15:z=a.pop()
 out.extend((z&((1<<64)-1),int(tag)))
if kind=='select-churn' and q%10==0:assert a==initial
raw=p.read_bytes();nn,mm=struct.unpack('<QQ',raw[:16]);assert nn==2*q and len(raw)==16+8*(nn+mm);actual=list(struct.unpack('<'+'Q'*(nn+mm),raw[16:]));expected=out+[x&((1<<64)-1) for x in a];assert len(actual)==len(expected);assert actual==expected, next((i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y)
print('full independent oracle PASS',p,nn,mm)

print(json.dumps({'oracle':'pass','kind':kind,'n':n,'q':q,'workload_truth':dict(observed)},sort_keys=True))
