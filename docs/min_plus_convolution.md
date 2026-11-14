---
title: "凸・凹 min-plus / max-plus 畳み込み"
documentation_of: //convolution/min_plus_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/min_plus_convolution.hpp"

std::vector<long long> a{3, 1, 0, 3}, b{5, 1, 3, 3, 2};
auto c = min_plus_convolution_convex(a, b); // {8,4,2,1,3,3,2,5}
```

min-plus は $$\displaystyle \min_{i+j=k}(a_i+b_j)$$、max-plus は $$\displaystyle \max_{i+j=k}(a_i+b_j)$$ を計算します。

| 関数 | $$a$$ の条件 | 時間計算量 |
| --- | --- | --- |
| `min_plus_convolution_convex(a,b)` | 隣接差が単調非減少 | $$O(L)$$ |
| `max_plus_convolution_concave(a,b)` | 隣接差が単調非増加 | $$O(L)$$ |
| `min_plus_convolution_concave(a,b)` | 隣接差が単調非増加 | $$O(L \log L)$$ |
| `max_plus_convolution_convex(a,b)` | 隣接差が単調非減少 | $$O(L \log L)$$ |

凸か凹かの条件は第 1 引数 `a` に必要です。例えば上の `a` の隣接差は `{-2,-1,3}` で、順に小さくならないので凸です。条件を満たす列を第 1 引数に渡します。両方が非空なら返り値は長さ `a.size()+b.size()-1` の配列、どちらかが空なら空配列です。入力は変更しません。

$$b$$ は任意です。要素と和は有限で、係数型に収まるものとします。$$L = \lvert a\rvert + \lvert b\rvert$$ として、追加空間はいずれも $$O(L)$$ です。
