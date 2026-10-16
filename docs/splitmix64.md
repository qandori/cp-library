---
title: "SplitMix64"
documentation_of: //components/splitmix64.hpp
redirect_from: /math/splitmix64.hpp.html
---

```cpp
#include "lib/components/splitmix64.hpp"

SplitMix64 rng(123);
auto random = rng();
auto hash = splitmix64(key);
```

`SplitMix64(seed = 0)` は 64 bit 乱数生成器です。`splitmix64(x)` は加算前処理を含まない 64 bit のミキサーです。

生成・混合は時間・空間ともに $$O(1)$$ です。
