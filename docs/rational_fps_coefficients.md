---
title: 有理 FPS の区間係数取得
documentation_of: //fps/rational_fps_coefficients.hpp
library_category: fps
---

$$[x^k]h(x)$$ は、$$h(x)$$ の $$x^k$$ の係数を表します。ここでは多項式の商 $$p(x)/q(x)$$ を、$$q(x)$$ と掛けると $$p(x)$$ になる係数列として扱います。

有理式で表された FPS の連続する係数をまとめて求めます。

```cpp
#include "lib/fps/rational_fps_coefficients.hpp"

fps p{1}, q{1, -1, -1};
auto a = rational_fps_coefficients(p, q, 10, 15);
// {89, 144, 233, 377, 610}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `rational_fps_coefficients(p,q,l,r)` | $$p(x)/q(x)$$ の $$x^l,x^{l+1},\ldots,x^{r-1}$$ の係数を順に返す | $$O(N\log N\log r+(N+M)\log(N+M))$$ |

区間は右端を含まない `[l,r)` です。返り値は長さ `r-l` の `fps` で、`a[i]` は $$[x^{l+i}]p(x)/q(x)$$ です。`l==r` なら空の FPS を返し、係数が 0 の部分も省略しません。

係数型は `fps` と同じ法 998244353 の modint です。`p[i]`・`q[i]` は $$x^i$$ の係数で、`q` は空でなく `q[0]!=0` としてください。分子の次数が分母以上でも、`q[0]` が 1 でなくても使えます。

`l,r` は `long long` で $$0\le l\le r$$ とし、$$N=\max(\lvert p\rvert,\lvert q\rvert)$$、$$M=r-l$$ に対して $$N+M\le2^{22}$$ としてください。開始位置が大きい区間にも対応します。引数は値渡しなので、不要になった配列は `std::move` で渡せます。

初期値と漸化式の係数から使う場合は、次のように分子と分母を作ります。

```cpp
#include "lib/fps/rational_fps_coefficients.hpp"

std::vector<mint> initial{0, 1}, coefficients{1, 1};
int d = initial.size();
fps q(d + 1);
q[0] = 1;
for (int i = 0; i < d; ++i) q[i + 1] = -coefficients[i];
fps p = fps::multiply(fps(initial), q).pre(d);
auto a = rational_fps_coefficients(p, q, 10, 15);
// {55, 89, 144, 233, 377}
```

初期値と係数列の長さは同じにし、`coefficients[0]` に 1 つ前の項の係数を入れます。1 項だけなら [Bostan–Mori](bostan_mori.md) や [線形漸化式の第 k 項](linear_recurrence.md) も使えます。

時間は $$O(N\log N\log r+(N+M)\log(N+M))$$、追加空間は $$O(N\log r+M)$$ です。先頭付近の取得は $$O((N+M)\log(N+M))$$ 時間です。$$r=1$$ は $$O(N+M)$$ 時間・追加空間、空区間は引数のコピーを含めて $$O(N)$$ 時間・追加空間以内です。

参考：[AtCoder Algorithm Lectures：Bostan–Mori の発展](https://info.atcoder.jp/entry/algorithm_lectures/bostan_mori_advanced)
