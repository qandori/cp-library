---
title: "GCD-plus・LCM-plus 畳み込み"
documentation_of: //convolution/gcd_lcm_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/gcd_lcm_convolution.hpp"

std::vector<long long> a{0, 1, 2, 3}, b{0, 4, 5, 6};
auto c = gcd_convolution(a, b); // {0,62,10,18}
auto d = lcm_convolution(a, b); // {0,4,23,36}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `gcd_convolution(a,b)` | $$\displaystyle c_k=\sum_{\gcd(i,j)=k}a_ib_j$$ | $$O(N\log\log N)$$ |
| `lcm_convolution(a,b)` | $$\displaystyle c_k=\sum_{\operatorname{lcm}(i,j)=k}a_ib_j$$ | $$O(N\log\log N)$$ |

値 1 からの配列を、先頭にダミーの 0 を置いて渡します。例えば `a[2]` は添字 2 の重みです。GCD 版では積 `a[2]*b[3]` を `c[1]` に加え、LCM 版では添字 6 が出力範囲内にある場合だけ加えます。入力を変更せず、入力と同じ係数型の配列を返します。

`gcd_convolution` は $$\displaystyle c_k = \sum_{\gcd(i,j)=k} a_i b_j$$、`lcm_convolution` は $$\displaystyle c_k = \sum_{\operatorname{lcm}(i,j)=k} a_i b_j$$ を計算します。

係数には加減算・乗算ができ、通常の分配法則などを満たす型（整数や modint など）を使い、$$a_0 = b_0 = 0$$ で使います。出力長 $$N = \max(\lvert a\rvert,\lvert b\rvert)$$ で、LCM が $$N$$ 以上になる項は出力しません。

時間 $$O(N \log \log N)$$、追加空間 $$O(N)$$ です。
