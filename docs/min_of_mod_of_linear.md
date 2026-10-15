---
title: "Min of Mod of Linear"
documentation_of: //math/min_of_mod_of_linear.hpp
---

```cpp
#include "lib/math/min_of_mod_of_linear.hpp"

auto value = min_of_mod_of_linear(n, m, a, b);
```

$$\min_{0 \le x < n} ((ax+b)\bmod m)$$ を返します。引数・返り値は `std::uint64_t`、$$n,m > 0$$ です。

時間 $$O(\log m)$$、追加空間 $$O(1)$$ です。
