# 階乗・逆階乗と二項係数

```cpp
#define COMB_MAX 500000
#include "lib/math/comb.hpp"

init_comb();
auto value = comb(10, 3);
```

使用前に `init_comb()` を呼びます。`fact[n]` は `n!`、`finv[n]` はその逆数です。mod は 998244353 です。

`COMB_MAX` は include 前に指定します。省略時は 502022 です。`n` は前計算範囲内とし、`COMB_MAX < 998244353` が必要です。
