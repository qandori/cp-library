---
title: 木の直径
documentation_of: //tree/tree_diameter.hpp
library_category: tree
---

重み付き無向木で、重みの合計が最大になる単純パスと、その長さを求めます。負の重みも扱えます。

```cpp
#include "lib/tree/tree_diameter.hpp"

TreeDiameter<> tree(4);
tree.add_edge(0, 1, 3);
tree.add_edge(1, 2, 4);
tree.add_edge(1, 3, 2);
auto result = tree.solve();
assert(result.distance == 7);
// result.path は 0, 1, 2 またはその逆順
```

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `TreeDiameter<T>(n = 0)` | 頂点数を指定。`T` の既定値は `int64_t` | $$O(n)$$ |
| `add_edge(u,v,weight = 1)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `solve()` | `Result{distance,path}` を返す | $$O(n)$$ |
| `result.distance` | 直径の重みの合計（`T`） | $$O(1)$$ |
| `result.path` | 両端を含むパスの頂点列（`vector<int>`） | $$O(1)$$ |
| `size()` / `edge_count()` | 頂点数／辺数 | $$O(1)$$ |
| `edges()` | 追加順の `Edge{from,to,weight}` の配列への `const` 参照 | $$O(1)$$ |
| `reserve(m)` | 辺 `m` 本分の領域を予約 | $$O(m)$$ |
| `clear()` | 全辺を削除。頂点数は維持 | $$O(n)$$ |

頂点番号は `[0,n)`、頂点数は `INT_MAX/2` 以下です。`solve()` の前に閉路のない連結な木を作り、非空なら辺数を `n-1` にしてください。`reserve(m)` は `0 <= m <= n-1` の範囲で指定します。空の木では `m=0` のみです。

単頂点のパスを長さ `0` として含めます。単点の木や全ての辺が負の木では、長さ `0` の単頂点パスを返します。空の木では長さ `0` と空列を返します。最適なパスが複数ある場合の選択・向きは保証しません。

`T` には重みと全てのパスの長さを表現できる型を指定します。128 bit 整数が必要なら `TreeDiameter<__int128_t>` を使ってください。`solve()` は入力を変更せず、繰り返し呼べます。コピーと返された結果は独立します。

頂点数を $$n$$ として、構築・直径の計算は $$O(n)$$ 時間、入力・結果・作業領域を含めて $$O(n)$$ 空間です。
