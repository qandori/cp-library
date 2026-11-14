---
title: 木ハッシュの共通部品
documentation_of: //components/tree_hash_core.hpp
library_category: components/tree
---

[根付き木のハッシュ](rooted_tree_hash.md)・[木のハッシュ](tree_hash.md) で使う共通部品です。通常は各公開ヘッダーを include してください。

`TreeHashValue` は 2 要素の `std::array<std::uint64_t,2> value` を持つ値型です。等価比較と辞書順の大小比較に対応します。`TreeHashValue::Hasher` を `std::unordered_map`・`std::unordered_set` のハッシュ関数として指定できます。

内部関数へ渡すハッシュ値は、各成分を `2^61-1` で割った余りとし、`0` 以上 `2^61-1` 未満に収めてください。根付き化は、無向木で根を指定して各頂点の親と子を決める処理です。

頂点数を $$n$$ として、根付き化・ハッシュ計算はそれぞれ $$O(n)$$ 時間・空間です。ハッシュの比較・ハッシュ関数は $$O(1)$$ 時間です。
