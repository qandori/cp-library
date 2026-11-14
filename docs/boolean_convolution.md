---
title: "AND-OR などの論理畳み込み"
documentation_of: //convolution/boolean_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/boolean_convolution.hpp"

std::vector<unsigned> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = and_or_convolution(a, b); // {7,6,7,0}
```

`*_or_convolution` は $$\displaystyle c_k = \bigvee_{i\circ j=k}(a_i \land b_j)$$、`*_and_convolution` は $$\displaystyle c_k = \bigwedge_{i\circ j=k}(a_i \lor b_j)$$ です。各 bit を独立に計算します。

| 添字 $$\circ$$ | OR で集約 | AND で集約 | 計算量 |
| --- | --- | --- | --- |
| $$i\mathbin{\&}j$$ | `and_or_convolution(a,b)` | `and_and_convolution(a,b)` | $$O(WN\log N)$$ |
| $$i\mathbin{\vert}j$$ | `or_or_convolution(a,b)` | `or_and_convolution(a,b)` | $$O(WN\log N)$$ |
| $$i\mathbin{\oplus}j$$ | `xor_or_convolution(a,b)` | `xor_and_convolution(a,b)` | $$O(WN\log N)$$ |
| $$\gcd(i,j)$$ | `gcd_or_convolution(a,b)` | `gcd_and_convolution(a,b)` | $$O(WN\log\log N)$$ |
| $$\operatorname{lcm}(i,j)$$ | `lcm_or_convolution(a,b)` | `lcm_and_convolution(a,b)` | $$O(WN\log\log N)$$ |
| $$i\mathbin{\vert}j$$、$$i\mathbin{\&}j = 0$$ | `subset_or_convolution(a,b)` | `subset_and_convolution(a,b)` | $$O(WN\log^2 N)$$ |

関数名の前半は添字の操作、後半は係数をまとめる操作です。`and_or_convolution` なら、全ての組 `i,j` について `c[i & j] |= a[i] & b[j]` とする結果になります。AND で集約する版は、各出力先を全 bit が 1 の値で初期化してから `&= a[i] | b[j]` します。該当する組がない添字には、その初期値が残ります。

係数は `bool` または 64 bit 以下の整数です。GCD・LCM では添字 0 を使用しません。出力長は GCD・LCM が長い方の長さ、それ以外はそれを 2 の冪に切り上げた長さ $$N$$ です。範囲外の LCM は出力しません。$$N \le 2^{30}$$ とします。

係数の bit 数を $$W$$ として、時間は AND・OR・XOR が $$O(WN \log N)$$、GCD・LCM が $$O(WN \log \log N)$$、Subset が $$O(WN \log^{2} N)$$ です。追加空間は Subset が $$O(N \log N)$$、その他が $$O(N)$$ です。
