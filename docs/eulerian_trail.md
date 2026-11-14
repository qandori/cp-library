---
title: 有向・無向 Euler 路
documentation_of: //graph/eulerian_trail.hpp
library_category: graph
---

全ての辺をちょうど 1 回ずつ通る Euler 路を求めます。始点と終点が同じ Euler 閉路も含みます。自己辺・多重辺に対応し、元の辺 ID と頂点列を返します。

```cpp
#include "lib/graph/eulerian_trail.hpp"
#include <cassert>

DirectedEulerianTrail graph(3);
graph.add_edge(0, 1); // 辺 ID 0
graph.add_edge(1, 2); // 辺 ID 1
auto result = graph.solve();
assert(result);
assert((result->vertices == std::vector<int>{0, 1, 2}));
assert((result->edges == std::vector<int>{0, 1}));

UndirectedEulerianTrail undirected(2);
undirected.add_edge(0, 1);
auto reversed = undirected.solve(1); // 頂点列は {1,0}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DirectedEulerianTrail(n = 0)` | 有向グラフ。`EulerianTrail<true>` の別名 | $$O(1)$$ |
| `UndirectedEulerianTrail(n = 0)` | 無向グラフ。`EulerianTrail<false>` の別名 | $$O(1)$$ |
| `EulerianTrail<Directed>(n,edges)` | `span<const pair<int,int>>` の辺列から構築。`vector<pair<int,int>>` も渡せる | $$O(m)$$ |
| `size()` / `edge_count()` | 頂点数／辺数 | $$O(1)$$ |
| `reserve(m)` | 辺の格納領域を予約 | 再確保時は既存の辺数に比例 |
| `add_edge(u,v)` | 辺を追加し、追加順の 0 始まりの辺 ID を返す。有向版では `u` から `v` | 償却 $$O(1)$$ |
| `get_edge(id)` | 辺の両端を `pair<int,int>` で返す | $$O(1)$$ |
| `edges()` | 入力した辺列の const 参照 | $$O(1)$$ |
| `clear()` | 頂点数・確保済み容量を保ち、全ての辺を取り除く | $$O(m)$$ 以下 |
| `solve(start = -1)` | Euler 路を `optional<Result>` で返す。存在しなければ `nullopt` | $$O(n+m)$$ |

頂点番号は `[0,n)` です。無向辺は `add_edge` を 1 回だけ呼んで登録します。両方向を別々に登録すると多重辺になります。

`Result::vertices` は通る順の頂点列、`Result::edges` は通る順の辺 ID 列で、どちらも `vector<int>` です。辺が `m` 本で頂点が存在する場合、長さはそれぞれ `m+1`・`m` です。`edges[i]` は `vertices[i]` から `vertices[i+1]` への辺で、無向版では登録時と逆向きに通る場合もあります。辺 ID は自己辺・多重辺も区別します。

`start = -1` では始点を自動選択します。始点と終点が異なる有向版では出次数が入次数より 1 大きい頂点、始点と終点が異なる無向版では奇数次数の小さい方の頂点を選びます。閉路の場合は辺に接続する最小番号の頂点を選びます。始点を指定した場合は、その頂点から全辺を通る路がなければ `nullopt` を返します。辞書順最小の路を返す保証はありません。

次数条件だけでなく、全ての辺を使えたかも確認します。辺が別々の連結成分に分かれている場合は `nullopt` です。辺を持たない孤立点は無視しますが、辺があるのに孤立点を始点として指定した場合は `nullopt` です。

辺が 0 本なら、頂点が存在する場合は指定した始点だけを含む頂点列を返します。自動選択なら頂点 0 です。頂点数も 0 の場合は、頂点列・辺列が共に空の結果を返します。この場合は既定の `solve()` を使ってください。

`solve()` はグラフを変更せず、繰り返し呼べます。辺の追加後も再実行できます。求解結果は独立した配列です。`edges()` の参照は辺の追加・削除・代入・破棄に伴う通常の `vector` の規則に従います。

反復版 Hierholzer 法と CSR を使います。探索中の頂点・辺スタックと逆順に確定する結果に、同じ配列の両端を使います。有向版は各辺を 1 回だけ格納し、無向版は両方向の隣接辺と辺 ID ごとの使用済みフラグを持ちます。再帰と辺 ID の復元用の辞書は使いません。

頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下とします。構築は $$O(1)$$、辺列からの構築は $$O(m)$$、辺追加は償却 $$O(1)$$、`solve()` は $$O(n+m)$$ 時間・追加空間です。辺が 0 本の `solve()` は $$O(1)$$ 時間・追加空間です。
