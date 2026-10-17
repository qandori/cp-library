#include "fps/multipoint_eval.hpp"
#include "baseline.hpp"
#include <cstdio>
#include <random>
#include <ctime>
#include <chrono>
using T=fps_detail::Mint;using F=FormalPowerSeries<T>;using Fn=std::vector<T>(*)(const F&,const std::vector<T>&);
volatile uint64_t sink=0;
uint64_t checksum(const std::vector<T>& a){uint64_t s=0;for(T v:a)s=s*1000003+v.val();return s;}
double cpu(){timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec+t.tv_nsec*1e-9;}
double wall(){return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
struct Timing{double cpu,wall;int count;};
Timing measure(Fn fn,const F&f,const std::vector<T>&x,uint64_t expect){double a=cpu(),b=wall(),elapsed;int count=0;do{auto out=fn(f,x);auto c=checksum(out);if(c!=expect)std::abort();sink=sink+c;++count;elapsed=cpu()-a;}while(elapsed<.15);return {elapsed,wall()-b,count};}
int main(){const std::vector<std::pair<int,int>> cases={
#include "shapes.inc"
};
puts("n,m,pair,order,baseline_cpu,candidate_cpu,baseline_wall,candidate_wall,baseline_count,candidate_count,checksum");
for(int pair=0;pair<8;++pair)for(size_t j=0;j<cases.size();++j){size_t index=pair%2?cases.size()-1-j:j;auto[n,m]=cases[index];std::mt19937 rng(491991+index);F f(n);std::vector<T>x(m);for(auto&v:f)v=rng();for(auto&v:x)v=rng();auto base=baseline_eval(f,x),cand=multipoint_eval(f,x);if(base!=cand)return 2;uint64_t expected=checksum(base);Timing a,b;if((pair+index)%2){b=measure(multipoint_eval<T>,f,x,expected);a=measure(baseline_eval<T>,f,x,expected);}else{a=measure(baseline_eval<T>,f,x,expected);b=measure(multipoint_eval<T>,f,x,expected);}printf("%d,%d,%d,%s,%.12g,%.12g,%.12g,%.12g,%d,%d,%llu\n",n,m,pair,(pair+index)%2?"CB":"BC",a.cpu,b.cpu,a.wall,b.wall,a.count,b.count,(unsigned long long)expected);fflush(stdout);}}
