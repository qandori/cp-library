---
title: "GCD-plus・LCM-plus 畳み込み"
documentation_of: //convolution/gcd_lcm_convolution.hpp
---

```cpp
#include "lib/convolution/gcd_lcm_convolution.hpp"

std::vector<long long> a{0, 1, 2, 3}, b{0, 4, 5, 6};
auto c = gcd_convolution(a, b);
auto d = lcm_convolution(a, b);
```

`gcd_convolution` は $$c_k = \sum_{\gcd(i,j)=k} a_i b_j$$、`lcm_convolution` は $$c_k = \sum_{\operatorname{lcm}(i,j)=k} a_i b_j$$ を計算します。

係数は環の要素とし、$$a_0 = b_0 = 0$$ で使います。出力長 $$N = \max(\lvert a\rvert,\lvert b\rvert)$$ で、LCM が $$N$$ 以上になる項は出力しません。

時間 $$O(N \log \log(N))$$、追加空間 $$O(N)$$ です。
