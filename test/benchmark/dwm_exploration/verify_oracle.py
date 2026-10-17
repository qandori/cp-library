import struct,sys,bisect
from pathlib import Path
p=Path(sys.argv[1]);f=Path(str(p)+'.input').read_text().splitlines();n,q=map(int,f[0].split());a=list(map(int,f[1].split()));out=[]
for line in f[2:]:
 t,p0,l,r,k,x,y=map(int,line.split());z=0;tag=0
 if t==0:a.insert(p0,x)
 elif t==1:z=a.pop(p0)
 elif t==2:a[p0]=x
 elif t==3:z=sorted(a[l:r])[k]
 elif t==4:z=sum(v<x for v in a[l:r])
 elif t==5:z=a[l:r].count(x)
 elif t in (6,12):z=a[p0]
 elif t==7:
  positions=[i for i,v in enumerate(a) if v==x];z=positions[k] if k<len(positions) else -1
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
raw=p.read_bytes();nn,mm=struct.unpack('<QQ',raw[:16]);actual=list(struct.unpack('<'+'Q'*(nn+mm),raw[16:]));expected=out+[x&((1<<64)-1) for x in a];assert actual==expected, next((i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y)
print('full independent oracle PASS',p,nn,mm)
