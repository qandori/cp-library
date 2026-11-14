---
title: "Taylor Shift"
documentation_of: //fps/taylor_shift.hpp
library_category: fps
---

```cpp
#include "lib/fps/taylor_shift.hpp"

fps f{1, 2, 3};
auto shifted = taylor_shift(f, 2); // {17,14,3}
f = taylor_shift(std::move(f), 3); // {34,20,3}
```

法は 998244353 です。係数列の長さは変わりません。元の `f.shift(c)` と異なり、結果を返す関数です。元の配列を書き換える場合は返り値を代入してください。不要な入力は `std::move` で渡せます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `taylor_shift(f,c)` | $$f(x+c)$$ を返す。`f` は値渡し、`c` は係数型へ変換可能な値 | $$O(n\log n)$$ |

例の最初の呼び出しは $$1+2(x+2)+3(x+2)^2=17+14x+3x^2$$ を返します。この時点で元の `f` は変わらないので、次の行は元の多項式を 3 だけ平行移動しています。配列の添字をずらす操作ではなく、多項式の変数に定数を足す操作です。

係数数を $$n$$ として、時間 $$O(n\log n)$$、追加空間 $$O(n)$$ です。
