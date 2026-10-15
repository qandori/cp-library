# modint998244353

```cpp
#include "lib/math/mint.hpp"

mint x = 3;
auto power = x.pow(5);
auto inverse = x.inv();
auto value = x.val();
```

`pow(n)` の指数は非負、`inv()` と除算の除数は非零とします。

`pow(k)` は `O(log(k+1))`、その他の演算は mod 固定のため `O(1)` 時間です。追加空間は `O(1)` です。
