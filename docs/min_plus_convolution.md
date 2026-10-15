---
title: "凸・凹 min-plus / max-plus 畳み込み"
documentation_of: //convolution/min_plus_convolution.hpp
---

```cpp
#include "lib/convolution/min_plus_convolution.hpp"

std::vector<long long> a{3, 1, 0, 3}, b{5, 1, 3, 3, 2};
auto c = min_plus_convolution_convex(a, b);
```

min-plus は `min_{i+j=k}(a[i]+b[j])`、max-plus は `max_{i+j=k}(a[i]+b[j])` を計算します。

| 関数 | `a` の条件 | 時間計算量 |
| --- | --- | --- |
| `min_plus_convolution_convex` | 隣接差が単調非減少 | `O(L)` |
| `max_plus_convolution_concave` | 隣接差が単調非増加 | `O(L)` |
| `min_plus_convolution_concave` | 隣接差が単調非増加 | `O(L log L)` |
| `max_plus_convolution_convex` | 隣接差が単調非減少 | `O(L log L)` |

`b` は任意です。要素と和は有限で、係数型に収まるものとします。`L = a.size()+b.size()` として、追加空間はいずれも `O(L)` です。
