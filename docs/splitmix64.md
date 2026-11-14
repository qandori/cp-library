---
title: "SplitMix64"
documentation_of: //components/splitmix64.hpp
redirect_from: /math/splitmix64.hpp.html
library_category: components/util
---

```cpp
#include "lib/components/splitmix64.hpp"

SplitMix64 rng(123);
auto random = rng();
auto hash = splitmix64(42);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SplitMix64(seed = 0)` | 64 bit 乱数生成器を構築 | $$O(1)$$ |
| `rng()` | 次の `uint64_t` 乱数を返す | $$O(1)$$ |
| `splitmix64(x)` | 64 bit 整数を混合して返す | $$O(1)$$ |

`SplitMix64(seed = 0)` は、初期値 `seed` から順に乱数を生成します。同じ初期値なら同じ列を返します。`splitmix64(x)` は整数の各 bit を混ぜる変換関数で、同じ `x` には常に同じ値を返します。

生成・混合は時間・空間ともに $$O(1)$$ です。
