---
title: 順序付き整数の中点
documentation_of: //components/ordered_midpoint.hpp
library_category: components/math
---

小さい端点 `l` と大きい端点 `r` の中点を求めます。

```cpp
#include "lib/components/ordered_midpoint.hpp"

constexpr auto middle = ordered_midpoint(-8LL, 3LL); // -3
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ordered_midpoint(l,r)` | オーバーフローを避けて中点を返す。端数は `l` 側へ丸める | $$O(1)$$ |

引数は同じ整数型で、`l <= r` が必要です。`bool` を除く 64 bit 以下の符号付き・符号なし整数型に対応し、型の最小値から最大値までの範囲を渡せます。

時間・追加空間は $$O(1)$$ です。
