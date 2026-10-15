# 形式的冪級数

```cpp
#include "lib/math/fps.hpp"

fps f{1, 2, 3}, g{4, 5};
fps product = f * g;
fps inverse = f.inv(10);
```

mod は 998244353、係数は定数項から順に格納します。`fps` は `vector<mint>` と同様に扱えます。`deg` は返す係数の個数で、省略時は元のサイズです。

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
