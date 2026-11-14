---
title: "AND・OR・XOR 畳み込み"
documentation_of: //convolution/bitwise_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/bitwise_convolution.hpp"

std::vector<long long> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = and_convolution(a, b); // {103,52,73,32}
auto d = or_convolution(a, b); // {5,28,43,184}
auto f = xor_convolution(a, b); // {70,68,62,60}
```

| 関数 | 定義 | 計算量 |
| --- | --- | --- |
| `and_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\&}j=k} a_i b_j$$ | $$O(N\log N)$$ |
| `or_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\vert}j=k} a_i b_j$$ | $$O(N\log N)$$ |
| `xor_convolution(a, b)` | $$\displaystyle c_k = \sum_{i\mathbin{\oplus}j=k} a_i b_j$$ | $$O(N\log N)$$ |

添字 `i,j` に AND・OR・XOR を適用し、その添字へ積 `a[i]*b[j]` を加えます。入力を変更せず、新しい配列を返します。短い配列の不足分は 0 で埋めます。どちらかが空なら空配列です。

長い方の長さを 2 の冪に切り上げた $$N$$ 個を返します。係数には加減算・乗算ができる型を使います。各演算は通常の整数と同じように括弧の位置を変えられ、加算の順序を変えられ、乗算を和に分配できることが必要です。XOR ではさらに $$N$$ で割れる型を使います。整数型では中間値も型の範囲内とします。

`bitwise_zeta<inverse, superset>(a)` は、各集合の部分集合または上位集合に対応する要素の和を計算します。`inverse=true` はその逆変換です。既定は `<false, false>`、長さは 2 の冪です。詳しくは[部分集合の和を計算する変換](bitwise_zeta.md)を参照してください。`walsh_hadamard(a)` は、出力先の添字 $$k$$ と入力元の添字 $$j$$ の AND で、1 の bit の個数が偶数なら $$a_j$$ を足し、奇数なら引く変換です。同じ変換を 2 回行うと全要素が元の値の $$N$$ 倍になるので、逆変換には $$N$$ での除算も必要です。

時間 $$O(N \log N)$$、追加空間 $$O(N)$$ です。変換のみの追加空間は $$O(1)$$ です。
