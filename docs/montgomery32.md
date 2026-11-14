---
title: "Montgomery 32"
documentation_of: //components/montgomery32.hpp
library_category: components/math
---

32 bit の奇数法で加算・乗算・累乗を行う部品です。法を $$m$$、$$R=2^{32}$$ として、整数 $$a$$ を $$aR\bmod m$$ に変換した値を「Montgomery 表現」と呼びます。`in` で変換してから `add`・`mul`・`pow` を使い、最後に `reduce` で通常の余りへ戻します。

```cpp
#include "lib/components/montgomery32.hpp"

Montgomery32 m(1000000007);
auto x = m.in(12), y = m.in(34);
auto product = m.reduce(m.mul(x, y));
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Montgomery32(mod)` | $$1<\mathrm{mod}<2^{32}$$ の奇数法で構築 | $$O(1)$$ |
| `mod` / `inverse` / `r2` / `one` | 法／法の $$2^{32}$$ に対する逆元／$$2^{64}\bmod\mathrm{mod}$$／Montgomery 表現の 1 | $$O(1)$$ |
| `in(x)` | 任意の `uint32_t` を受け取り、`x % mod` の Montgomery 表現に変換 | $$O(1)$$ |
| `reduce(x)` | $$0\le x<2^{32}\mathrm{mod}$$ を Montgomery 縮約。`reduce(in(a))` で通常の値に戻す | $$O(1)$$ |
| `mul(a,b)` / `add(a,b)` | 法未満の Montgomery 表現で乗算／加算 | $$O(1)$$ |
| `pow(a,k)` | Montgomery 表現で非負整数乗。指数は `uint32_t` | $$O(\log k)$$ |

型 `U` は `uint32_t`、`W` は `uint64_t` です。表現は常に法未満です。演算途中の加算が 32 bit を超える法にも対応します。より大きい法には [Montgomery64](https://qandori.github.io/cp-library/components/montgomery.hpp.html) を使います。

構築・変換・加算・乗算は $$O(1)$$、累乗は $$O(\log k)$$ 時間、空間は $$O(1)$$ です。
