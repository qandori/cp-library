---
title: 有理 FPS の区間係数取得（軽量実装）
documentation_of: //fps/rational_fps_coefficients_light.hpp
library_category: fps
---

$$[x^k]h(x)$$ は、$$h(x)$$ の $$x^k$$ の係数を表します。ここでは多項式の商 $$p(x)/q(x)$$ を、$$q(x)$$ と掛けると $$p(x)$$ になる係数列として扱います。

軽量 FPS を使い、有理式で表された FPS の連続する係数をまとめて求めます。

```cpp
#include "lib/fps/rational_fps_coefficients_light.hpp"

fps p{1}, q{1, -1, -1};
auto a = rational_fps_coefficients(p, q, 10, 15);
// {89, 144, 233, 377, 610}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `rational_fps_coefficients(p,q,l,r)` | $$p(x)/q(x)$$ の $$x^l,x^{l+1},\ldots,x^{r-1}$$ の係数を順に返す | $$O(N\log N\log r+(N+M)\log(N+M))$$ |

区間は右端を含まない `[l,r)` です。返り値は長さ `r-l` の `FormalPowerSeries<T>` で、`a[i]` は $$[x^{l+i}]p(x)/q(x)$$ です。空区間には空の FPS を返し、係数が 0 の部分も残します。

係数型は法 998244353 の modint です。`q` は空でなく `q[0]!=0` としてください。分子の次数が分母以上でも、分母の定数項が 1 でなくても使えます。

`l,r` は `long long` で $$0\le l\le r$$ とし、$$N=\max(\lvert p\rvert,\lvert q\rvert)$$、$$M=r-l$$ に対して $$N+M\le2^{22}$$ としてください。開始位置が大きい区間にも対応します。引数は値渡しなので `std::move` も使えます。

初期値と漸化式からの使い方は [通常版の説明](rational_fps_coefficients.md) を参照してください。include 先を `fps/rational_fps_coefficients_light.hpp` に切り替えると、同じコードで使えます。

時間は $$O(N\log N\log r+(N+M)\log(N+M))$$、追加空間は $$O(N\log r+M)$$ です。先頭付近の取得は $$O((N+M)\log(N+M))$$ 時間です。$$r=1$$ は $$O(N+M)$$ 時間・追加空間、空区間は引数のコピーを含めて $$O(N)$$ 時間・追加空間以内です。
