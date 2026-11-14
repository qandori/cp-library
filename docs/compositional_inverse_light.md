---
title: "軽量 FPS の合成逆元"
documentation_of: //fps/compositional_inverse_light.hpp
library_category: fps
---

```cpp
#include "lib/fps/compositional_inverse_light.hpp"

fps f{0, 1, 1};
auto g = compositional_inverse(f, 5);
// {0, 1, -1, 2, -5} (mod 998244353)
```

`compositional_inverse(f,deg = -1)` は $$f(g(x))\equiv x\pmod{x^{\mathrm{deg}}}$$ を満たす定数項 0 の係数列を返します。`deg` は返す係数数で、省略時は `f.size()`、0 なら空列、1 なら `{0}` です。入力は変更しません。

`f` の定数項は 0 とし、`deg >= 2` なら `f.size() >= 2` かつ $$f_1\ne0$$ が必要です。`deg` は $$2^{21}$$ 以下とします。法や係数型の条件は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) と同じです。

出力係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
