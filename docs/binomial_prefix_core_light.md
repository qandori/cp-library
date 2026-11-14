---
title: 軽量な二項係数の累積和の内部演算
documentation_of: //components/binomial_prefix_core_light.hpp
library_category: components/math
---

[軽量な二項係数の累積和](binomial_prefix_sum_light.md)と[差分更新版](binomial_prefix_sum_incremental_light.md)の共通部品です。通常版 FPS の依存を持たず、`light_binomial_prefix_detail::Table<Mint>` を使います。

| 操作・メンバ | 内容 |
| --- | --- |
| `Table(n=0)` / `build(n)` | 階乗・逆階乗・2 の冪を構築／再構築 |
| `binom(n,k)` | 二項係数。`k < 0` または `k > n` なら 0 |
| `center(n)` | $$\displaystyle \sum_{i=0}^{\lfloor(n+1)/2\rfloor-1}\binom ni$$ |
| `range(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$。`0 <= l <= r <= n+1` が必要 |
| `factorial` / `inverse_factorial` / `power2` | 各前計算の `vector<Mint>` |
| `half` | 奇素数法では 2 の逆元 |

法は素数、構築する上限は非負で `INT_MAX` 未満かつ法未満にします。各操作の `n` は構築済みの範囲内で指定します。`Mint` は公開ライブラリと同じ条件です。

前計算は $$O(n)$$ 時間・空間と逆元計算 1 回です。`binom`・`center` は $$O(1)$$ 時間・追加空間、`range` は $$O(r-l)$$ 時間・$$O(1)$$ 追加空間です。
