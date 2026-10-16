---
title: "多倍長整数"
documentation_of: //math/bigint.hpp
---

```cpp
#include "lib/math/bigint.hpp"
#include <iostream>

BigInt a("123456789012345678901234567890"), b = 12345;
auto [q, r] = divmod(a, b);
std::cout << a * b << '\n';
std::string s = a.str();
auto p = pow(b, 100);
```

`BigInt` は符号付き整数です。整数型・十進文字列から構築でき、四則演算・剰余・比較・複合代入・`++`・`--`・`abs`・ストリーム入出力に対応します。整数型への変換は `static_cast<T>(a)` です。

`divmod(a, b)` は商・余りを同時に返します。除算は 0 方向への切り捨て、余りの符号は被除数に従います。`pow(a, k)` の `k` は `uint64_t` です。

桁数を $$n$$、乗算の時間を $$M(n)$$ として、構築・十進変換・加減算は $$O(n)$$、比較は最悪 $$O(n)$$、乗算は筆算・Karatsuba・NTT を切り替えます。NTT の変換長が $$2^{23}$$ 以下なら $$M(n)=O(n\log n)$$、それを超える場合は分割して計算します。被除数・除数の桁数を $$n\ge m$$ として、除算・剰余は筆算で $$O(nm)$$、逆数法で $$O(\frac{n}{m}M(m))$$ です。`pow` は結果の桁数を $$n$$ として $$O(M(n)\log k)$$、空間は $$O(n)$$ です。
