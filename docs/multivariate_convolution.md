---
title: "多変数の切り捨て畳み込み"
documentation_of: //convolution/multivariate_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/multivariate_convolution.hpp"

std::vector<mint> a{1, 2, 3, 4, 5, 6}, b{7, 8, 9, 10, 11, 12};
auto c = multivariate_convolution(a, b, {2, 3}); // {7,22,30,80,73,182}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `multivariate_convolution(a,b,dimensions)` | 各変数の次数上限を `dimensions` で指定して、切り捨てた積を返す | 998244353: $$O(K+DN\log N+D^2N)$$、その他: $$O(K+M(ND))$$ |

例の `{2,3}` は、$$x$$ の指数を 0〜1、$$y$$ の指数を 0〜2 に制限する指定です。係数を $$1,x,y,xy,y^2,xy^2$$ の順に並べた長さ 6 の配列を渡し、同じ順序・長さで返します。積の $$x^2$$ 以上や $$y^3$$ 以上の項は捨てます。

変数の個数を $$K=\texttt{dimensions.size()}$$、各変数の上限を $$d_j=\texttt{dimensions[j]}$$ とします。積の各項について、どれか一つでも変数 $$x_j$$ の指数が $$d_j$$ 以上なら、その項を捨てます。各 $$d_j>0$$、入力長は $$\displaystyle N = \prod_{j=0}^{K-1} d_j$$ とし、添字は $$i_0 + d_0 i_1 + d_0 d_1 i_2 + \cdots$$ の順です。各指数は $$0\le i_j<d_j$$ とします。

$$d_j>1$$ となる変数の個数を $$D$$、通常の長さ $$L$$ の畳み込み時間を $$M(L)$$ とします。mod 998244353 では $$N \le 2^{22}$$、時間 $$O(K + DN \log N + D^{2}N)$$、その他は時間 $$O(K + M(ND))$$ です。係数型・変換長の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じで、追加空間はいずれも $$O(ND)$$ です。$$N=1$$ は時間 $$O(K)$$、追加空間 $$O(1)$$ です。
