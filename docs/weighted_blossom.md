---
title: Weighted Blossom（最大重みマッチングの内部部品）
documentation_of: //components/weighted_blossom.hpp
library_category: components/graph
---

一般グラフの最大重みマッチングを計算する内部部品です。通常は `graph/general_weighted_matching.hpp` の `GeneralWeightedMatching` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `weighted_matching_detail::Blossom<T>(n,weights,maximum)` | 重み行列をコピーして計算用の状態を作る | $$O(n^2)$$ |
| `solve(mate)` | マッチングを計算し、相手の頂点を長さ `n` の `vector<int>` に書き込む。未マッチは -1 | $$O(n^3)$$ |

`weights` は長さ `n*n` の `span<const int64_t>` で、位置 `u*n+v` が無向辺の重みです。対称行列とし、対角成分は 0、存在しない辺は 0、有効な辺は正の重みとしてください。`maximum` は最大の辺重みで、`T` は符号付き整数型です。`4*(n+1)*maximum` が `T` に収まることを要求します。公開側のライブラリが型を選択します。

`solve` の出力は 0 始まりです。出力先はあらかじめ長さ `n` とし、同じオブジェクトに対する `solve` は 1 回としてください。`n` は非負で `2*n+1` が int に収まる値とします。入力行列は構築後に破棄できます。

頂点数を $$n$$ として、構築は $$O(n^2)$$、`solve` は $$O(n^3)$$ 時間、空間は $$O(n^2)$$ です。$$n=0$$ では $$O(1)$$ です。
