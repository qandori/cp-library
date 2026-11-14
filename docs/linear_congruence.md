---
title: 一次合同式
documentation_of: //math/linear_congruence.hpp
redirect_from:
- /math/number_theory/linear_congruence.hpp.html
- /lightweight/math/number_theory/linear_congruence.hpp.html
- /math/util/linear_congruence.hpp.html
- /lightweight/math/util/linear_congruence.hpp.html
library_category: math/number_theory
---

$$ax\equiv b\pmod m$$ を満たす整数 $$x$$ を求めます。法は素数でなくてもよく、$$a,b$$ は負でも構いません。前計算は不要です。

```cpp
#include "lib/math/linear_congruence.hpp"
#include <cassert>

// 6x ≡ 4 (mod 10): x = 4 + 5k
const auto solution = linear_congruence(6, 4, 10);
assert(solution);
assert(solution->residue == 4);
assert(solution->period == 5);
assert(solution->count == 2);
assert(!linear_congruence(6, 3, 10));
```

| 操作・型 | 引数・返り値 | 計算量 |
| --- | --- | --- |
| `linear_congruence(a, b, m)` | 3 引数は `std::int64_t`。解があれば `std::optional<LinearCongruenceSolution>`、なければ `std::nullopt` を返す | $$O(\log m)$$ |
| `solution.residue` | 最小非負解 `std::int64_t` | $$O(1)$$ |
| `solution.period` | 全解の周期 `std::int64_t` | $$O(1)$$ |
| `solution.count` | `0 <= x < m` の解の個数 `std::int64_t` | $$O(1)$$ |

$$1\le m\le\mathrm{INT64\_MAX}$$ とし、$$a,b$$ は符号付き 64 bit 整数の全範囲で指定できます。`m <= 0` は `std::invalid_argument` を送出します。全ての入力・返り値は 64 bit で扱い、途中の積だけ 128 bit を使ってオーバーフローを防ぎます。`constexpr` に対応します。

$$g=\gcd(a,m)$$ とすると、$$g$$ が $$b$$ を割り切る場合に限り解があります。そのとき、返り値は次の条件を満たします。

$$0\le\mathrm{residue}<\mathrm{period},\qquad\mathrm{period}=\displaystyle\frac{m}{g},\qquad\mathrm{count}=g.$$

全ての整数解は

$$x=\mathrm{residue}+k\,\mathrm{period}\quad(k\in\mathbb{Z})$$

です。特に $$0\le x<m$$ の解は $$k=0,\ldots,\mathrm{count}-1$$ で重複なく表せます。先頭の例では 4 と 9 です。解の配列は確保しないので、個数が多くても同じ API で扱えます。

- `a = 0`：`b` が `m` の倍数なら全ての整数が解で、`{0, 1, m}` を返します。それ以外は解なしです。
- `m = 1`：常に全ての整数が解で、`{0, 1, 1}` を返します。
- `b = 1`：解がある場合、`residue` は `a` の `m` を法とする逆元です。合成数の法にも使えます。

拡張 Euclid の互除法で解きます。時間は $$O(\log m)$$、追加空間は $$O(1)$$ です。`m = 1` は $$O(1)$$ 時間です。
