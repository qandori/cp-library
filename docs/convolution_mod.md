---
title: "任意 mod・整数の畳み込み"
documentation_of: //convolution/convolution_mod.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/convolution_mod.hpp"

std::vector<long long> a{1, 2, 3}, b{4, 5};
auto c = convolution_mod(a, b, 1000000007); // {4,13,22,15}
auto exact = convolution_integer(a, b); // {4,13,22,15}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convolution_mod(a,b,mod)` | 通常の畳み込みを指定法で計算し、`vector<uint32_t>` を返す | $$O(L\log L)$$ |
| `convolution_integer(a,b)` | 整数の通常の畳み込みを厳密計算し、入力と同じ係数型で返す | $$O(L\log L)$$ |

どちらも $$\displaystyle \sum_{i+j=k} a_i b_j$$ を計算します。入力は 64 bit 以下の整数型です。

各配列を定数項から順に並べた多項式の係数として渡します。両方が非空なら返す長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。法を指定する版は各係数をその法で割った非負の余りにし、整数版は余りを取らずに返します。

`convolution_mod` は $$1 \le \mathrm{mod} \le 2^{32}-1$$ とし、`std::vector<std::uint32_t>` を返します。`convolution_integer` は入力と同じ係数型を返し、結果がその型に収まる必要があります。出力長は $$2^{24}$$ 以下です。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、時間 $$O(L \log L)$$、追加空間 $$O(L)$$ です。
