import json
from pathlib import Path
cases={}
def c(name,kind,n=100000,q=20000,dist='full',loc='random',bits=20,block=1024,branch=8):
 cases[name]=dict(kind=kind,n=n,q=q,seed=713,dist=dist,loc=loc,cache='warm',bits=bits,block=block,branch=branch)
c('mixed','mixed');c('updates','updates');c('access','access',q=40000);c('queries','queries');c('neighbors','neighbors');c('select','select');c('range','range');c('narrow','range-narrow');c('short','short');c('insert','insert',q=10000);c('erase','erase',q=10000)
c('build','build',q=0);c('builddup','build',q=0,dist='duplicates');c('buildequal','build',q=0,dist='equal');c('rebuild','rebuild',q=0)
c('dupupdate','updates',dist='duplicates');c('dupmixed','mixed',dist='duplicates');c('dupselect','select',dist='duplicates');c('dupneighbor','neighbors',dist='duplicates')
c('localupdate','updates',loc='local');c('smallupdate','updates',n=1024,loc='local');c('wide_select','select',bits=64);c('wide_neighbor','neighbors',dist='duplicates',bits=64)
plans={
 'leaf':['updates','localupdate','smallupdate','mixed','access'],
 'height':['updates','insert','erase','mixed','access'],
 'neighbors':['neighbors','dupneighbor','mixed','access','short'],
 'paired':['queries','range','short','mixed','access'],
 'pdep':['select','dupselect','mixed','access'],
 'avxbuild':['build','builddup','buildequal','mixed','access'],
 'reservebuild':['build','rebuild','insert','mixed','access'],
 'directbuild':['build','rebuild','insert','mixed','access'],
 'noop':['updates','dupupdate','dupmixed','mixed','access'],
 'noopearly':['updates','dupupdate','dupmixed','mixed','access'],
 'selectprefix':['select','dupselect','wide_select','mixed','access'],
 'rangeprefix':['range','narrow','mixed','access'],
 'uniformneighbors':['neighbors','dupneighbor','wide_neighbor','mixed','access']}
Path(__file__).with_name('plan.json').write_text(json.dumps(dict(base='829efa17b452f904eef5558930d91dd3792feffa',cases=cases,plans=plans,calibration=['mixed','updates','access','build'],trials=6,target_cpu=.4,aa_target_cpu=1.5,minimum_cpu=.15),indent=2)+'\n')
