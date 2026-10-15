---
title: "Montgomery 64"
documentation_of: //math/montgomery.hpp
---

```cpp
#include "lib/math/montgomery.hpp"

Montgomery64 m(1000000007);
auto a = m.in(12), b = m.in(34);
auto value = m.reduce(m.mul(a, b));
```

法は 1 より大きい 64 bit 符号なし奇数です。`in(x)` で Montgomery 表現に変換し、`reduce(x)` で戻します。`add(a,b)`・`mul(a,b)`・`pow(a,k)` は Montgomery 表現で計算します。入力は法未満とします。

構築・変換・加算・乗算は $$O(1)$$、累乗は $$O(\log k)$$、空間は $$O(1)$$ です。
