---
title: "NTT（998244353）"
documentation_of: //convolution/ntt.hpp
---

```cpp
#include "lib/convolution/ntt.hpp"

std::vector<fps_mint> a = {1, 2, 3, 4};
ntt(a);
ntt(a, true);
```

法 998244353・原始根 3 の NTT です。長さは $$2^{23}$$ 以下の 2 の冪とします。逆変換には長さの逆数の乗算を含みます。

長さを $$n$$ として時間は $$O(n\log n)$$、キャッシュの空間は $$O(n)$$ です。
