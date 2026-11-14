---
title: 対称群上の畳み込み
documentation_of: //convolution/symmetric_group_convolution.hpp
library_category: convolution
---

長さ $$n$$ の全順列を添字とする配列の畳み込みを求めます。通常の加算や XOR の代わりに、順列の合成を用います。全順列の集合にこの合成を組み合わせたものを対称群と呼びます。

$$c_p=\displaystyle\sum_{q\circ r=p}a_qb_r,\qquad(q\circ r)[i]=q[r[i]].$$

順列は `0, ..., n-1` を並べたものとし、配列は **辞書順**に並べます。長さは $$n!$$ です。合成は交換可能ではなく、一般に `convolution(a, b)` と `convolution(b, a)` は異なります。

```cpp
#include "lib/convolution/symmetric_group_convolution.hpp"
#include <cassert>

std::vector<mint> a{1, 2};
std::vector<mint> b{3, 4};
auto c = symmetric_group_convolution(a, b);
assert((c == std::vector<mint>{11, 10}));
```

長さ 2 では順列は `[0, 1]` と `[1, 0]` の順です。前者はどの順列と合成しても相手を変えず、後者を自分自身と合成すると前者になります。一般の順列と添字の相互変換には [順列の辞書順順位・復元](permutation_rank.md) を使えます。

$$\lambda$$ は $$n$$ を正整数の和として表し、大きい順に並べたもの（整数分割）です。変換結果でその分割に対応する正方行列の行数を $$d_\lambda$$ とすると、$$\displaystyle\sum_\lambda d_\lambda^2=n!$$ です。

| 操作 | 引数・返り値 | 時間計算量 |
| --- | --- | --- |
| `symmetric_group_convolution(a, b)` | 同じ長さの `const std::vector<Mint>&` 2 本から `std::vector<Mint>` を返す。長さから `n` を決定 | $$O\left(n^2n!\log(n!)+\displaystyle\sum_\lambda d_\lambda^3\right)$$ |
| `symmetric_group_convolution(fft, a, b)` | 構築済みの `const SymmetricGroupFFT<Mint>&` を使い、同じ畳み込みを返す | $$O\left(n^2n!+\displaystyle\sum_\lambda d_\lambda^3\right)$$ |

入力は変更しません。型 `Mint` は両配列で同じ modint 型とし、法は **`n` より大きい素数**を使います。`mint`・`atcoder::modint998244353`・`atcoder::modint1000000007`・動的 modint に対応します。整数からの構築・四則演算・単項マイナス・`mod()` を備える型を指定してください。動的 modint の法は `fft` の構築から使用終了まで変更しません。

$$0\le n\le10$$ に対応します。空配列、長さが一致しない配列、長さが対応する階乗でない配列には `std::invalid_argument` を送出します。長さ 1 は1 要素同士の積を返します。長さ 0 の順列も長さ 1 の順列も 1 通りなので、どちらも同じ結果です。構築済み `fft` を渡す場合は、その `size()` と配列長を一致させます。

同じ次数で繰り返し計算するときは、変換の準備を共有できます。

```cpp
#include "lib/convolution/symmetric_group_convolution.hpp"
#include <cassert>

SymmetricGroupFFT<> fft(3);
std::vector<mint> a{1, 2, 3, 4, 5, 6};
std::vector<mint> unit(6);
unit[0] = 1;
assert(symmetric_group_convolution(fft, a, unit) == a);
assert(symmetric_group_convolution(fft, unit, a) == a);
```

順列を添字とする係数列を複数の行列に変換し、それぞれで行列積を求め、逆変換して畳み込みの係数に戻します。変換自体の API は [対称群 Fourier 変換](symmetric_group_fft.md) を参照してください。

参考: [Library Checker issue #1432](https://github.com/yosupo06/library-checker-problems/issues/1432)、David K. Maslen, [The efficient computation of Fourier transforms on the symmetric group](https://www.ams.org/journals/mcom/1998-67-223/S0025-5718-98-00964-8/)、同手法の [FPSAC 1997 発表版](https://fpsac-archive.github.io/FPSAC97/articles.html)。

`fft` を渡さない形式は、呼び出すたびに変換を準備します。準備を含む追加空間は $$O(nn!)$$ です。構築済み `fft` を渡す形式は、保持済みの領域に加えて $$O(n!)$$ の作業空間を使います。`fft` の構築・保持空間は [Fourier 変換](symmetric_group_fft.md) を参照してください。$$n=0,1$$ はどちらの形式も $$O(1)$$ 時間・追加空間です。modint の演算を $$O(1)$$ とします。
