#define main author_tests_main
#include "correctness.cpp"
#undef main
#include <iostream>
template<class T> void independent() {
 size_t count=0;
 for(auto [n,m]:std::vector<std::pair<int,int>>{{33,33},{33,65},{65,33},{64,33},{65,64},{129,65},{257,33},{513,129},{1025,33},{1025,513},{4097,33},{8193,65},{33,513},{129,1025}})
 for(int mode=0;mode<8;++mode){
  FormalPowerSeries<T> f(n);std::vector<T>x(m);
  for(int j=0;j<m;++j)x[j]=mode%4==0?T(0):mode%4==1?T(1):mode%4==2?T(T::mod()-1):T(3).pow((T::mod()-1)/128*(j%128));
  if(mode==0||mode==1||mode==2)f[n-1]=T(T::mod()-1);
  if(mode==3)f[0]=1;
  if(mode==4)for(int i=0;i<n;++i)f[i]=i%2?1:T::mod()-1;
  if(mode==5)for(int i=0;i<std::min(n,33);++i)f[i]=i+1;
  if(mode==6){f[0]=-1;f[n-1]=1;}
  auto a=multipoint_eval(f,x),b=baseline_eval(f,x);assert(a==b);assert(a.size()==x.size());
  for(int j=0;j<m;++j){uint64_t v=0;for(int i=n-1;i>=0;--i)v=(v*x[j].val()+f[i].val())%T::mod();assert(v==a[j].val());}
  ++count;
 }
 // Point product vanishes at every point, including repeated zeros and padded leading coefficients.
 for(int m:{33,40,63,64,65,129}){
  FormalPowerSeries<T> f{1};std::vector<T>x(m);
  for(int j=0;j<m;++j){x[j]=j%5;f.push_back(0);for(int k=j+1;k>0;--k)f[k]=f[k-1]-x[j]*f[k];f[0]*=-x[j];}
  f.resize(1025); auto a=multipoint_eval(f,x);assert(a==baseline_eval(f,x));for(auto v:a)assert(v.val()==0);++count;
 }
 std::cout << "independent adversarial " << count << " cases passed\n";
}
int main(){independent<fps_detail::Mint>();independent<PlainMint>();}
