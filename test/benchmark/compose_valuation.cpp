#include "compose_baseline.hpp"
#include "fps/compose.hpp"
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <iomanip>
using F=FormalPowerSeries<mint>;
double minimum_cpu;
double cpu(){timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);return t.tv_sec+t.tv_nsec*1e-9;}
std::uint64_t sink=0;
template<class Op>std::pair<double,int> run(Op op){int count=0;double t=cpu();do{auto a=op();for(auto x:a)sink+=x.val();++count;}while(cpu()-t<minimum_cpu);return{cpu()-t,count};}
int main(int argc,char**argv){if(argc!=6)return 1;minimum_cpu=std::stod(argv[4]);int trials=std::stoi(argv[5]);std::cout<<std::setprecision(17);int n=std::stoi(argv[1]),v=std::stoi(argv[2]),kind=std::stoi(argv[3]);std::mt19937 rng(1234);F f(n),g(n);for(auto &x:f)x=rng();for(int i=v;i<n;++i)g[i]=rng();if(v<n)g[v]=1;if(kind==1)g.resize(v+1);if(kind==2){for(int i=0;i<std::min(n,(n-1)/v+1);++i)f[i]=0;}if(kind==3)f.resize(std::min(n,8));auto old=[&]{return baseline_compose(f,g,n);};auto cur=[&]{return compose(f,g,n);};if(old()!=cur())return 2;for(int t=0;t<trials;++t){std::pair<double,int>b,c;if((t+n+v)&1){b=run(old);c=run(cur);}else{c=run(cur);b=run(old);}std::cout<<t<<' '<<b.first<<' '<<b.second<<' '<<c.first<<' '<<c.second<<' '<<(b.first/b.second)/(c.first/c.second)<<'\n';}std::cerr<<sink<<'\n';}
