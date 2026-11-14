---
title: "Subset 畳み込み"
documentation_of: //convolution/subset_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/subset_convolution.hpp"

std::vector<long long> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = subset_convolution(a, b); // {5,16,22,60}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `subset_convolution(a,b)` | $$\displaystyle c_S=\sum_{A\subseteq S}a_Ab_{S\setminus A}$$ | $$O(N\log^2 N)$$ |

添字を bitmask として扱い、重ならない 2 集合に分けた全ての組の積を加えます。例の添字 3 は集合 `{0,1}` で、`c[3] = a[0]*b[3]+a[1]*b[2]+a[2]*b[1]+a[3]*b[0] = 60` です。入力を変更せず、新しい配列を返します。足りない係数は 0 とし、どちらかが空なら空配列です。

$$\displaystyle c_S = \sum_{A\subseteq S} a_A b_{S\setminus A}$$。添字を部分集合の bit 表現として扱います。係数には加減算・乗算ができ、通常の分配法則などを満たす型（整数や modint など）を使い、長い方の長さを 2 の冪に切り上げた $$N$$ 個を返します。

時間 $$O(N \log^{2} N)$$、追加空間 $$O(N \log N)$$ です。
