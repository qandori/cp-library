---
title: "素数 mod の平方根"
documentation_of: //math/mod_sqrt.hpp
---

```cpp
#include "lib/math/mod_sqrt.hpp"
#include <iostream>

auto root = mod_sqrt(10, 13);
if (root) std::cout << *root << '\n';
```

| 関数 | 内容・返り値 |
| --- | --- |
| `mod_sqrt(a,prime)` | $$x^2\equiv a\pmod{\mathrm{prime}}$$ を満たす小さい方の非負整数を `optional<uint64_t>` で返す。解がなければ `nullopt` |

`a` と `prime` は `uint64_t` です。法は 2 または任意の奇素数で、$$2^{64}$$ 未満に対応します。素数判定は行いません。`a` は法以上でも構いません。負数を渡す場合は、呼び出し前に非負の剰余へ正規化してください。

0 の平方根は 0 です。0 以外の平方根は返された値と `prime - *root` の 2 個です。法が 2 の場合は平方根が 1 個だけです。

32 bit の法には 32 bit Montgomery 演算、それ以外は 64 bit Montgomery 演算を使います。$$p\equiv3\pmod4$$ は累乗 1 回と確認です。それ以外は Tonelli–Shanks 法を使い、大きい 2 群では離散対数を分割して補正します。非平方剰余の探索に固定 seed の乱数を使い、乱数の共有状態や永続的な前計算表は持ちません。

奇素数の法を $$p$$、$$s=\nu_2(p-1)$$ として、期待 $$O(\log p+s\log s)$$ 時間、追加空間は $$O(\log p)$$ です。法が 2 の場合は $$O(1)$$ 時間・空間です。
