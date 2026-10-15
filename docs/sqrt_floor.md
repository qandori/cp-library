---
title: "Mod of Linear の等差数列分割"
documentation_of: //math/sqrt_floor.hpp
---

```cpp
#include "lib/math/sqrt_floor.hpp"

auto [dx, dy, segments] = sqrt_floor(n, m, a, b);
```

$$0 \le i < n$$ の $$(ai+b)\bmod m$$ を分割します。$$n,a,b \ge 0$$、$$m > 0$$ とし、途中の積が `long` に収まる必要があります。

`segments` の各要素 $$(x,y,\mathrm{len})$$ は、$$0 \le j < \mathrm{len}$$ に対して添字 $$x+\mathrm{dx}\,j$$ の値が $$y+\mathrm{dy}\,j$$ となることを表します。

時間・出力を含む空間ともに $$O(\sqrt{n})$$ です。
