---
title: "多項式補間"
documentation_of: //fps/interpolate.hpp
library_category: fps
---

```cpp
#include "lib/fps/interpolate.hpp"

std::vector<mint> xs{0, 1, 2}, ys{1, 6, 17};
auto f = interpolate(xs, ys); // {1,2,3}
```

法は 998244353 です。`xs` と `ys` の長さは等しく、`xs` の値は法上で相異なる必要があります。入力は変更しません。

`xs[i]` と `ys[i]` を一組の点として渡します。例では `(0,1),(1,6),(2,17)` を通る $$1+2x+3x^2$$ が求まります。返り値の `f[i]` は $$x^i$$ の係数で、長さは点数と同じです。点の順番は任意ですが、同じ x 座標を複数渡すことはできません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `interpolate(xs,ys)` | $$f(x_i)=y_i$$ を満たす次数 $$n$$ 未満の多項式を `FormalPowerSeries<T>` で返す。空の入力は空配列 | $$O(n\log^2 n)$$ |

点数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n\log n)$$ です。
