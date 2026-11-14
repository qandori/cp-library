---
title: "多点評価"
documentation_of: //fps/multipoint_eval.hpp
library_category: fps
---

```cpp
#include "lib/fps/multipoint_eval.hpp"

fps f{1, 2, 3};
std::vector<mint> xs{0, 1, 2};
auto values = multipoint_eval(f, xs); // {1,6,17}
```

法は 998244353 です。評価点の重複を許し、入力は変更しません。

`f={1,2,3}` は $$1+2x+3x^2$$ です。`values[i]` は `f(xs[i])` で、点をソートする必要はありません。`values.size() == xs.size()` となり、同じ点を複数渡した場合もそれぞれの位置に答えが入ります。`mint` の値を通常の整数にするには `values[i].val()` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `multipoint_eval(f,xs)` | `xs` の順に多項式の値を返す。返り値は `vector<T>`。空の点列は空配列 | $$O((n+m)\log^2(n+m))$$ |

係数数を $$n$$、点数を $$m$$ として、時間 $$O((n+m)\log^2(n+m))$$、追加空間 $$O((n+m)\log(n+m))$$ です。
