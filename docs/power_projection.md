---
title: "Power Projection"
documentation_of: //fps/power_projection.hpp
library_category: fps
---

$$[x^j]h(x)$$ は $$h(x)$$ の $$x^j$$ の係数を表します。各冪 $$f(x)^0,f(x)^1,\ldots$$ から指定した係数、または重み付きの和を取り出して並べる操作です。

```cpp
#include "lib/fps/power_projection.hpp"

fps f{0, 1, 1}, w{1, 2, 3};
auto a = power_projection(f, w, 4); // {1, 5, 3, 0}
auto b = power_projection(f, 2, 4); // {0, 1, 1, 0}
```

法は 998244353 です。`f`・`w` は変更せず、返り値は `FormalPowerSeries<T>` です。`f` の定数項は任意です。

返り値の添字 `i` は `f` を何乗するかを表します。係数版の第 2 引数を 2 にすると、`f^0,f^1,f^2,...` のそれぞれから $$x^2$$ の係数を取り出して並べます。重み付き版では、それぞれの冪の係数列と `w` の同じ位置同士を掛けて足した値を並べます。

重み数を $$N$$、出力数を $$m$$ とします。係数を一つ指定する形式では、$$N=n+1$$ です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `power_projection(f,w,deg = -1)` | $$\displaystyle a_i=\sum_{j=0}^{N-1}w_j[x^j]f(x)^i$$ を $$0\le i<\mathrm{deg}$$ について返す。$$N=\texttt{w.size()}$$。省略時の長さは `w.size()` | $$f_0=0$$: $$O(N\log^2 N+m)$$、その他: $$O(N\log^2 N+m\log m)$$ |
| `power_projection(f,n,deg = -1)` | $$a_i=[x^n]f(x)^i$$ を返す。$$n\ge0$$。省略時の長さは `n + 1` | $$f_0=0$$: $$O(N\log^2 N+m)$$、その他: $$O(N\log^2 N+m\log m)$$ |

`deg` は返す要素数です。0 なら空列、空の重みなら零列を返します。空の `f` は零多項式として扱い、$$f(x)^0=1$$ とします。重み付き版の `w.size()`、係数版の `n + 1` は $$2^{21}$$ 以下、`deg` は $$2^{22}$$ 以下とします。

$$[x^n]g(x)f(x)^i$$ が必要なら、長さ `n + 1` の重みを $$w_j=g_{n-j}$$ として渡します。`f` と重みの長さを揃える必要はありません。不要な高次係数は参照しません。

時間は $$f_0=0$$ なら $$O(N\log^2 N+m)$$、一般には $$O(N\log^2 N+m\log m)$$ です。追加空間は $$O(N\log N+m)$$ で、Linux x86_64/GCC の AVX2 対応 CPU で専用の計算を使う場合は $$O(N+m)$$ です。AVX2 は複数の整数をまとめて処理する CPU 命令です。
