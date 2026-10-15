---
title: "Subset 畳み込み"
documentation_of: //convolution/subset_convolution.hpp
---

```cpp
#include "lib/convolution/subset_convolution.hpp"

std::vector<long long> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = subset_convolution(a, b);
```

$$c_S = \sum_{A\subseteq S} a_A b_{S\setminus A}$$。添字を部分集合の bit 表現として扱います。係数は環の要素とし、長い方の長さを 2 の冪に切り上げた $$N$$ 個を返します。

時間 $$O(N \log^{2} N)$$、追加空間 $$O(N \log N)$$ です。
