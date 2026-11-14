---
title: "Bitwise Zeta / Möbius Transform"
documentation_of: //components/bitwise_zeta.hpp
redirect_from: /convolution/bitwise_zeta.hpp.html
library_category: components
---

```cpp
#include "lib/components/bitwise_zeta.hpp"

std::vector<long long> a{1, 2, 3, 4};
bitwise_zeta(a); // {1,3,4,10}
bitwise_zeta<true>(a); // {1,2,3,4}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `bitwise_zeta<inverse = false,superset = false>(a)` | 配列を部分集合ゼータ変換。`superset = true` は上位集合、`inverse = true` はメビウス変換 | $$O(n\log n)$$ |

添字を集合の bitmask として扱います。既定の変換後は `a[S]` が元の `a[T]` のうち `T` が `S` の部分集合であるものの和になります。例の添字 3 は `{0,1}` なので全 4 要素の和 10 です。返り値は `void` で、渡した配列が書き換わります。元に戻すときも、同じ `superset` の指定で逆変換します。

`bitwise_zeta<inverse = false, superset = false>(a)` は部分集合和、`superset = true` なら上位集合和を計算します。`inverse = true` は逆変換です。長さは 2 の冪とします。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。要素の演算は $$O(1)$$ とします。
