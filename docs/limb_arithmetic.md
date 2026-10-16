---
title: "Limb Arithmetic"
documentation_of: //components/limb_arithmetic.hpp
---

```cpp
#include "lib/components/limb_arithmetic.hpp"
auto carry = limb_arithmetic::add<1000000000000000000ULL>(a, b, n);
auto borrow = limb_arithmetic::subtract<1000000000000000000ULL>(a, b, n);
```

`uint64_t` の配列を下位桁から並べ、`a` に加減算の結果を上書きします。各要素は基数未満、基数は $$2\le B\le2^{62}$$ としてください。第 4 引数で繰り上がり・借り $$0/1$$ を指定でき、戻り値も $$0/1$$ です。`a == b` に対応します。それ以外の範囲の重なりは避けてください。AVX2 が使える x86 環境では自動で切り替えます。

時間は $$O(n)$$、追加空間は $$O(1)$$ です。
