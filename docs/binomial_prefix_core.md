---
title: 二項係数の累積和の内部演算
documentation_of: //components/binomial_prefix_core.hpp
library_category: components/math
---

[二項係数の累積和](https://qandori.github.io/cp-library/math/binomial_prefix_sum.hpp.html) の共通部品です。`binomial_prefix_detail::Table<Mint>` を使います。

| 操作・メンバ | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Table(n=0)` | $$0\leq i\leq n$$ の階乗・逆階乗・2 の冪を前計算 | $$O(n)$$ と逆元 1 回 |
| `build(n)` | 前計算を作り直す | $$O(n)$$ と逆元 1 回 |
| `binom(n,k)` | $$\displaystyle \binom nk$$。`k < 0` または `k > n` なら 0 | $$O(1)$$ |
| `center(n)` | $$\displaystyle \sum_{i=0}^{\lfloor(n+1)/2\rfloor-1}\binom ni$$ | $$O(1)$$ |
| `range(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$。`0 <= l <= r <= n+1` が必要 | $$O(r-l)$$ |
| `factorial` | 階乗の `vector<Mint>` | $$O(1)$$ |
| `inverse_factorial` | 逆階乗の `vector<Mint>` | $$O(1)$$ |
| `power2` | 2 の冪の `vector<Mint>` | $$O(1)$$ |
| `half` | 奇素数 mod では 2 の逆元 | $$O(1)$$ |

法は素数、前計算の上限は非負で `INT_MAX` 未満かつ法未満にします。各操作の `n` は前計算の範囲内で指定します。`Mint` の条件は公開ライブラリと同じです。

`Mint = mint` では [modint の階乗・逆階乗生成](mint_factorials.md)を使い、他の型では要素を一つずつ計算します。NTT・FPS には依存しません。

前計算は $$O(n)$$ 時間・空間と逆元計算 1 回です。`binom`・`center` は $$O(1)$$、`range` は $$O(r-l)$$ 時間・$$O(1)$$ 追加空間です。
