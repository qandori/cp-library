
#include "components/dynamic_quad_vector.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>
template<int Block> void test() {
 using V=dynamic_wavelet_detail::QuadVector<Block,4>;
 for(int n:{1025,4095,4096,4097,Block-1,Block,Block+1}) for(int mode=0;mode<4;++mode){
  std::vector<std::uint8_t>a(n); for(int i=0;i<n;++i)a[i]=mode==0?(i*11+i/13)%4:mode==1?(i==0||i+1==n?0:1):mode==2?3:(i/64/5)%4;
  V v(a);
  auto check=[&](){std::array<std::vector<int>,4> positions;for(int i=0;i<int(a.size());++i)positions[a[i]].push_back(i);for(int d=0;d<4;++d){assert(v.select(d,-1)==-1);assert(v.select(d,int(positions[d].size()))==-1);for(int k=0;k<int(positions[d].size());++k)assert(v.select(d,k)==positions[d][k]);}};
  check();
  for(int p:{0,64,n-1}){int d=(a[p]+1)%4;v.set_rank(p,d);a[p]=d;check();}
  int d=(a.front()+1)%4;v.move_rank(0,n-1,d);a.erase(a.begin());a.push_back(d);check();
  v.erase_rank(n-1);a.pop_back();v.insert(63,2);a.insert(a.begin()+63,2);check();
 }
}
int main(){test<8192>();test<32768>();}
