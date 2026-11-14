---
title: "Walsh–Hadamard Transform"
documentation_of: //components/walsh_hadamard.hpp
redirect_from: /convolution/walsh_hadamard.hpp.html
library_category: components
---

```cpp
#include "lib/components/walsh_hadamard.hpp"

std::vector<long long> a{1, 2, 3, 4};
walsh_hadamard(a); // {10,-2,-4,0}
walsh_hadamard(a);
for (auto &x : a) x /= 4; // {1,2,3,4}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `walsh_hadamard(a)` | 配列長による除算を行わない Walsh–Hadamard 変換。入力を変換結果で上書きする | $$O(n\log n)$$ |

長さは 2 の冪とします。同じ変換をもう一度適用し、各要素を長さで割ると逆変換になります。

引数は `vector<T>` で、戻り値は `void`、元の配列を直接書き換えます。要素型は加算・減算に対応させます。逆変換では長さによる除算も必要です。元に戻す場合は、例の 4 を実際の配列長に置き換えます。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。要素の演算は $$O(1)$$ とします。
