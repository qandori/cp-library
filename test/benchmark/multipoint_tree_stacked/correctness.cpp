#include "fps/multipoint_eval.hpp"
#include "baseline.hpp"
#include <cstdio>
#include <random>
#include <cassert>
struct PlainMint {
	unsigned int v = 0;
	static constexpr int mod() { return 998244353; }
	PlainMint() = default;
	PlainMint(long long x) { x %= mod(); v = static_cast<unsigned int>(x < 0 ? x + mod() : x); }
	static PlainMint raw(unsigned int x) { PlainMint r; r.v = x; return r; }
	unsigned int val() const { return v; }
	PlainMint &operator+=(PlainMint x) { v += x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator-=(PlainMint x) { v += mod() - x.v; if (v >= mod()) v -= mod(); return *this; }
	PlainMint &operator*=(PlainMint x) { v = static_cast<unsigned long long>(v) * x.v % mod(); return *this; }
	PlainMint &operator/=(PlainMint x) { return *this *= x.inv(); }
	PlainMint operator-() const { return raw(v ? mod() - v : 0); }
	PlainMint pow(long long k) const {
		PlainMint a = *this, r = 1;
		for (; k; k >>= 1, a *= a) if (k & 1) r *= a;
		return r;
	}
	PlainMint inv() const { return pow(mod() - 2); }
	friend PlainMint operator+(PlainMint a, PlainMint b) { return a += b; }
	friend PlainMint operator-(PlainMint a, PlainMint b) { return a -= b; }
	friend PlainMint operator*(PlainMint a, PlainMint b) { return a *= b; }
	friend PlainMint operator/(PlainMint a, PlainMint b) { return a /= b; }
	friend bool operator==(PlainMint, PlainMint) = default;
};
template<class T> void run() {
std::mt19937 rng(89341); size_t count=0;
for(int n: {0,1,7,8,31,32,33,63,64,65,127,128,129,255,256,257,1023,1024,1025,4096,4097})
for(int m: {0,1,7,8,31,32,33,63,64,65,127,128,129,255,256,257,1023,1024,1025}) {
FormalPowerSeries<T> f(n); std::vector<T>x(m);
for(auto& v:f)v=rng(); for(int j=0;j<m;++j)x[j]=j%4==0?0:j%4==1?T::mod()-1:j%4==2?7:rng();
auto a=baseline_eval(f,x),b=multipoint_eval(f,x);assert(a==b);
for(int j=0;j<m;++j){uint64_t value=0;for(int i=n-1;i>=0;--i)value=(value*x[j].val()+f[i].val())%T::mod();assert(value==b[j].val());}
if(n>32&&m>32){int length=8;while(length<std::max(n,m))length*=2;int old=length/8,now=1;std::vector<std::vector<T>> fo,fn;auto a=fps_detail::ProductTree<T>::build(x,old,8,true,&fo),b=fps_detail::ProductTree<T>::build(x,now,8,true,&fn);assert(a[1]==b[1]);}
++count;
}
for(auto [n,m]:std::vector<std::pair<int,int>>{{32768,33},{131072,129},{33,32768},{1025,1025}}){FormalPowerSeries<T>f(n);std::vector<T>x(m,0);assert(baseline_eval(f,x)==multipoint_eval(f,x));}
printf("correctness %zu shapes passed\n",count);
}
int main(){run<fps_detail::Mint>();run<PlainMint>();}
