---
title: "二項係数の累積和の差分更新部品"
documentation_of: //components/binomial_prefix_incremental_core.hpp
library_category: components/math
---

[二項係数の累積和（差分更新）](binomial_prefix_sum_incremental.md) と [軽量版](binomial_prefix_sum_incremental_light.md) の共通部品です。通常は公開ライブラリを使います。

`binomial_prefix_detail::IncrementalPrefixSum<Mint, Table, FastHalf>` は、直前の問い合わせの状態を保持します。`Table` は階乗・逆階乗・2 の冪と `binom`・`range` を提供する前計算型、`FastHalf` は `mint` の内部表現による半分の計算を使うかを指定する `bool` です。

| 操作 | 内容・返り値 |
| --- | --- |
| `IncrementalPrefixSum(max_n=0)` | `int max_n` まで前計算し、状態を `(0,0)` にする |
| `build(max_n)` | 前計算と状態を作り直す。返り値は `void` |
| `max_n()` | 前計算の上限を `int` で返す |
| `binom(n,k)` | 二項係数を `Mint` で返す。`n < 0`・`k < 0`・`k > n` なら 0 |
| `query(n,k)` | $$\displaystyle \sum_{i=0}^{k-1}\binom ni$$ を `Mint` で返す。`n` は `int`、`k` は `std::int64_t` |

`max_n` は非負で `INT_MAX` 未満かつ素数法未満、`query` の `n` は前計算の範囲内とします。`query` の `k` は `[0,n+1]` に収めて扱います。コピーは前計算を共有し、問い合わせの状態は独立します。`Mint` の条件は公開ライブラリと同じです。`FastHalf=true` は通常版が `mint` を使う場合に指定し、それ以外では `false` とします。NTT・FPS には依存しません。

前計算は $$O(N)$$ 時間・空間と逆元計算 1 回です。`max_n`・`binom` は $$O(1)$$、`query` は直前の状態との差を $$\Delta n,\Delta k$$ として $$O(\lvert\Delta n\rvert+\lvert\Delta k\rvert)$$ 時間・$$O(1)$$ 追加空間です。
