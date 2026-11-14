---
title: Dijkstra Topological
documentation_of: //components/dijkstra_topological.hpp
library_category: components/graph
---

[Dijkstra](https://qandori.github.io/cp-library/graph/dijkstra.hpp.html) のトポロジカル前処理です。通常は `graph/dijkstra.hpp` を include し、`dijkstra<true>` または `dijkstra_path<true>` で有効にします。既定の `dijkstra`・`dijkstra_path` では使いません。

まず入次数が 0 の頂点から順に距離を確定し、残った部分を Dijkstra 法で探索します。有向閉路があるグラフにも使えます。有向閉路のないグラフ（DAG）では優先度付きキューを使わず線形時間で処理します。

```cpp
#include "lib/graph/dijkstra.hpp"

using I = std::int64_t;
std::vector<std::vector<std::pair<int, I>>> g(5);
g[0] = { {1, 4}, {2, 10} };
g[1] = { {2, 3} };
g[2] = { {3, 1} };
g[3] = { {2, 0}, {4, 2} };

auto distance = dijkstra<true>(g, 0); // {0,4,7,8,10}
auto [cost, path] = dijkstra_path<true>(g, 0, 4);
// cost = 10, path = {0,1,2,3,4}
```

全頂点の距離が必要なら `dijkstra<true>`、1 つの目的地までの距離と経路が必要なら `dijkstra_path<true>` を使います。隣接リストの辺は `{行き先, 重み}`、頂点番号は 0 始まりです。グラフは変更しません。グラフ全体の入次数を数えるため、閉路の多いグラフや一部だけ探索する場合には前処理のない `dijkstra` の方が軽くなることがあります。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `dijkstra<true>(g,start)` / `dijkstra<true>(g,start,inf)` | 始点から全頂点への距離列。到達不能は `T(-1)` | DAG: $$O(n+m)$$、一般の 64 bit 以下の整数距離: $$O(n+mw)$$、その他: $$O(n+m\log m)$$ |
| `dijkstra_path<true>(g,start,target)` / `dijkstra_path<true>(g,start,target,inf)` | `{最短距離, 頂点列}`。頂点列は始点から目的地の順。到達不能は `{T(-1),{}}` | DAG: $$O(n+m)$$、一般の 64 bit 以下の整数距離: $$O(n+mw)$$、その他: $$O(n+m\log m)$$ |
| `shortest_path_detail::topological_run<Path>(g,start,target,inf,parent)` | 前処理付き探索の内部関数。到達不能の距離は `inf` のまま。`Path=true` では長さ $$n$$ の `parent` に経路を保存し、目的地以外の距離は未確定の場合がある | DAG: $$O(n+m)$$、一般の 64 bit 以下の整数距離: $$O(n+mw)$$、その他: $$O(n+m\log m)$$ |

`start == target` の経路は `{start}`、距離は 0 です。複数の最短経路がある場合はそのうち 1 つを返します。重み 0 の辺・自己辺・多重辺に対応します。無向グラフでは各辺を両方向に追加しますが、入次数 0 の頂点が少なく前処理の効果は限られます。

`inf` は扱う距離より大きい正の値を指定します。既定値は $$\min(4\cdot 10^{18},\text{距離型の最大値})$$ です。距離が `inf` 以上になる経路は扱いません。符号なし整数型の到達不能値 `T(-1)` はその型の最大値です。

入次数が 0 になった頂点は、その頂点に入る辺を持つ頂点が全て処理済みなので距離を確定できます。この除去を終えると、残った頂点から除去済みの頂点への辺はありません。確定済みの部分から残った部分へ渡した距離を初期値として、残った部分を Dijkstra 法で探索します。残る部分には閉路だけでなく、その下流の頂点も含まれます。

内部関数の `g` は `size()` と `g[v]` が使え、`g[v]` は `{行き先, 重み}` の範囲と `empty()` を持つ必要があります。全ての重みは非負とします。通常は公開関数を使います。

頂点数を $$n$$、辺数を $$m$$、距離型のビット数を $$w\le64$$ とします。DAG では $$O(n+m)$$ 時間、一般のグラフでは 64 bit 以下の整数距離に対して $$O(n+mw)$$、その他に対して $$O(n+m\log m)$$ 時間です。追加空間は $$O(n+m)$$ です。`start == target` の `dijkstra_path<true>` は $$O(1)$$ 時間・空間です。
