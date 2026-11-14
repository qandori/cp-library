---
title: "軽量 FPS の Power Projection"
documentation_of: //fps/power_projection_light.hpp
library_category: fps
---

$$[x^j]h(x)$$ は $$h(x)$$ の $$x^j$$ の係数を表します。各冪 $$f(x)^0,f(x)^1,\ldots$$ から指定した係数、または重み付きの和を取り出して並べる操作です。

```cpp
#include "lib/fps/power_projection_light.hpp"

fps f{0, 1, 1}, w{1, 2, 3};
auto a = power_projection(f, w, 4); // {1, 5, 3, 0}
auto b = power_projection(f, 2, 4); // {0, 1, 1, 0}
```

返り値は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) の `FormalPowerSeries<T>` です。定数項は任意、入力は変更しません。高速実装の FPS や専用 NTT には依存しません。

重み数を $$N$$、出力数を $$m$$ とします。係数を一つ指定する形式では、$$N=n+1$$ です。

| 操作 | 内容 | 時間計算量 |
| --- | --- | --- |
| `power_projection(f,w,deg = -1)` | $$\displaystyle a_i=\sum_{j=0}^{N-1}w_j[x^j]f(x)^i$$ を返す。$$N=\texttt{w.size()}$$ | $$f_0=0$$: $$O(N\log^2 N+m)$$、その他: $$O(N\log^2 N+m\log m)$$ |
| `power_projection(f,n,deg = -1)` | $$a_i=[x^n]f(x)^i$$ を返す。$$n\ge0$$ | $$f_0=0$$: $$O(N\log^2 N+m)$$、その他: $$O(N\log^2 N+m\log m)$$ |

`deg` は返す要素数で、省略時は重み付き版が `w.size()`、係数版が `n + 1` です。0 なら空列、空の重みなら零列を返します。空の `f` は零多項式とし、$$f(x)^0=1$$ とします。

重み数と係数版の `n + 1` は $$2^{21}$$ 以下、`deg` は $$2^{22}$$ 以下とします。法や係数型の条件は軽量 FPS と同じです。

時間は $$f_0=0$$ なら $$O(N\log^2 N+m)$$、一般には $$O(N\log^2 N+m\log m)$$、追加空間は $$O(N\log N+m)$$ です。
