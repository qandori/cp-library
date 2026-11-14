---
title: "桁ごとの整数加減算"
documentation_of: //components/limb_arithmetic.hpp
library_category: components/math
---

整数を基数 $$B$$ ごとの桁に分けて配列に保存し、その配列どうしを加減算します。例えば基数 1000 では、配列 `{567,234,1}` は $$1\cdot1000^2+234\cdot1000+567=1234567$$ を表します。

```cpp
#include "lib/components/limb_arithmetic.hpp"

std::uint64_t a[]{99, 1}, b[]{2, 0}; // 100 進数で 199 と 2
auto carry = limb_arithmetic::add<100>(a, b, 2);      // a = {1,2}, carry = 0
auto borrow = limb_arithmetic::subtract<100>(a, b, 2); // a = {99,1}, borrow = 0
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `limb_arithmetic::add<B>(a,b,n,carry = 0)` | 下位 `n` 桁を `a += b + carry` とし、最上位の繰り上がりを返す | $$O(n)$$ |
| `limb_arithmetic::subtract<B>(a,b,n,borrow = 0)` | 下位 `n` 桁を `a -= b + borrow` とし、最上位の借りを返す | $$O(n)$$ |

`uint64_t` の配列を下位桁から並べ、`a` に加減算の結果を上書きします。各要素は基数未満、基数は $$2\le B\le2^{62}$$ としてください。第 4 引数で繰り上がり・借り $$0/1$$ を指定でき、戻り値も $$0/1$$ です。`a == b` に対応します。それ以外の範囲の重なりは避けてください。AVX2 が使える x86 環境では自動で切り替えます。

時間は $$O(n)$$、追加空間は $$O(1)$$ です。
