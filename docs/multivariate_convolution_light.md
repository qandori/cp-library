---
title: "多変数の切り捨て畳み込み（軽量実装）"
documentation_of: //convolution/multivariate_convolution_light.hpp
---

`light_convolution` 名前空間で呼び出します。法 998244353 の計算は軽量 NTT を使い、FPS 本体は読み込みません。高速実装と同時に include できます。64 KiB の展開サイズに収めるため、小さい次元向けの ranked AVX2 専用処理は通常版のみで使います。その対象となる入力では軽量版の方が遅くなることがあるため、速度を優先する場合は通常版を使ってください。

```cpp
#include "lib/convolution/multivariate_convolution_light.hpp"

std::vector<mint> a{1, 2, 3, 4, 5, 6}, b{7, 8, 9, 10, 11, 12};
auto c = light_convolution::multivariate_convolution(a, b, {2, 3}); // {7,22,30,80,73,182}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_convolution::multivariate_convolution(a,b,dimensions)` | 各変数の次数上限を `dimensions` で指定して、切り捨てた積を返す | 998244353: $$O(K+DN\log N+D^2N)$$、その他: $$O(K+M(ND))$$ |

例の `{2,3}` は、$$x$$ の指数を 0〜1、$$y$$ の指数を 0〜2 に制限する指定です。係数を $$1,x,y,xy,y^2,xy^2$$ の順に並べた長さ 6 の配列を渡し、同じ順序・長さで返します。積の $$x^2$$ 以上や $$y^3$$ 以上の項は捨てます。

多項式の積を $$(x_0^{d_0},\ldots,x_s^{d_s})$$ で切り捨てます。各 $$d > 0$$、入力長は $$\displaystyle N = \prod_{j=0}^{s} d_j$$ とし、添字は $$i_0 + d_0 i_1 + d_0 d_1 i_2 + \cdots$$ の順です。

入力の次元数を $$K$$、1 より大きい次元数を $$D$$、通常の長さ $$L$$ の畳み込み時間を $$M(L)$$ とします。mod 998244353 では $$N \le 2^{22}$$、時間 $$O(K + DN \log N + D^{2}N)$$、その他は時間 $$O(K + M(ND))$$ です。係数型・変換長の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じで、追加空間はいずれも $$O(ND)$$ です。$$N=1$$ は時間 $$O(K)$$、追加空間 $$O(1)$$ です。
