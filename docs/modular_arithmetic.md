---
title: 剰余累乗・逆元
documentation_of: //math/modular_arithmetic.hpp
library_category: math/number_theory
redirect_from:
- /math/util/modular_arithmetic.hpp.html
- /lightweight/math/util/modular_arithmetic.hpp.html
---

```cpp
#include "lib/math/modular_arithmetic.hpp"
#include <cassert>

int main() {
	assert(pow_mod(-2, 10, 1000) == 24);
	assert(inv_mod(3, 10) == 7);
}
```

| 関数 | 引数・返り値 | 計算量 |
| --- | --- | --- |
| `pow_mod(x, n, m)` | `x, n` は `std::int64_t`、`m` は `std::uint32_t`。`n >= 0, m >= 1` で $$x^n\bmod m$$ を `std::uint32_t` で返す | $$O(\log n)$$ |
| `inv_mod(x, m)` | 両引数は `std::int64_t`、`m >= 1`。$$xy\equiv1\pmod m$$ を満たす `0 <= y < m` を `std::int64_t` で返す | $$O(\log m)$$ |

どちらも素数・合成数の法と負の `x` に対応します。法 1 の返り値は 0 です。0 の 0 乗は 1 を法で割った余りです。不正な法・負の指数は `std::invalid_argument`、逆元が存在しない場合は `std::domain_error` です。`inv_mod` は定数式でも利用できます。

`pow_mod` は $$O(\log n)$$ 時間、`inv_mod` は $$O(\log m)$$ 時間、追加空間はいずれも $$O(1)$$ です。指数 0 は定数時間です。
