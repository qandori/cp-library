---
title: "NTT（998244353）"
documentation_of: //components/ntt.hpp
redirect_from: /convolution/ntt.hpp.html
library_category: components
---

NTT は、係数列を素数を法とした値の列へ変換し、多項式の積などを計算しやすくする方法です。法を $$p=998244353$$、長さを $$n$$、$$\omega=3^{(p-1)/n}$$ とすると、順変換は $$\displaystyle A_k=\sum_{j=0}^{n-1}a_j\omega^{jk}$$ です。逆変換で元の係数列に戻せます。

```cpp
#include "lib/components/ntt.hpp"

std::vector<mint> a = {1, 2, 3, 4};
ntt(a);
ntt(a, true);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ntt(a,inverse = false)` | `vector<mint>` をその場で NTT。`true` は元の配列に戻す逆変換 | $$O(n\log n)$$ |

変換に使う定数は法 998244353 の原始根 3 から求めます。原始根とは、$$0$$ 乗から $$p-2$$ 乗までが、法で割った余り 1 から $$p-1$$ を全て一度ずつ取る値です。長さは $$2^{23}$$ 以下の 2 の冪で、空配列は渡せません。入力を変換結果で上書きし、返り値は `void` です。逆変換には長さの逆数の乗算を含みます。

時間は $$O(n\log n)$$ です。変換に使う定数の表は、これまでに使用した最大変換長を $$L$$ として $$O(L)$$ 空間を保持します。表の拡張時は $$O(n)$$ 時間・追加空間を使い、拡張しない場合の追加作業空間は $$O(1)$$ です。長さ 1 は $$O(1)$$ 時間です。
