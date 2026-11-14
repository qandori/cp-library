---
title: 定数時間 mod 演算の内部部品
documentation_of: //components/mod_fast_core.hpp
library_category: components/math
---

[ModInv](https://qandori.github.io/cp-library/math/mod_inv.hpp.html) と [ModPow](https://qandori.github.io/cp-library/math/mod_pow.hpp.html) が使う内部部品です。通常は各ライブラリのヘッダーを include します。名前空間は `mod_fast_detail` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Reducer(m)` | $$1\le m<2^{31}$$ の法を指定し、逆数を前計算 | $$O(1)$$ |
| `Reducer::reduce(x)` | 任意の `uint64_t` の剰余を `uint32_t` で返す | $$O(1)$$ |
| `Reducer::mul(a,b)` | `uint32_t` の積の剰余 | $$O(1)$$ |
| `Reducer::pow(a,e)` | 非負の `uint64_t` 指数による繰り返し二乗法 | $$O(\log e)$$ |
| `Reducer::mod` / `Reducer::reciprocal` | 法／剰余計算用の定数。参照用 | $$O(1)$$ |
| `RationalTable(p)` | $$2\le p<2^{31}$$ の素数に対する有理数近似の表を構築 | $$O(p^{2/3})$$ |
| `RationalTable::direct()` | 小さい法に対して全要素の表を使うか | $$O(1)$$ |
| `RationalTable::limit()` | 分子の絶対値の上限 | $$O(1)$$ |
| `RationalTable::fraction(x)` | $$x\equiv a/b\pmod p$$ を満たす `{a,b}`。$$1\le x<p$$、$$a\ne0$$、$$b>0$$ | $$O(1)$$ |

`RationalTable::fraction(x)` の `{a,b}` は、`a` を `b` で割る計算を法 `p` で行うと `x` になることを表します。ここでの除算は `b` の逆元を掛ける操作です。

`Reducer` の構築・剰余計算・積は $$O(1)$$ 時間・空間、累乗は $$O(\log e)$$ 時間です。`RationalTable` の構築は $$O(p^{2/3})$$ 時間・空間、各クエリは $$O(1)$$ 時間です。
