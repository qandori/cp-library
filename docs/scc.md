---
title: 強連結成分分解
documentation_of: //graph/scc.hpp
library_category: graph/connectivity
redirect_from:
- /graph/connectivity/scc.hpp.html
- /lightweight/graph/connectivity/scc.hpp.html
---

```cpp
#include "lib/graph/scc.hpp"
#include <cassert>

int main() {
	SCCGraph graph(3);
	graph.add_edge(0, 1);
	graph.add_edge(1, 0);
	graph.add_edge(0, 2);
	graph.add_edge(1, 2);
	auto [dag, id] = graph.scc_dag();
	assert(id[0] == id[1] && id[1] < id[2]);
	assert(dag[id[0]] == std::vector<int>{id[2]});
	assert(dag[id[2]].empty());
}
```

有向グラフを強連結成分に分解します。

`SCCGraph(n)` で 0 始まりの `n` 頂点を作り、`add_edge(from,to)` で有向辺を追加します。自己ループ・多重辺・空グラフにも対応します。

`scc()` は成分の配列を返し、各成分内の頂点は昇順です。成分は、異なる成分を結ぶ辺が必ず前から後へ向かう順に並びます。独立した成分間の順序は保証しません。

`scc_ids()` は成分数と長さ `n` の成分 ID 配列を返します。同じ成分の頂点は同じ ID を持ち、ID は `scc()` の成分番号と一致します。結果を保持しないため、呼ぶたびに再計算します。辺追加後も再利用でき、コピーは独立します。

`decomposition()` は次の `Components` を返します。成分の順序・各成分の頂点順は `scc()` と同じです。

| メンバー | 内容 |
| --- | --- |
| `size()` | 成分数 |
| `id[v]` | 頂点 `v` の成分 ID |
| `operator[](c)` | 成分 `c` の頂点列を `span<const int>` で取得 |
| `vertices` | 全成分の頂点列を連結した配列 |
| `offset` | 成分 `c` が `vertices` の半開区間 `[offset[c],offset[c+1])` に対応する境界配列 |

結果はグラフを参照せず、コピーは独立します。頂点列の `span` は、その結果の破棄・代入や配列の変更まで有効です。各メンバーの参照・頂点列の取得は $$O(1)$$ です。

`scc_dag()` は強連結成分を 1 頂点ずつにまとめ、縮約 DAG と元の頂点の成分 ID を `SCCGraph::Condensation` で返します。成分 ID は `scc_ids()` と同じです。

| メンバー | 内容 |
| --- | --- |
| `dag` | `std::vector<std::vector<int>>` の隣接リスト。`dag[c]` は成分 `c` から直接辺がある成分 ID の列 |
| `id` | `std::vector<int>`。`id[v]` は元の頂点 `v` が属する成分 ID |

成分 ID は `[0,dag.size())` で、全ての辺は小さい ID から大きい ID へ向かいます。このため、ID の昇順で DAG の DP を処理できます。同じ成分内の辺は除き、同じ成分対を結ぶ辺は 1 本にまとめます。隣接リスト内の順序は保証しません。空グラフでは両方とも空です。呼ぶたびに再計算し、返り値は元のグラフから独立しています。

`SCCGraph::size()` は頂点数、`edge_count()` は辺数を、それぞれ $$O(1)$$ 時間で返します。`reserve(r)` は辺の格納容量を予約し、予約量 `r` は `std::size_t` で `INT_MAX` 以下にします。頂点数は非負の `int`、辺数は `INT_MAX` 以下、辺の端点は `[0,n)` が必要です。

頂点数を $$n$$、辺数を $$m$$ として、構築は $$O(1)$$、辺追加は償却 $$O(1)$$、分解と `scc_dag()` は $$O(n+m)$$ 時間・$$O(n+m)$$ 追加空間、グラフの保持は $$O(n+m)$$ 空間です。縮約後の辺数を $$d$$ として、`scc_dag()` の返り値は $$O(n+d)$$ 空間です。`reserve(r)` は再確保がある場合に $$O(m)$$ 時間、ない場合に $$O(1)$$ 時間で、予約した容量 $$O(r)$$ も保持空間に含めます。
