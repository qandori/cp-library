# monoid floor sum

```cpp
#include "lib/math/monoid_floor_sum.hpp"

auto result = monoid_floor_sum<S, op, e>(n, m, a, b, x, y);
```

`op` は結合則を満たす演算、`e()` は単位元です。`n, m, a, b` は `uint64_t`、`m > 0` とします。

`h(i) = floor((a*i+b)/m)` として、次の列を左から `op` で集約します。累乗は同じ要素の繰り返しです。

```text
y^h(0), x, y^(h(1)-h(0)), x, y^(h(2)-h(1)), ..., x, y^(h(n)-h(n-1))
```

時間は `O(log(n+a+b+m+2))`、追加空間は `O(1)` です。`op`・`e` と `S` のコピーは `O(1)` とします。
