---
title: "AND・OR・XOR 畳み込み"
documentation_of: //convolution/bitwise_convolution.hpp
---

```cpp
#include "lib/convolution/bitwise_convolution.hpp"

std::vector<long long> a{1, 2, 3, 4}, b{5, 6, 7, 8};
auto c = and_convolution(a, b);
```

| 関数 | 定義 |
| --- | --- |
| `and_convolution(a, b)` | `c[k] = Σ_{i&j=k} a[i]*b[j]` |
| `or_convolution(a, b)` | `c[k] = Σ_{i\|j=k} a[i]*b[j]` |
| `xor_convolution(a, b)` | `c[k] = Σ_{i^j=k} a[i]*b[j]` |

長い方の長さを 2 の冪に切り上げた `N` 個を返します。係数は環の要素、XOR ではさらに `N` で割れる型を使います。整数型では中間値も型の範囲内とします。

`bitwise_zeta<inverse, superset>(a)` はその場でゼータ／メビウス変換します。既定は `<false, false>`、長さは 2 の冪です。`walsh_hadamard(a)` は非正規化の Hadamard 変換です。

時間 `O(N log N)`、追加空間 `O(N)` です。変換のみの追加空間は `O(1)` です。
