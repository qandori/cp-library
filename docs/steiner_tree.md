---
title: 最小シュタイナー木
documentation_of: //graph/steiner_tree.hpp
library_category: graph/spanning_tree
---

非負重みの無向グラフで、指定した端点を全て連結にする最小コストの木を求めます。指定していない頂点も利用できます。厳密解を求めるため、端点数が小さい場合に使います。

```cpp
#include "lib/graph/steiner_tree.hpp"
#include <cassert>

int main() {
	SteinerTree<> graph(4);
	graph.add_edge(0, 3, 2);
	graph.add_edge(1, 3, 3);
	graph.add_edge(2, 3, 4);
	graph.add_edge(0, 1, 10);
	auto result = graph.solve({0, 1, 2});
	assert(result && result->cost == 9);
	assert(result->edges.size() == 3);
	for (int id : result->edges) {
		auto [u, v, cost] = graph.get_edge(id);
	}
	assert(graph.minimum_cost({0, 2}) == 6);
}
```

| 操作・メンバー | 内容・返り値 |
| --- | --- |
| `SteinerTree<Cost,Sum>(n = 0)` | 頂点数を指定。既定値は `Cost=int64_t`、`Sum=Cost` |
| `add_edge(u,v,cost)` | 非負重みの無向辺を追加し、0 始まりの辺 ID（`int`）を返す |
| `solve(terminals,inf = numeric_limits<Sum>::max())` | `optional<Result>`。指定端点を全て含む最小シュタイナー木 |
| `minimum_cost(terminals,inf = numeric_limits<Sum>::max())` | `optional<Sum>`。採用辺が不要な場合の最小コスト |
| `result.cost` | 採用辺の重みの総和（`Sum`） |
| `result.edges` | 採用した辺 ID の `vector<int>`。順序は未規定 |
| `get_edge(id)` | 入力した辺を `Edge{from,to,cost}` として取得 |
| `edges()` | 入力順の辺一覧を `const vector<Edge>&` で取得 |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で取得 |
| `reserve(m)` | 辺 `m` 本分の領域を予約 |
| `clear()` | 全辺を削除。頂点数は維持し、辺 ID は 0 から再開 |

頂点番号は `[0,n)`、辺 ID は `[0,edge_count())` です。`terminals` は頂点番号の `vector<int>` を渡します。順序は任意で、重複は取り除きます。異なる端点が0個または1個なら、コスト0・辺なしを返します。空グラフも扱えます。

多重辺・自己ループ・重み0の辺に対応し、採用辺には閉路を含めません。全端点が連結にできない場合は `nullopt` です。端点を含まない別の連結成分があっても構いません。`solve` と `minimum_cost` は入力を変更せず、辺の追加後にも再実行できます。返された結果はグラフを参照しません。

`Cost` は辺重み、`Sum` は経路・木の総和の型です。どちらも整数型、またはどちらも浮動小数点型とし、非負の `Cost` の全ての値を `Sum` で表現できるようにします。例えば `SteinerTree<uint32_t,uint64_t>`、`SteinerTree<int64_t,__int128_t>` が使えます。重みは有限の非負値とし、浮動小数点では通常の丸め誤差があります。

`inf` は正数で、求めたい答えより厳密に大きい値を指定します。コストが `inf` 以上の解は扱わず、答えがその範囲にしかない場合も `nullopt` を返します。整数の加算は上限を確認して行います。型の最大値と等しい答えが必要なら、より広い `Sum` を指定してください。

頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下とします。異なる端点数は31以下とし、DP の領域がメモリに収まる必要があります。端点数は指数的な時間・空間に影響するため、実用上はさらに小さい入力に使います。

入力された端点数を $$t$$、重複を除いた端点数を $$k$$、頂点数を $$n$$、辺数を $$m$$ とします。`solve`・`minimum_cost` は、64 bit 以下の整数の `Sum` では $$O(t\log t+3^k n+2^k(n+m)w)$$ 時間（$$w$$ は `Sum` のビット数）、その他の型では $$O(t\log t+3^k n+2^k(n+m)\log(n+m))$$ 時間です。空間は $$O(t+2^k n+m)$$ です。コストだけが必要なら、採用辺の復元領域を使わない `minimum_cost` を選べます。

構築は $$O(1)$$、`add_edge` は償却 $$O(1)$$、`reserve(m)` は $$O(m)$$、`clear` は $$O(m)$$、辺・サイズの参照は $$O(1)$$ です。型の演算・比較を $$O(1)$$ とします。
