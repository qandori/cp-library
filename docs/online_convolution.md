---
title: "Online Convolution"
documentation_of: //convolution/online_convolution.hpp
---

```cpp
#include "lib/convolution/online_convolution.hpp"

OnlineConvolution<> product(3);
mint c0 = product.push(2, 4);
mint c1 = product.push(3, 5);
mint known = product.pending();
mint c2 = product.push(7, 11);
```

`a`・`b` の係数を定数項から同時に 1 項ずつ追加し、積の次の係数を返します。上の例では `c0=8`、`c1=22`、`known=15`、`c2=65` です。将来の入力は不要で、確定した結果から次の入力を決められます。

| 関数・型 | 内容・返り値 |
| --- | --- |
| `OnlineConvolution<T = mint>` | 法 998244353 のオンライン畳み込み。係数型 `T` は `mod()`・`val()`・`raw()` を持つ modint |
| `OnlineConvolution(max_size = 1 << 23)` | 追加できる項数の上限を指定して構築。最初から上限分を確保することはない |
| `push(x,y)` | 次の係数対 `a[n]=x`・`b[n]=y` を追加し、積の対応する係数を `T` で返す。追加前の項数を `n` とすると $$\sum_{i=0}^n a_i b_{n-i}$$ |
| `pending()` | 次の係数のうち、追加済みの入力だけで決まる部分を `T` で返す |
| `operator[](i)` | 確定済みの積の第 `i` 係数を `T` で返す。`0 <= i < size()` |
| `size()` | 追加済みの係数対の数 |
| `max_size()` | 構築時に指定した項数の上限 |
| `empty()` | 係数対をまだ追加していなければ `true` |
| `reserve(n)` | 入力列と結果の領域を事前確保。`0 <= n <= max_size()`。上限は変わらない |
| `clear()` | 入力・結果・変換済みの先頭区間を消去して再利用。上限は変わらない |

`0 <= max_size <= 2^23` とし、`push()`・`pending()` は `size() < max_size()` のときに呼びます。必要な項数が分かる場合は、構築時に指定すると最後の不要な先読み計算を省けます。長さの異なる有限列を最後まで畳み込む場合は、上限を `a.size()+b.size()-1` とし、それぞれの列の末尾に 0 を追加します。

追加済みの項数を $$n$$ とすると、`pending()` は $$\sum_{i=1}^{n-1} a_i b_{n-i}$$ です。$$n=0,1$$ では 0 を返します。次の `push(x,y)` の返り値は、$$n>0$$ なら $$\mathrm{pending}()+xb_0+a_0y$$、$$n=0$$ なら $$xy$$ になります。`pending()` は状態を変更しません。

係数の法は 998244353 に固定しています。[mint](https://qandori.github.io/cp-library/math/mint.hpp.html) を既定とし、同じ法の別の modint でも使えます。内部では既存の NTT と Montgomery 表現を使い、対応する x86 CPU では AVX2 を利用します。Mac・Clang でも利用できます。

項数を $$n$$ として、`push()` を合計 $$n$$ 回行う時間は $$O(n\log^2 n)$$、1 回の最悪時間は $$O(n\log n)$$、使用空間は $$O(n)$$ です。構築・`size()`・`max_size()`・`empty()`・`operator[]`・`pending()` は $$O(1)$$ です。`reserve(r)` は $$O(n)$$ 時間・$$O(r)$$ 空間、`clear()` は $$O(n)$$ 時間以内です。事前確保した領域と共有 NTT キャッシュは別途保持されます。
