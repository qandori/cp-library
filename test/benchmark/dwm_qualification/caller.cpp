#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sys/resource.h>
#include <vector>
#ifndef BLOCK
#define BLOCK 1024
#endif
#ifndef BRANCH
#define BRANCH 8
#endif
#ifndef BITS
#define BITS 20
#endif
#ifndef VALUE_TYPE
#define VALUE_TYPE unsigned
#endif
#ifndef CALLER_SHAPE
#error "CALLER_SHAPE must explicitly select 1 (access) or 2 (ordinary mixed)"
#endif
static_assert(CALLER_SHAPE == 1 || CALLER_SHAPE == 2);
using T=VALUE_TYPE;
using U=std::make_unsigned_t<T>;
constexpr U value_mask=std::numeric_limits<U>::max()>>(std::numeric_limits<U>::digits-BITS);
constexpr U value_sign=std::is_signed_v<T>?U(1)<<(BITS-1):0;
T decode_key(U key){key^=value_sign;if constexpr(std::is_signed_v<T>&&BITS<std::numeric_limits<U>::digits)if(key&value_sign)key|=U(~value_mask);return T(key);}
using WM=DynamicWaveletMatrix<T,BITS,DynamicBitVector<BLOCK,BRANCH>>;
extern void escape_memory(const void *,std::size_t);
struct Op { int type,p,l,r,k; T x,y; };
double cpu_now(){timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec+t.tv_nsec*1e-9;}
double wall_now(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
std::uint64_t digest(const std::vector<std::uint64_t>&v){std::uint64_t h=1469598103934665603ULL;for(auto x:v) h=(h^x)*1099511628211ULL;return h;}
int main(int argc,char **argv){
 if(argc<10)return 2;
 std::string kind=argv[1],dist=argv[5],loc=argv[6],cache=argv[8];
 if constexpr(CALLER_SHAPE==1)if(kind!="access")return 2;
 if constexpr(CALLER_SHAPE==2)if(kind!="simplemixed")return 2;
 int n=std::stoi(argv[2]),q=std::stoi(argv[3]),reps=std::stoi(argv[7]); unsigned seed=std::stoul(argv[4]);
 if(n<=0||q<0||reps<=0)return 2;
 std::string dump=argv[9]; std::mt19937_64 rng(seed); constexpr U mask=value_mask;
 auto value=[&](){U x=U(rng())&mask;if(dist=="duplicates")x&=15U;if(dist=="narrow20")x&=U((1ULL<<std::min(BITS,20))-1);if(dist=="equal")x=7U&mask;if(dist=="sparse")x=U((x&15U)<<std::max(BITS-4,0));return decode_key(x);};
 std::vector<T>a(n);for(int i=0;i<n;++i)a[i]=dist=="sorted"?decode_key(U(__uint128_t(i)*mask/std::max(n,1))):value();
 std::vector<Op>ops;ops.reserve(q);int size=n;
 for(int i=0;i<q;++i){
  int t;
  if constexpr(CALLER_SHAPE==2){constexpr int types[]={6,2,3,4};t=types[rng()%4];}
  else if(kind=="access")t=6;else if(kind=="select")t=7;else if(kind=="updates")t=2;else if(kind=="insert")t=0;else if(kind=="erase")t=size>1?1:0;else if(kind=="neighbors")t=8+(rng()&1);else if(kind=="prev")t=8;else if(kind=="next")t=9;else if(kind=="count")t=5;else if(kind=="rank")t=13;else if(kind=="freq")t=4;else if(kind=="range"||kind=="range-narrow")t=10;else if(kind=="kth")t=3;else if(kind=="largest")t=11;else if(kind=="queries"||kind=="short")t=3+rng()%11;else if(kind=="pushpop")t=(i&1)?15:14;else t=rng()%16;
  if(!size)t=0;
  int p=size?int(rng()%(size+(t==0))):0;
  if(loc=="front")p=0;else if(loc=="back")p=size-(t!=0);else if(loc=="local")p=std::min(size-(t!=0),int((i/256*97+rng()%65)%(size+(t==0))));else if(loc=="boundary")p=std::min(size-(t!=0),int((rng()%std::max(1,size/BLOCK+1))*BLOCK));
  int l=rng()%(size+1),r=rng()%(size+1);if(l>r)std::swap(l,r);
  if(kind=="short")r=std::min(r,l+16);
  if((t==3||t==11)&&l==r){l=0;r=std::min(size,kind=="short"?16:size);}
  T x=value(),y=value();if(kind!="range-narrow"&&t==10&&x>y)std::swap(x,y);if(kind=="range-narrow"){U key=(U(x)&mask)^value_sign;key+=std::min<U>(16,mask-key);y=decode_key(key);}
  if(t==7)x=a.empty()?0:a[rng()%a.size()];
  int k=l<r?rng()%(r-l):0;if(t==7)k=dist=="duplicates"?rng()%std::max(1,n/16):0;
  ops.push_back({t,p,l,r,k,x,y});size+=(t==0||t==14)-(t==1||t==15);
 }
 if(dump!="-"){std::ofstream f(dump+".input");f<<n<<" "<<q<<"\n";for(auto v:a)f<<+v<<" ";f<<"\n";for(auto o:ops)f<<o.type<<" "<<o.p<<" "<<o.l<<" "<<o.r<<" "<<o.k<<" "<<+o.x<<" "<<+o.y<<"\n";}
 std::vector<std::uint64_t>output(2*std::size_t(q)),final,first_output;std::vector<std::uint64_t>flush(cache=="cold"?(512ULL<<20)/8:0,1);
 double cpu=0,wall=0,buildcpu=0;std::uint64_t hash=0,finalhash=0;std::size_t memory=0;
 for(int rep=0;rep<reps;++rep){
  WM wm;double bc=cpu_now();if(kind!="build")wm.build(a);buildcpu+=cpu_now()-bc;
  if(cache=="warm"&&!wm.empty()){for(int i=0;i<1024;++i){auto x=wm.access((std::uint64_t(i)*7919)%a.size());escape_memory(&x,sizeof(x));}}
  if(cache=="cold"){for(auto &x:flush)x=x*6364136223846793005ULL+1;escape_memory(flush.data(),flush.size()*8);}
  double c=cpu_now(),w=wall_now();
  if(kind=="build"||kind=="rebuild")wm.build(a);
  else for(std::size_t i=0;i<ops.size();++i){auto [t,p,l,r,k,x,y]=ops[i];std::uint64_t z=0,tag=0;
#if CALLER_SHAPE == 1
   z=wm.access(p);
#elif CALLER_SHAPE == 2
   if(t==6)z=wm.access(p);
   else if(t==2)wm.set(p,x);
   else if(t==3)z=wm.kth_smallest(l,r,k);
   else z=wm.range_freq(l,r,x);
#else
   switch(t){case 0:wm.insert(p,x);break;case 1:z=wm.erase(p);break;case 2:wm.set(p,x);break;case 3:z=wm.kth_smallest(l,r,k);break;case 4:z=wm.range_freq(l,r,x);break;case 5:z=wm.count(l,r,x);break;case 6:z=wm.access(p);break;case 7:z=std::uint64_t(std::int64_t(wm.select(x,k)));break;case 8:{auto v=wm.prev_value(l,r,x);z=v?std::uint64_t(*v):0;tag=v.has_value();break;}case 9:{auto v=wm.next_value(l,r,x);z=v?std::uint64_t(*v):0;tag=v.has_value();break;}case 10:z=wm.range_freq(l,r,x,y);break;case 11:z=wm.kth_largest(l,r,k);break;case 12:z=wm[p];break;case 13:z=wm.rank(x,r);break;case 14:wm.push_back(x);break;case 15:z=wm.pop_back();break;}
#endif
   output[2*i]=z;output[2*i+1]=tag;
  }
  escape_memory(output.data(),output.size()*sizeof(output[0]));escape_memory(&wm,sizeof(wm));
  wall+=wall_now()-w;cpu+=cpu_now()-c;
  escape_memory(output.data(),output.size()*sizeof(output[0]));escape_memory(&wm,sizeof(wm));
  memory=std::max(memory,wm.memory_usage());
  auto output_hash=digest(output);if(rep&&output_hash!=hash)std::abort();if(!rep&&std::getenv("DWM_EXPECTED_HASH")&&output_hash!=std::stoull(std::getenv("DWM_EXPECTED_HASH")))std::abort();
  if(std::getenv("DWM_VALIDATE_REPLAYS")){std::vector<std::uint64_t>v(wm.size());for(int i=0;i<wm.size();++i)v[i]=wm[i];if(rep&&(output!=first_output||v!=final))std::abort();if(!rep)first_output=output;}
  if(rep==0){hash=digest(output);final.resize(wm.size());for(int i=0;i<wm.size();++i)final[i]=wm[i];finalhash=digest(final);if(std::getenv("DWM_EXPECTED_FINAL_HASH")&&finalhash!=std::stoull(std::getenv("DWM_EXPECTED_FINAL_HASH")))std::abort();if(dump!="-"){std::ofstream f(dump,std::ios::binary);std::uint64_t sizes[]={output.size(),final.size()};f.write((char*)sizes,sizeof(sizes));f.write((char*)output.data(),output.size()*8);f.write((char*)final.data(),final.size()*8);}}
 }
 rusage ru;getrusage(RUSAGE_SELF,&ru);
 std::cout<<std::setprecision(12)<<"{\"cpu\":"<<cpu<<",\"wall\":"<<wall<<",\"buildcpu\":"<<buildcpu<<",\"reps\":"<<reps<<",\"hash\":\""<<hash<<"\",\"finalhash\":\""<<finalhash<<"\",\"memory\":"<<memory<<",\"rss_kib\":"<<ru.ru_maxrss<<"}\n";
}
