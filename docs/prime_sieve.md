---
title: "素数篩"
documentation_of: //math/prime_sieve.hpp
---

```cpp
#include "lib/math/prime_sieve.hpp"

auto primes = prime_sieve(n);
```

$$n$$ 未満の素数を昇順の `std::vector<std::size_t>` で返します。

時間は $$O(n\log\log n)$$、空間は $$O(n)$$ bit と出力分です。
