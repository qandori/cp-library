---
title: "NTT（法指定）"
documentation_of: //convolution/ntt_mod.hpp
---

```cpp
#include "lib/convolution/ntt_mod.hpp"

std::vector<std::uint32_t> a = {1, 2, 3, 4};
ntt_mod<998244353, 3>(a);
ntt_mod<998244353, 3>(a, true);
```

法 `mod` は $$2 < \mathrm{mod} < 2^{31}$$ を満たす素数、`root` は原始根です。要素は法未満、長さ $$n$$ は $$\mathrm{mod}-1$$ を割り切る 2 の冪とします。逆変換には長さの逆数の乗算を含みます。

時間は $$O(n\log n+\log n\log\mathrm{mod})$$、追加空間は $$O(1)$$ です。
