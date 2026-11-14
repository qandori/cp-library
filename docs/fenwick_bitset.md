---
title: 更新可能ビット列の区間個数（補助部品）
documentation_of: //components/fenwick_bitset.hpp
library_category: components/data_structure
---

`FenwickBitset(n)` は長さ `n` の全要素が `false` のビット列を作ります。`set(p,value)` で一点を代入し、`contains(p)` で値、`count(l,r)` で半開区間 `[l,r)` の `true` の個数を返します。`size()` は列長です。

長さ・添字は `int` で、`n >= 0`、`0 <= p < n`、`0 <= l <= r <= n` とします。`set` の返り値は `void`、`contains` は `bool`、`count`・`size` は `int` です。空の列も構築でき、空区間の `count` は 0 を返します。

構築・空間は $$O(n)$$、一点代入・区間個数は $$O(\log n)$$、値の取得・`size` は $$O(1)$$ です。取得・更新の追加空間は $$O(1)$$ です。
