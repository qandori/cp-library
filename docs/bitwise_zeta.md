---
title: "Bitwise Zeta / Möbius Transform"
documentation_of: //convolution/bitwise_zeta.hpp
---

```cpp
#include "lib/convolution/bitwise_zeta.hpp"

bitwise_zeta(a);
bitwise_zeta<true>(a);
```

`bitwise_zeta<inverse = false, superset = false>(a)` は部分集合和、`superset = true` なら上位集合和を計算します。`inverse = true` は逆変換です。長さは 2 の冪とします。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。要素の演算は $$O(1)$$ とします。
