---
title: "NTT-friendly な素数上の畳み込み"
documentation_of: //convolution/convolution_ntt.hpp
library_category: convolution
---

指定した素数の上で直接 NTT を行い、2 つの係数列の畳み込みを求めます。法はコンパイル時に決まるので、実行時に法を渡す場合は [任意 mod の畳み込み](convolution_mod.md) を使います。

```cpp
#include "lib/convolution/convolution_ntt.hpp"
#include "lib/math/modint.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	std::vector<int> a{1, 2}, b{3, 4};
	assert((convolution_ntt<924844033>(a, b) == std::vector<std::uint32_t>{3, 10, 8}));

	using M = StaticModint<998244353>;
	std::vector<M> x{1, 2}, y{3, 4};
	assert((convolution_ntt(x, y) == std::vector<M>{3, 10, 8}));
}
```

両方の列は同じ型で、係数を定数項から順に渡します。各結果は指定した法での余りです。入力は変更せず、どちらかの列が空なら空の配列を返します。両方が非空なら返り値の長さは `a.size() + b.size() - 1` です。

法を $$p$$、$$p-1$$ を割り切る最大の 2 の冪を $$D$$ とします。法は $$3 \le p < 2^{31}$$ の素数で、出力長は $$D$$ 以下にします。例えば 998244353 では $$D=2^{23}$$、924844033 では $$D=2^{21}$$ です。必要な根はコンパイル時に求めます。

入力の長さの和を $$L$$ とします。

| 操作 | 内容・返り値 | 時間 | 追加空間 |
| --- | --- | --- | --- |
| `convolution_ntt<Mod>(a,b)` | 64 bit 以下の整数列（`bool` を除く）を法 `Mod` で畳み込む。負の入力も可。返り値は `std::vector<std::uint32_t>` | $$O(L\log L)$$ | $$O(L)$$ |
| `convolution_ntt(a,b)` | 静的 modint の列をその法で畳み込む。要素型 `T` は定数式の `T::mod()`、`x.val()`、`T::raw(x)` を備え、`std::vector<T>` を返す | $$O(L\log L)$$ | $$O(L)$$ |

どちらも実行時は $$O(L\log L)$$ 時間・$$O(L)$$ 追加空間です。変換に使う表は呼び出しごとに作ります。空入力は定数時間です。
