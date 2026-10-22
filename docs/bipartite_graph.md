---
title: 二部グラフ（自動色分け）
documentation_of: //graph/matching/bipartite_graph.hpp
---

無向グラフの隣接リスト `vector<vector<int>>` を渡し、二部判定・色分け・最大マッチング・最大独立集合・最小頂点被覆を求めます。左右の頂点番号を付け直す必要はなく、結果はすべて元の頂点番号です。

```cpp
#include "lib/graph/matching/bipartite_graph.hpp"

int main() {
	std::vector<std::vector<int>> g(5);
	for (auto [u, v] : std::vector<std::pair<int, int>>{ {0, 1}, {1, 2}, {2, 3}, {3, 0} }) {
		g[u].push_back(v);
		g[v].push_back(u);
	}
	BipartiteGraph graph(g);
	if (!graph.is_bipartite()) return 0;
	auto independent = graph.maximum_independent_set();
	auto cover = graph.minimum_vertex_cover();
	auto matching = graph.maximum_matching();
	assert(independent.size() == 3);
	assert(cover.size() == 2 && matching.size() == 2);
}
```

各無向辺は両端の隣接リストへ追加します。頂点番号は $$[0,N)$$、頂点数は `INT_MAX` 未満、辺数は `INT_MAX` 以下です。非連結グラフ・孤立頂点・多重辺・空グラフに対応します。自己ループや奇数長の閉路があれば `is_bipartite()` は偽になります。

二部グラフでない場合、`size()`・`empty()`・`is_bipartite()`・再構築の `build(g)` を使えます。色分けや集合を取得する操作は、二部グラフであることを確認してから呼びます。入力を変更せず、構築後は入力を参照しません。

$$N$$ を頂点数、$$M$$ を無向辺数、$$K$$ を色分け後の小さい側の頂点数とします。集合の取得時に必要なら最大マッチングを計算し、その結果を再利用します。下表の取得操作の計算量は、最大マッチングを計算済みの場合です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BipartiteGraph(g)` | 無向隣接リストから構築し、二部判定・色分けを行う | $$O(N+M)$$ |
| `build(g)` | 指定したグラフで構築し直し、二部グラフなら真を返す | $$O(N+M)$$ |
| `size()` / `empty()` | 頂点数／頂点数が 0 か | $$O(1)$$ |
| `is_bipartite()` | 二部グラフか | $$O(1)$$ |
| `coloring()` | 各頂点の色 0 または 1 の `const vector<int>&` | $$O(1)$$ |
| `matching_size()` | 最大マッチングの辺数 | $$O(1)$$ |
| `maximum_matching()` | 最大マッチングの頂点対 `(色0,色1)` の列 | $$O(N)$$ |
| `mates()` | 各頂点の対応相手。未対応は -1 | $$O(N)$$ |
| `minimum_vertex_cover()` | 全辺を少なくとも片端で覆う最小の頂点集合 | $$O(N+M)$$ |
| `maximum_independent_set()` | 選んだ頂点間に辺がない最大の頂点集合 | $$O(N+M)$$ |
| `minimum_edge_cover()` | 全頂点を覆う最小の頂点対の列。孤立頂点があれば `nullopt` | $$O(N+M)$$ |

最大独立集合の頂点数は $$N-\lvert\mathrm{matching}\rvert$$、最小頂点被覆の頂点数は最大マッチングの辺数です。両者は互いの補集合になります。最小辺被覆が存在する場合、その辺数も $$N-\lvert\mathrm{matching}\rvert$$ です。空グラフの最小辺被覆は空の列です。

[ABC445 G - Knight Placement](https://atcoder.jp/contests/abc445/tasks/abc445_g) では、空きマスを頂点、同時にコマを置けない 2 マスを辺にしたグラフを渡します。`maximum_independent_set()` が返す頂点のマスへコマを置けば、最大個数の配置を復元できます。マスの座標から色を計算する必要はありません。

左右を指定して辺を追加したり、Hall 条件の反例・辺の分類を求めたりする場合は [二部マッチング](bipartite_matching.md) を使います。このライブラリも同じマッチング処理を利用します。

構築は $$O(N+M)$$、最大マッチングの計算は $$K>0$$ なら $$O((N+M)\sqrt K)$$ 時間、$$K=0$$ なら $$O(1)$$ 時間です。保存領域は $$O(N+M)$$、集合の復元は $$O(N+M)$$ 時間・追加空間です。
