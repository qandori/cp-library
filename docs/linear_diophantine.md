---
title: 区間制約付き一次不定方程式
documentation_of: //math/linear_diophantine.hpp
library_category: math/number_theory
redirect_from:
- /math/util/linear_diophantine.hpp.html
- /lightweight/math/util/linear_diophantine.hpp.html
---

$$ax+by=c,\qquad x_l\le x<x_r,\qquad y_l\le y<y_r$$

を満たす整数の組 $$(x,y)$$ を求めます。係数と区間端点は負でも構いません。解を列挙する配列を持たずに、個数と辞書順で指定した解を取得できます。

```cpp
#include "lib/math/linear_diophantine.hpp"
#include <cassert>
#include <cstdint>
#include <utility>

const auto a = linear_diophantine(2, 3, 12, 0, 7, 0, 5);
assert(a && a->count() == 3);
assert((a->first() == std::pair<std::int64_t, std::int64_t>(0, 4)));
assert((a->kth(1) == std::pair<std::int64_t, std::int64_t>(3, 2)));
assert((a->kth(2) == std::pair<std::int64_t, std::int64_t>(6, 0)));
assert(!linear_diophantine(2, 4, 1, -10, 10, -10, 10));

const auto rectangle = linear_diophantine(0, 0, 0, 2, 5, 7, 9);
assert(rectangle && rectangle->count() == 6);
assert((rectangle->kth(3) == std::pair<std::int64_t, std::int64_t>(3, 8)));
```

以下では係数 `a,b` の絶対値の最大を $$A$$ とします。

| 操作・型 | 引数・返り値 | 計算量 |
| --- | --- | --- |
| `linear_diophantine(a, b, c, xl, xr, yl, yr)` | 7 引数は全て `std::int64_t`。`std::optional<LinearDiophantineSolution>` を返す。区間は `[xl, xr)` と `[yl, yr)` | $$O(\log A)$$ |
| `solution.count()` | 解の個数を `__uint128_t` で返す | $$O(1)$$ |
| `solution.first()` | 辞書順最小の解を `std::pair<std::int64_t, std::int64_t>` で返す | $$O(1)$$ |
| `solution.kth(k)` | `__uint128_t` 型の 0 始まりの順位 `k` にある解を `std::pair<std::int64_t, std::int64_t>` で返す。`x`、次に `y` の昇順 | $$O(1)$$ |

全引数は符号付き 64 bit 整数の全範囲に対応します。区間が空、方程式に整数解がない、または解が区間内にない場合は `std::nullopt` を返します。`xl > xr` または `yl > yr` は `std::invalid_argument`、`k >= count()` は `std::out_of_range` を送出します。負の順位は指定しないでください。既定引数はなく、全操作が `constexpr` に対応します。

`a = b = 0` のときも扱えます。`c = 0` なら区間内の全ての組が解で、それ以外は解なしです。解の個数は 64 bit を超え得るため `__uint128_t` で返します。この型は有効な全入力の個数を表現できます。座標の返り値は 64 bit、途中の計算は 128 bit 整数を使います。

係数 `a,b` の絶対値の最大を $$A$$ として、構築は $$O(\log A)$$ 時間・$$O(1)$$ 追加空間です。`a = b = 0` の場合は $$O(1)$$ 時間です。`count()`・`first()`・`kth(k)` は $$O(1)$$ 時間です。
