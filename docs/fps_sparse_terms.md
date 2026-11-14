---
title: 疎 FPS の項収集・演算種別
documentation_of: //components/fps_sparse_terms.hpp
library_category: components/fps
---

`fps_detail::SparseTerms` は、通常版と軽量版の疎 FPS が共有する内部部品です。

`Series` は `vector<mint>`、`Terms` は次数と係数の `vector<pair<int,mint>>` です。`Kind` は逆数・対数・指数・冪の演算種別を表します。

`collect(f,size,n,limit)` は次数 `1` 以上 `min(size,n)` 未満の非零項を昇順で返します。定数項は含めず、項数が `limit` を超えた時点で走査を終了します。

走査する係数数を $$m$$、収集した項数を $$k$$ として、時間は $$O(m)$$、追加空間は $$O(k)$$ です。
