---
title: "AND-OR などの論理畳み込み"
documentation_of: //convolution/boolean_convolution.hpp
---

```cpp
#include "lib/convolution/boolean_convolution.hpp"

std::vector<unsigned> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = and_or_convolution(a, b);
```

`*_or_convolution` は $$c_k = \bigvee_{i\circ j=k}(a_i \land b_j)$$、`*_and_convolution` は $$c_k = \bigwedge_{i\circ j=k}(a_i \lor b_j)$$ です。各 bit を独立に計算します。

| 添字 $$\circ$$ | OR で集約 | AND で集約 |
| --- | --- | --- |
| $$i\mathbin{\&}j$$ | `and_or_convolution` | `and_and_convolution` |
| $$i\mathbin{\vert}j$$ | `or_or_convolution` | `or_and_convolution` |
| $$i\mathbin{\oplus}j$$ | `xor_or_convolution` | `xor_and_convolution` |
| $$\gcd(i,j)$$ | `gcd_or_convolution` | `gcd_and_convolution` |
| $$\operatorname{lcm}(i,j)$$ | `lcm_or_convolution` | `lcm_and_convolution` |
| $$i\mathbin{\vert}j$$、$$i\mathbin{\&}j = 0$$ | `subset_or_convolution` | `subset_and_convolution` |

係数は `bool` または 64 bit 以下の整数です。GCD・LCM では添字 0 を使用しません。出力長は GCD・LCM が長い方の長さ、それ以外はそれを 2 の冪に切り上げた長さ $$N$$ です。範囲外の LCM は出力しません。$$N \le 2^{30}$$ とします。

係数の bit 数を $$W$$ として、時間は AND・OR・XOR が $$O(WN \log N)$$、GCD・LCM が $$O(WN \log \log(N))$$、Subset が $$O(WN \log^{2} N)$$ です。追加空間は Subset が $$O(N \log N)$$、その他が $$O(N)$$ です。
