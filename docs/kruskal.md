---
title: Kruskal（最小全域木）
documentation_of: //graph/kruskal.hpp
library_category: graph/spanning_tree
---

重み付き無向グラフの最小全域木を求めます。非連結グラフでは、各連結成分の最小全域木を合わせた最小全域森を返します。

```cpp
#include "lib/graph/kruskal.hpp"

Kruskal<> graph(4);
graph.add_edge(0, 1, 3); // 辺 ID 0
graph.add_edge(1, 2, 1); // 辺 ID 1
graph.add_edge(0, 2, 2); // 辺 ID 2
graph.add_edge(2, 3, 4); // 辺 ID 3
auto result = graph.solve();
assert(result.connected());
assert(result.cost == 7);
assert((result.edges == std::vector<int>{1, 2, 3}));
for (int id : result.edges) {
	auto [u, v, cost] = graph.get_edge(id);
}
```

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Kruskal<Cost,Sum>(n = 0)` | 頂点数を指定。既定値は `Cost=int64_t`、`Sum=Cost` | $$O(1)$$ |
| `add_edge(u,v,cost)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `solve()` | 最小全域森の `Result` を返す | $$O(n+m\log m)$$ |
| `result.cost` | 採用した辺の重みの総和（`Sum`） | $$O(1)$$ |
| `result.edges` | 採用した辺 ID の `vector<int>` | $$O(1)$$ |
| `result.components` | 元のグラフの連結成分数。孤立点も数える | $$O(1)$$ |
| `result.connected()` | 連結か。空グラフも `true` とする | $$O(1)$$ |
| `get_edge(id)` | 辺を `Edge{from,to,cost}` として取得 | $$O(1)$$ |
| `edges()` | 入力順の辺一覧を `const vector<Edge>&` で取得 | $$O(1)$$ |
| `size()` / `edge_count()` | 頂点数／辺数 | $$O(1)$$ |
| `reserve(m)` | 辺 `m` 本分の領域を予約 | $$O(m)$$ |
| `clear()` | 全ての辺を削除。頂点数は維持し、辺 ID は 0 から再開 | $$O(m)$$ |

頂点番号は `[0,n)` です。負の重み・多重辺・自己ループ・孤立点に対応します。自己ループは採用しません。採用する辺数は `n-result.components` です。空グラフでは総和 `0`、辺一覧は空、成分数 `0` を返します。

辺は重みの昇順で採用し、同じ重みなら辺 ID が小さいものを優先します。`solve()` は入力を変更せず、辺の追加後にも再実行できます。返された結果はグラフを参照しません。

`Cost` は各辺の重みの型、`Sum` は総和の型です。どちらも整数型、またはどちらも浮動小数点型とし、`Sum` は `Cost` の全ての値を表現できる型を指定します。例えば `Kruskal<uint32_t,uint64_t>` は辺を 32 bit、総和を 64 bit で扱います。128 bit の総和には `Kruskal<int64_t,__int128_t>` を使えます。

整数の加算結果が `Sum` に収まらない場合は `std::overflow_error` を送出します。採用順の途中の総和も表現できる型を選んでください。浮動小数点の重みに NaN は使えず、総和には通常の丸め誤差があります。

頂点数・辺数はそれぞれ `INT_MAX` 以下とします。頂点数を $$n$$、辺数を $$m$$ とすると、構築全体は $$O(n+m\log m)$$ 時間、入力・結果・作業領域を含めて $$O(n+m)$$ 空間です。
