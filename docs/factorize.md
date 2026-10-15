# 素因数分解・素数判定

```cpp
#include "lib/math/factorize.hpp"

auto factors = factorize(360);
auto counts = factorize_count(360);
bool prime = is_prime(998244353);
```

引数は `uint64_t` です。`factorize(n)` は素因数の昇順列、`factorize_count(n)` は `{素因数, 指数}` の昇順列を返します。素因数分解は `n >= 1` とします。
