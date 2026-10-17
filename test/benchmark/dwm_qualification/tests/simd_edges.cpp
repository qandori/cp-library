#include "components/dynamic_bit_vector.hpp"
#include "data_structure/dynamic_wavelet_matrix.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

template<class T, int Bits> void pack_edges() {
 using U=std::make_unsigned_t<T>;
 constexpr int W=std::numeric_limits<U>::digits;
 constexpr U mask=std::numeric_limits<U>::max()>>(W-Bits);
 constexpr U sign=std::is_signed_v<T>?U(1)<<(Bits-1):0;
 auto decode=[](U k){k^=sign;if constexpr(std::is_signed_v<T>&&Bits<W)if(k&sign)k|=U(~mask);return T(k);};
 DynamicWaveletMatrix<T,Bits> wm;
 for(int n=0;n<=129;++n) {
  for(int shift=0;shift<Bits;++shift) {
   std::vector<T> a(n);
   for(int i=0;i<n;++i) {U k=(i%3==0)?mask:(i%3==1)?U(1)<<shift:U(0);a[i]=decode(k);}
   wm.build(a);
   for(int i=0;i<n;++i) assert(wm.access(i)==a[i]);
   for(T x:{decode(0),decode(mask),decode(U(1)<<shift)}) {
    int k=0;
    for(int i=0;i<n;++i)if(a[i]==x)assert(wm.select(x,k++)==i);
    assert(wm.select(x,k)==-1);
   }
  }
 }
}
int main() {
 DynamicBitVector<128,4> v;
 v.build(65,std::vector<std::uint64_t>{~std::uint64_t(0),0});
 assert(v.select1(63)==63&&v.select0(0)==64&&v.select1(64)==-1);
 v.build(65,std::vector<std::uint64_t>{0,1});
 assert(v.select0(63)==63&&v.select1(0)==64&&v.select0(64)==-1);
 pack_edges<std::uint32_t,1>();pack_edges<std::uint32_t,31>();pack_edges<std::uint32_t,32>();
 pack_edges<std::int32_t,1>();pack_edges<std::int32_t,31>();pack_edges<std::int32_t,32>();
 pack_edges<std::uint64_t,1>();pack_edges<std::uint64_t,31>();pack_edges<std::uint64_t,33>();pack_edges<std::uint64_t,63>();pack_edges<std::uint64_t,64>();
 pack_edges<std::int64_t,1>();pack_edges<std::int64_t,31>();pack_edges<std::int64_t,33>();pack_edges<std::int64_t,63>();pack_edges<std::int64_t,64>();
}
