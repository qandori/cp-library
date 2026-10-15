---
title: "素因数分解・素数判定"
documentation_of: //math/factorize.hpp
---

```cpp
#include "lib/math/factorize.hpp"

auto factors = factorize(360);
auto counts = factorize_count(360);
bool prime = is_prime(998244353);
```

引数は `uint64_t` です。`factorize(n)` は素因数の昇順列、`factorize_count(n)` は `{素因数, 指数}` の昇順列を返します。素因数分解は `n >= 1` とします。

64 bit 整数演算を `O(1)` として、`is_prime(n)` は `O(log n)` 時間、`O(1)` 空間です。`factorize(n)`・`factorize_count(n)` は Pollard Rho の期待計算量で `O(n^(1/4) log n)` 時間、`O(log n)` 空間です。
