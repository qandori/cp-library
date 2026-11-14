---
title: "Divisor Zeta / Möbius Transform"
documentation_of: //components/divisor_zeta.hpp
redirect_from: /convolution/divisor_zeta.hpp.html
library_category: components
---

```cpp
#include "lib/components/divisor_zeta.hpp"

std::vector<long long> a{0, 1, 2, 3, 4};
divisor_zeta(a); // {0,1,3,4,7}
divisor_zeta<true>(a); // {0,1,2,3,4}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `divisor_zeta<inverse = false,multiples = false>(a)` | 約数和で上書き。`multiples = true` は倍数和、`inverse = true` は逆変換 | $$O(n\log\log n)$$ |
| `divisor_zeta<inverse,multiples>(a,primes)` | 事前計算した `prime_sieve(a.size())` を再利用 | $$O(n\log\log n)$$ |

`divisor_zeta<inverse = false, multiples = false>(a)` は約数和、`multiples = true` なら倍数和を計算します。`inverse = true` は逆変換です。添字 0 は使いません。

同じ長さで繰り返す場合は `prime_sieve(a.size())` の結果を第 2 引数に渡せます。

既定では、変換後の `a[i]` が元の配列の `a[d]`（`d` は `i` の約数）の和です。例の添字 4 は `1+2+4=7` になります。`multiples=true` なら `i` の倍数の添字を合計します。配列をその場で書き換え、戻り値は `void` です。逆変換でも同じ `multiples` を指定します。

長さを $$n$$ として時間は $$O(n\log\log n)$$、追加空間は素数列を渡す場合 $$O(1)$$、省略する場合 $$O(n)$$ です。要素の演算は $$O(1)$$ とします。
