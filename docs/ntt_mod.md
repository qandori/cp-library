---
title: "NTT（法指定）"
documentation_of: //components/ntt_mod.hpp
redirect_from: /convolution/ntt_mod.hpp.html
library_category: components
---

NTT は、素数を法として係数列を別の値の列へ変換する方法です。長さを $$n$$、$$\omega=\mathrm{root}^{(\mathrm{mod}-1)/n}$$ とすると、順変換は $$\displaystyle A_k=\sum_{j=0}^{n-1}a_j\omega^{jk}$$ です。逆変換で元の係数列に戻せます。

```cpp
#include "lib/components/ntt_mod.hpp"

std::vector<std::uint32_t> a = {1, 2, 3, 4};
ntt_mod<998244353, 3>(a);
ntt_mod<998244353, 3>(a, true);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ntt_mod<mod,root>(a,inverse = false)` | `vector<uint32_t>` を指定法で NTT。`true` は元の配列に戻す逆変換 | $$O(n\log n+\log\mathrm{mod})$$ |

法 `mod` は $$2 < \mathrm{mod} < 2^{31}$$ を満たす素数、`root` は原始根、つまり0 乗から `mod-2` 乗までが、法で割った余り 1 から `mod-1` を全て一度ずつ取る値です。要素は法未満、長さ $$n$$ は $$\mathrm{mod}-1$$ を割り切る 2 の冪とし、空配列は渡せません。入力を変換結果で上書きし、返り値は `void` です。逆変換には長さの逆数の乗算を含みます。

時間は $$O(n\log n+\log\mathrm{mod})$$、追加空間は $$O(n)$$ です。
