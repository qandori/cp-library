---
title: "max-min・min-max 畳み込み"
documentation_of: //convolution/max_min_convolution.hpp
---

```cpp
#include "lib/convolution/max_min_convolution.hpp"

std::vector<long long> a{3, 1, 4}, b{2, 5};
auto c = max_min_convolution(a, b);
auto d = min_max_convolution(a, b);
```

`max_min_convolution` は `max_{i+j=k} min(a[i], b[j])`、`min_max_convolution` は `min_{i+j=k} max(a[i], b[j])` を計算します。係数には大小比較できる型を使えます。出力長は `2^23` 以下です。

`L = a.size()+b.size()` として、時間 `O(L sqrt(L log L))`、追加空間 `O(L)` です。
