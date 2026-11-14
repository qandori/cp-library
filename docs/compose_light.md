---
title: "軽量 FPS の合成"
documentation_of: //fps/compose_light.hpp
library_category: fps
---

```cpp
#include "lib/fps/compose_light.hpp"

fps f{1, 2, 3}, g{0, 1, 1};
auto h = compose(f, g, 4); // {1, 2, 5, 6}
```

`compose(f,g,deg = -1)` は $$f(g(x))\bmod x^{\mathrm{deg}}$$ の係数列を返します。`deg` は返す係数数で、省略時は `f.size()`、0 なら空列です。`g` は空列または定数項が 0 である必要があります。入力は変更しません。

[軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) と同じ `FormalPowerSeries<T>` を使います。

`deg` は $$2^{21}$$ 以下とします。法や係数型の条件は軽量 FPS と同じです。

出力係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
