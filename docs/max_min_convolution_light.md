---
title: "max-min・min-max 畳み込み（軽量実装）"
documentation_of: //convolution/max_min_convolution_light.hpp
library_category: convolution
---

通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法 998244353 の計算には軽量 FPS を使います。

```cpp
#include "lib/convolution/max_min_convolution_light.hpp"

std::vector<long long> a{3, 1, 4}, b{2, 5};
auto c = max_min_convolution(a, b); // {2,3,2,4}
auto d = min_max_convolution(a, b); // {3,2,4,5}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `max_min_convolution(a,b)` | $$\displaystyle c_k=\max_{i+j=k}\min(a_i,b_j)$$ | $$\displaystyle O(L\sqrt{L\log L})$$ |
| `min_max_convolution(a,b)` | $$\displaystyle c_k=\min_{i+j=k}\max(a_i,b_j)$$ | $$\displaystyle O(L\sqrt{L\log L})$$ |

例えば `c[1]` は `max(min(a[0],b[1]),min(a[1],b[0])) = 3` です。返り値は入力と同じ型の配列で、両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。

`max_min_convolution` は $$\displaystyle \max_{i+j=k} \min(a_i,b_j)$$、`min_max_convolution` は $$\displaystyle \min_{i+j=k} \max(a_i,b_j)$$ を計算します。係数には大小比較できる型を使えます。出力長は $$2^{23}$$ 以下です。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、時間 $$\displaystyle O(L \sqrt{L \log L})$$、追加空間 $$O(L)$$ です。
