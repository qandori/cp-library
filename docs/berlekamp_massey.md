---
title: 線形漸化式の復元（Berlekamp–Massey）
documentation_of: //fps/berlekamp_massey.hpp
library_category: fps
---

初めの $$d$$ 項を初期値とし、それ以降の各項を直前の $$d$$ 項の定数倍の和で表す漸化式を求めます。この $$d$$ を位数と呼び、位数が最小になるものを 1 つ返します。

```cpp
#include "lib/fps/berlekamp_massey.hpp"
#include "lib/math/mint.hpp"

std::vector<mint> a{0, 1, 1, 2, 3, 5, 8, 13};
auto c = berlekamp_massey(a); // {1,1}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `berlekamp_massey(a)` | 最小位数の漸化式の係数を `vector<T>` で返す | $$O(N^2)$$ |

返り値の長さを $$d$$ とすると、入力の全ての $$d\le i<N$$ について次が成り立ちます。

$$\displaystyle a_i=\sum_{j=0}^{d-1}c_j a_{i-j-1}$$

`c[0]` は 1 つ前の項の係数です。法 998244353 なら[線形漸化式の第 k 項](linear_recurrence.md)にそのまま渡せます。次は上の `a`・`c` を使う例です。初期値は `a` の先頭 `c.size()` 項を渡してください。

```cpp
#include "lib/fps/linear_recurrence.hpp"

std::vector<mint> initial(a.begin(), a.begin() + c.size());
auto value = linear_recurrence(initial, c, 10); // 55
```

`T` は正確な四則演算と 0 の等値判定ができ、0 でない全ての値で割れる型を使います。このような数の集まりを体と呼びます。`mint`、`atcoder::modint1000000007` などの素数 modint、有理数型に対応します。整数型や合成数 mod の modint は対象外です。入力長は `INT_MAX` 未満としてください。

空の入力や全て 0 の数列には空の係数列を返します。位数に必要な末尾の 0 は残します。例えば `{1,0,0}` に対する返り値は `{0}` です。

解が複数ある場合はそのうち 1 つを返します。元の無限数列の位数が $$D$$ 以下と分かっていれば、先頭 $$2D$$ 項以上から後続項も定まります。

入力長を $$N$$、返す位数を $$d$$ とし、体の演算を $$O(1)$$ とすると、時間は $$O(Nd)$$、最悪 $$O(N^2)$$、追加空間は $$O(N)$$ です。全て 0 の数列は $$O(N)$$ 時間です。

参考：[AtCoder Algorithm Lectures：線形漸化式の復元](https://info.atcoder.jp/entry/algorithm_lectures/linearly_recurrent_sequence_reconstruction)
