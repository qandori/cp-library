---
title: Range LIS の前計算
documentation_of: //components/range_lis.hpp
library_category: components/data_structure
---

[Range LIS Query](range_lis_query.md) が使う前計算部品です。通常は `RangeLISQuery` を利用してください。

`range_lis_detail::seaweed(p)` は、`0,1,...,n-1` の順列を `span<const int>` として受け取り、長さ $$n$$ の `vector<int>` を返します。長さは `INT_MAX` 以下とし、空の順列にも対応します。

返り値を `next` とすると、`next[i]` は `i` より大きい添字または `-1` です。区間 `[l,r)` の LIS の長さは、`r-l` から `l <= i < r` かつ `0 <= next[i] < r` を満たす添字の個数を引いた値になります。`-1` は該当する終点が存在しないことを表します。

前計算時間は $$O(n\log^2 n)$$、返り値を含む追加空間は $$O(n)$$ です。
