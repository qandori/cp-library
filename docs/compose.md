---
title: "形式的冪級数の合成"
documentation_of: //fps/compose.hpp
library_category: fps
---

```cpp
#include "lib/fps/compose.hpp"

fps f{1, 2, 3}, g{0, 1, 1};
auto result = compose(f, g, 8); // {1,2,5,6,3,0,0,0}
```

`f`・`g` は変更しません。法は 998244353、`deg` は返す係数の個数です。

`f` が外側、`g` が内側の多項式です。例は $$1+2(x+x^2)+3(x+x^2)^2$$ を計算しています。`deg=8` は $$x^0$$ から $$x^7$$ までの 8 係数を返す指定で、足りない高次係数は 0 になります。`g` の定数項は必ず 0 にします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `compose(f,g,deg = -1)` | $$f(g(x))\bmod x^{\mathrm{deg}}$$。$$g_0=0$$。省略時の長さは `f.size()` | $$O(n\log^2 n)$$ |

返り値は `FormalPowerSeries<T>` です。`deg` は $$0\le\mathrm{deg}\le2^{22}$$ とし、係数型と法の条件は [FPS](fps.md) と同じです。`deg=0` は空列です。空の `f`・`g` は零多項式として扱い、`g` が空の場合も使えます。

入出力の最大係数数を $$n$$ として、時間 $$O(n\log^2 n)$$、追加空間 $$O(n\log n)$$ です。
