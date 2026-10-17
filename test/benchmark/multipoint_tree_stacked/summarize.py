import csv,statistics,math,sys
from collections import defaultdict
for path in sys.argv[1:]:
 d=defaultdict(list)
 for r in csv.DictReader(open(path)):d[int(r['n']),int(r['m'])].append(r)
 target=[];adverse=[]
 print('\n'+path+'\nn,m,paired_cpu_speedup,ratio_median_cpu,baseline_cpu_us,candidate_cpu_us,paired_wall_speedup,baseline_wall_us,candidate_wall_us')
 for (n,m),rs in sorted(d.items()):
  assert len(rs)==8
  a=[float(r['baseline_cpu'])/int(r['baseline_count']) for r in rs];b=[float(r['candidate_cpu'])/int(r['candidate_count']) for r in rs]
  wa=[float(r['baseline_wall'])/int(r['baseline_count']) for r in rs];wb=[float(r['candidate_wall'])/int(r['candidate_count']) for r in rs]
  for r in rs:assert float(r['baseline_cpu'])>=.15 and float(r['candidate_cpu'])>=.15
  paired=statistics.median(x/y for x,y in zip(a,b));ratio=statistics.median(a)/statistics.median(b)
  if n>=1024 and 33<=m<=129:target.append(paired)
  if paired<1/1.05:adverse.append((n,m,paired))
  print('%d,%d,%.5f,%.5f,%.4f,%.4f,%.5f,%.4f,%.4f'%(n,m,paired,ratio,statistics.median(a)*1e6,statistics.median(b)*1e6,statistics.median(x/y for x,y in zip(wa,wb)),statistics.median(wa)*1e6,statistics.median(wb)*1e6))
 gm=math.exp(statistics.mean(map(math.log,target)));share=sum(v>=1.05 for v in target)/len(target)
 print('Target GM',gm,'share >=1.05',share,'adverse',adverse,'GATE',gm>=1.10 and share>=.75 and not adverse)
