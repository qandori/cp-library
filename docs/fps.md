---
title: "形式的冪級数"
documentation_of: //math/fps.hpp
---

```cpp
#include "lib/math/fps.hpp"

fps f{1, 2, 3}, g{4, 5};
fps product = f * g;
fps inverse = f.inv(10);
```

mod は 998244353、係数は定数項から順に格納します。`fps` は `std::vector<mint>` と同様に扱えます。`deg` は返す係数の個数で、省略時は元のサイズです。

| 操作 | 内容・条件 |
| --- | --- |
| `f / g` / `f.div_series(g, deg)` | 級数の除算。`g[0] != 0` |
| `f.divmod(g)` / `f % g` | 多項式の商と余り／余り。`g != 0` |
| `f.inv(deg)` | 逆数。`f[0] != 0` |
| `f.log(deg)` / `f.exp(deg)` | 対数／指数。定数項はそれぞれ 1／0 |
| `f.sqrt(deg)` | 平方根。存在しなければ空配列 |
| `f.pow(k)` | 非負整数乗。元のサイズで打ち切る |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分。元のサイズを保つ |
| `f.compose(g, deg)` | 合成 `f(g(x))`。`g[0] == 0` |
| `f.compositional_inverse(deg)` | 合成逆元。`f[0] == 0`、`f[1] != 0` |
| `f.eval(x)` / `f.eval(xs)` | 一点／多点評価 |
| `fps::interpolate(xs, ys)` | 相異なる点からの補間 |
| `f.shift(c)` | `f(x+c)` に自身を書き換える |
| `f.onemul(d, c)` / `f.onediv(d, c)` | `1+c*x^d` を掛ける／割る。自身のサイズを保つ |
| `fps::bostan_mori(p, q, k)` | `p/q` の `x^k` の係数。`q[0] != 0`、`k >= 0` |
| `fps::linear_recurrence(initial, c, k)` | `a[n] = c[0]*a[n-1] + ... + c[d-1]*a[n-d]` の `a[k]`。初期値・係数は各 `d` 個 |

`n` は入力・出力の最大係数数、`m` は評価点数、`d` は漸化式の次数（Bostan–Mori では `max(p.size(), q.size())`）とします。

| 操作 | 時間計算量 |
| --- | --- |
| 加減算・スカラー演算・`diff`・`integral`・`onemul`・`onediv`・一点評価 | `O(n)` |
| 乗算・級数除算・多項式除算・`inv`・`log`・`exp`・`sqrt`・`shift` | `O(n log n)` |
| `pow(k)` | `O(n log n + log(k+1))` |
| `compose`・`compositional_inverse` | `O(n log^2 n)` |
| 多点評価 | `O((n+m) log^2(n+m))` |
| `interpolate` | `O(m log^2 m)` |
| `bostan_mori`・`linear_recurrence` | `O(d log(d+1) log(k+2))` |

追加空間は、合成・多点評価・補間が `O(N log N)`、その他が `O(N)` です（`N = max(n, m, d)`）。
