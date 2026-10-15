---
title: "Divisor Zeta / Möbius Transform"
documentation_of: //convolution/divisor_zeta.hpp
---

```cpp
#include "lib/convolution/divisor_zeta.hpp"

divisor_zeta(a);
divisor_zeta<true>(a);
```

`divisor_zeta<inverse = false, multiples = false>(a)` は約数和、`multiples = true` なら倍数和を計算します。`inverse = true` は逆変換です。添字 0 は使いません。

同じ長さで繰り返す場合は `prime_sieve(a.size())` の結果を第 2 引数に渡せます。

長さを $$n$$ として時間は $$O(n\log\log n)$$、追加空間は素数列を渡す場合 $$O(1)$$、省略する場合 $$O(n)$$ です。要素の演算は $$O(1)$$ とします。
