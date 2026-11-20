---
title: Floor Sum
documentation_of: //math/floor_sum.hpp
library_category: math/floor_sum
redirect_from:
- /math/floor_sum/floor_sum.hpp.html
- /lightweight/math/floor_sum/floor_sum.hpp.html
---

一次式を除算し、小数部分を負の無限大の方向へ切り捨てた値の総和を求めます。例えば $$\lfloor-1/2\rfloor=-1$$ です。

$$\displaystyle \sum_{i=0}^{n-1}\left\lfloor\frac{ai+b}{m}\right\rfloor$$

```cpp
#include "lib/math/floor_sum.hpp"

auto a = floor_sum(4, 10, 6, 3); // 3
auto b = floor_sum(6, 6, -2, 9); // 1
auto c = floor_sum<__int128_t>(10000000000ULL, 1, 1, 0);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `floor_sum<T = std::int64_t>(n,m,a,b)` | 上記の総和を整数型 `T` で返す | $$O(\log m)$$ |

引数は項数 `n`、法 `m`、係数 `a`、定数項 `b` の順です。`n,m` は `uint64_t`、$$m>0$$ です。`a,b` は 64 bit 以下の整数型で、符号付き・符号なしを混在させられます。負の値も床関数として扱い、`n=0` では 0 を返します。

答えが `T` に収まるようにしてください。既定の返り値は `int64_t` で、それを超える場合は `floor_sum<__int128_t>(...)` と指定します。

時間は $$O(\log m)$$、追加空間は $$O(1)$$ です。
