---
title: "形式的冪級数"
documentation_of: //fps/fps.hpp
---

```cpp
#include "lib/fps/fps.hpp"

fps f{1, 2, 3}, g{4, 5};
fps product = f * g;
fps inverse = f.inv(10);
```

mod は 998244353、係数は定数項から順に格納します。`fps` は `std::vector<mint>` と同様に扱えます。`deg` は返す係数の個数で、省略時は元のサイズです。

| 操作 | 内容・返り値 |
| --- | --- |
| `fps()` / `fps(n)` / `fps{a0,a1,...}` / `fps(vector<mint>)` | 空列／長さ指定／係数列から構築 |
| `f = a` / `f = {a0,a1,...}` | 配列／初期値列を代入 |
| `f[i]` / `f.size()` / `f.resize(n)` / `f.empty()` | `std::vector<mint>` と同じ要素取得・サイズ操作。イテレータや `push_back` なども利用可 |
| `fps::multiply(f,g)` / `f * g` | 多項式の積 |
| `-f` / `f + g` / `f - g` | 符号反転／加算／減算 |
| `f + c` / `f - c` / `f * c` / `f / c` | 定数項への加減算／全係数の定数倍・除算。除数は非零 |
| `f += g` / `f -= g` / `f *= g` / `f /= g` / `f %= g` | 演算結果を代入。スカラーとの複合代入も利用可 |
| `f << d` / `f >> d` / `f <<= d` / `f >>= d` | 係数列をずらし、元のサイズを保つ。`d >= 0` |
| `f.pre(n)` | 先頭から高々 `n` 個の係数を返す |
| `f / g` / `f.div_series(g,deg = -1)` | 級数の除算。$$g_0 \ne 0$$ |
| `f.divmod(g)` / `f % g` | 多項式の商と余り／余り。$$g \ne 0$$ |
| `f.inv(deg = -1)` | 逆数。$$f_0 \ne 0$$ |
| `f.log(deg = -1)` / `f.exp(deg = -1)` | 対数／指数。定数項はそれぞれ 1／0 |
| `f.sqrt(deg = -1)` | 平方根。存在しなければ空配列 |
| `f.pow(k)` | 非負整数乗。元のサイズで打ち切る |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分。元のサイズを保つ |
| `f.eval(x)` | 一点評価 |
| `f.onemul(d,c)` / `f.onediv(d,c)` | $$1+cx^d$$ を掛ける／割る。`d >= 0`、自身のサイズを保つ。除算で `d = 0` なら `c != -1` |

[Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html)、[線形漸化式](https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html)、[合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)、[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html)、[多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html)、[補間](https://qandori.github.io/cp-library/fps/interpolate.hpp.html)、[Taylor shift](https://qandori.github.io/cp-library/fps/taylor_shift.hpp.html) は専用ヘッダーの通常の関数として使います。本ヘッダーには含まれません。

`log`・`exp` は隣接区間の NTT を共有し、Linux x86_64 では AVX2 で積和をまとめます。`pow` は途中の積分と微分を省略し、多項式除算は必要に応じて巡回畳み込みで余りを計算します。Linux x86_64 / GCC の対数と冪は専用のブロック変換を使い、入力の表現変換と結果の二重初期化を省きます。AVX2 が使えない環境でも動作します。

入力・出力の最大係数数を $$n$$ とします。

| 操作 | 時間計算量 |
| --- | --- |
| 加減算・スカラー演算・`diff`・`integral`・`onemul`・`onediv`・一点評価 | $$O(n)$$ |
| 乗算・級数除算・多項式除算・`inv`・`log`・`exp`・`sqrt` | $$O(n \log n)$$ |
| `pow(k)` | $$O(n \log n + \log k)$$ |

追加空間は $$O(n)$$ です。
