---
title: Dominator Tree
documentation_of: //graph/dominator_tree.hpp
library_category: graph/connectivity
---

有向グラフの始点からある頂点へ至る全ての経路が頂点 `u` を通るとき、`u` はその頂点を支配するといいます。到達可能な各頂点の直接支配頂点を求め、支配関係を表す木を構築します。

```cpp
#include "lib/graph/dominator_tree.hpp"
#include <cassert>
#include <vector>

int main() {
	DominatorTree graph(6);
	graph.add_edge(0, 1);
	graph.add_edge(0, 2);
	graph.add_edge(1, 3);
	graph.add_edge(2, 3);
	graph.add_edge(3, 4);
	auto result = graph.solve(0);
	assert((result.idom == std::vector<int>{0, 0, 0, 0, 3, -1}));
	assert(graph.idom(0) == result.idom);
	assert(result.dominates(0, 4) && result.dominates(3, 4));
	assert(!result.dominates(1, 4) && !result.reachable(5));
	assert(result.dominates(4, 4) && !result.dominates(5, 5));
	auto tree = result.tree();
	assert((tree[3] == std::vector<int>{4}) && tree[5].empty());
}
```

`idom[v]` は、`v` 自身を除いた支配頂点のうち、支配木上で `v` に最も近い頂点です。始点の直接支配頂点は始点自身とし、到達できない頂点は `-1` とします。
上の例では、`0` から `4` への全経路が `3` を通るので `idom[4] = 3` です。`1` と `2` のどちらかを通れば `3` に進めるため、`idom[3] = 0` です。

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DominatorTree(n = 0)` | 頂点数を指定 | $$O(1)$$ |
| `add_edge(from,to)` | 有向辺を追加し、0始まりの辺 ID を `int` で返す | 償却 $$O(1)$$ |
| `solve(root = 0)` | 始点を指定して `Result` を返す | $$O((n+m)\log n)$$ |
| `idom(root = 0)` | 直接支配頂点の配列だけを `std::vector<int>` で返す | $$O((n+m)\log n)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で取得 | $$O(1)$$ |
| `get_edge(id)` | `Edge{from,to}` を取得 | $$O(1)$$ |
| `edges()` | 入力順の辺一覧を `const std::vector<Edge>&` で取得 | $$O(1)$$ |
| `reserve(m)` | 辺 `m` 本分の領域を予約 | $$O(m)$$ |
| `clear()` | 全ての辺を削除。頂点数は維持し、辺 ID は0から再開 | $$O(m)$$ |
| `result.root` | 指定した始点。空グラフでは `-1` | $$O(1)$$ |
| `result.idom` | 直接支配頂点を持つ長さ `n` の `std::vector<int>` | $$O(1)$$ |
| `result.size()` / `result.empty()` | 元のグラフの頂点数／頂点数が0か | $$O(1)$$ |
| `result.reachable(v)` | 始点から `v` に到達できるかを `bool` で返す | $$O(1)$$ |
| `result.dominates(u,v)` | `u` が `v` を支配するかを `bool` で返す | $$O(1)$$ |
| `result.tree()` | 支配木の子リストを `std::vector<std::vector<int>>` で返す | $$O(n)$$ |

頂点番号は `[0,n)`、辺 ID は `[0,edge_count())` です。非空グラフの `root` は `[0,n)` とし、結果の問い合わせに渡す `u,v` も `[0,n)` とします。自己ループ・多重辺・閉路を扱えます。

`dominates(u,v)` は、どちらかの頂点が到達不可なら `false` です。到達可能な頂点は自分自身を支配し、始点は全ての到達可能な頂点を支配します。
`tree()` は元の頂点番号を使う長さ `n` の子リストを返します。始点への自己辺は作らず、到達不可の頂点と葉のリストは空です。

直接支配頂点の配列だけが必要なら `idom(root)` を使えます。返り値は `solve(root).idom` と同じで、支配関係の問い合わせ用データを作りません。`dominates`・`reachable`・`tree` など結果の操作を使う場合は `solve(root)` を選びます。

空グラフでは `root = 0` または `-1` を指定できます。`solve` は `root = -1`、空の `idom` を持つ結果を、`idom` は空配列を返します。1頂点では `idom[0] = 0` です。辺がない非空グラフでも、始点自身には到達できます。

`solve`・`idom` は入力を変更しません。根を変えた再実行、辺の追加後の再実行にも対応し、返された結果はグラフを参照しません。`Result` の公開メンバーは読み取りに使い、支配関係を問い合わせる間は変更しないでください。
辺の追加・予約・削除・グラフの代入後は、以前の辺要素への参照が無効になることがあります。グラフの破棄後は `edges()` の返した参照を使えません。

頂点数・辺数はそれぞれ `INT_MAX` 以下とします。頂点数を $$n$$、辺数を $$m$$ とすると、`solve`・`idom` は $$O((n+m)\log n)$$ 時間、入力・結果・作業領域を含めて $$O(n+m)$$ 空間です。`n=0,1` の `solve`・`idom` は $$O(1)$$ 時間・追加空間です。`tree()` は $$O(n)$$ 時間・空間です。
