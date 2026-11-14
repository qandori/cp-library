---
title: "Montgomery 64"
documentation_of: //components/montgomery.hpp
redirect_from: /math/montgomery.hpp.html
library_category: components/math
---

奇数の法で加算・乗算・累乗を行う部品です。法を $$m$$、$$R=2^{64}$$ として、整数 $$a$$ を $$aR\bmod m$$ に変換した値を「Montgomery 表現」と呼びます。`in` でこの表現へ変換し、そのまま `add`・`mul`・`pow` で計算してから、`reduce` で通常の余りに戻します。

```cpp
#include "lib/components/montgomery.hpp"

Montgomery64 m(1000000007);
auto a = m.in(12), b = m.in(34);
auto value = m.reduce(m.mul(a, b));
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Montgomery64(mod)` | 64 bit の奇数法を指定して構築 | $$O(1)$$ |
| `in(x)` | 通常の整数を Montgomery 表現に変換 | $$O(1)$$ |
| `reduce(x)` | Montgomery 縮約。`reduce(in(a))` で通常の整数に戻す | $$O(1)$$ |
| `add(a,b)` / `mul(a,b)` | Montgomery 表現で加算／乗算 | $$O(1)$$ |
| `pow(a,k)` | Montgomery 表現で非負整数乗 | $$O(\log k)$$ |

法は 1 より大きい 64 bit 符号なし奇数です。`in(x)` は任意の `std::uint64_t` を受け取り、`x % mod` の Montgomery 表現に変換します。`reduce(x)` で通常の値に戻します。`add(a,b)`・`mul(a,b)`・`pow(a,k)` の入力は法未満の Montgomery 表現とします。

構築・変換・加算・乗算は $$O(1)$$、累乗は $$O(\log k)$$、空間は $$O(1)$$ です。
