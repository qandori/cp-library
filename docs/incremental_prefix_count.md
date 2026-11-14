---
title: 一点加算・接頭辞個数（補助部品）
documentation_of: //components/incremental_prefix_count.hpp
library_category: components/data_structure
---

`IncrementalPrefixCount(n)` は長さ `n` の非負整数列を `0` で初期化します。`increment(p)` で一点に `1` を加え、`prefix(r)` で `[0,r)` の和を返します。`size()` は列長です。加算回数の合計は `INT_MAX` 以下とします。

長さ・添字は `int` で、`n >= 0`、`0 <= p < n`、`0 <= r <= n` とします。`increment` の返り値は `void`、`prefix`・`size` は `int` です。空の列も構築でき、`prefix(0)` は 0 を返します。

構築・空間は $$O(n)$$、加算・接頭辞和は $$O(\log n)$$、`size` は $$O(1)$$ 時間です。加算・取得の追加空間は $$O(1)$$ です。
