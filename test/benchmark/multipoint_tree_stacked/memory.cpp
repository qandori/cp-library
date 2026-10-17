#include <cstddef>
#include <cstdlib>
#include <new>
#include <cstdint>
#include <cstdio>
struct Header{void* raw;size_t bytes;bool counted;};
bool active=false;size_t live=0,peak=0,total=0,allocations=0;
void* allocate(size_t n,size_t alignment){alignment=alignment<alignof(Header)?alignof(Header):alignment;void* raw=std::malloc(n+alignment+sizeof(Header));if(!raw)throw std::bad_alloc();uintptr_t addr=(reinterpret_cast<uintptr_t>(raw)+sizeof(Header)+alignment-1)&~(alignment-1);Header*h=reinterpret_cast<Header*>(addr)-1;*h={raw,n,active};live+=n;if(active){total+=n;++allocations;peak=peak<live?live:peak;}return reinterpret_cast<void*>(addr);}
void release(void*p){if(!p)return;auto h=reinterpret_cast<Header*>(p)-1;live-=h->bytes;std::free(h->raw);}
void*operator new(size_t n){return allocate(n,alignof(std::max_align_t));}void*operator new[](size_t n){return ::operator new(n);}void*operator new(size_t n,std::align_val_t a){return allocate(n,(size_t)a);}void*operator new[](size_t n,std::align_val_t a){return allocate(n,(size_t)a);}
void operator delete(void*p)noexcept{release(p);}void operator delete[](void*p)noexcept{release(p);}void operator delete(void*p,size_t)noexcept{release(p);}void operator delete[](void*p,size_t)noexcept{release(p);}void operator delete(void*p,std::align_val_t)noexcept{release(p);}void operator delete[](void*p,std::align_val_t)noexcept{release(p);}void operator delete(void*p,size_t,std::align_val_t)noexcept{release(p);}void operator delete[](void*p,size_t,std::align_val_t)noexcept{release(p);}
#include "fps/multipoint_eval.hpp"
#include "baseline.hpp"
#include <random>
using T=fps_detail::Mint;using F=FormalPowerSeries<T>;
int main(int argc,char**argv){
 if(argc!=4)return 1;int n=std::atoi(argv[1]),m=std::atoi(argv[2]),variant=std::atoi(argv[3]);
 F f(n);std::vector<T>x(m);std::mt19937 rng(761);for(auto&v:f)v=rng();for(auto&v:x)v=rng();
 {auto a=baseline_eval(f,x);auto b=multipoint_eval(f,x);assert(a==b);}
 size_t start=live;peak=live;total=allocations=0;active=true;
 {auto a=variant?multipoint_eval(f,x):baseline_eval(f,x);active=false;}
 printf("%d,%d,%s,%zu,%zu,%zu,%zu,%zu,%zu\n",n,m,variant?"candidate":"baseline",start,peak,live,total,allocations,sizeof(std::vector<T>));
}
