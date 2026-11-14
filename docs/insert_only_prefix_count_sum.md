---
title: 挿入のみの区間和・個数（補助部品）
documentation_of: //components/insert_only_prefix_count_sum.hpp
library_category: components/data_structure
---

[値の上限付き区間和](offline_range_sum.md)で使う、値の挿入と区間の和・個数の集計部品です。局所的な集計値が 64 bit に収まることを確認してから利用します。

`insert_only_prefix_detail::Packed(n,minimum,bits)` の `insert(p,delta)` は、位置 `p` に値 `minimum+delta` を挿入します。`n >= 0`・`1 <= bits < 64` とし、各位置への挿入は一度にしてください。

位置を 65536 要素ずつ区切った各区画について、挿入した `delta` の和が `2^bits` 未満、挿入個数が `2^(64-bits)` 未満となる `bits` を渡します。和を下位 `bits` bit、個数を残りの上位 bit に格納するための条件です。`prefix(r)` は `[0,r)`、`fold(l,r)` は `[l,r)` の和と個数を `CountSum{sum,count}` で返します。`sum` は `__int128_t` です。

長さ・添字は `int` で、`0 <= p < n`、`0 <= l <= r <= n` とします。`minimum` と和の計算途中は `__int128_t` に収めてください。空の列も構築でき、空区間の集計は和・個数がともに 0 です。

構築・空間は $$O(n)$$、挿入・集計は $$O(\log n)$$ 時間・$$O(1)$$ 追加空間です。
