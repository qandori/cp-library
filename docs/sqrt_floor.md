---
title: "Mod of Linear の等差数列分割"
documentation_of: //math/sqrt_floor.hpp
---

```cpp
#include "lib/math/sqrt_floor.hpp"

auto [dx, dy, segments] = sqrt_floor(n, m, a, b);
```

`0 <= i < n` の `(a*i+b) % m` を分割します。`n, a, b >= 0`、`m > 0` とし、途中の積が `long` に収まる必要があります。

`segments` の各要素 `(x, y, len)` は、`0 <= j < len` に対して添字 `x + dx*j` の値が `y + dy*j` となることを表します。

時間・出力を含む空間ともに `O(sqrt(n))` です。
