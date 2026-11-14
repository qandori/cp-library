---
title: 有向最小全域木
documentation_of: //graph/directed_mst.hpp
library_category: graph/spanning_tree
---

指定した根から全頂点へ到達できる有向木のうち、辺の重みの総和が最小のものを求めます。根以外の各頂点には、採用した入辺がちょうど1本あります。

```cpp
#include "lib/graph/directed_mst.hpp"
#include <cassert>
#include <vector>

int main() {
	DirectedMST<> graph(4);
	graph.add_edge(0, 1, 5);
	graph.add_edge(0, 2, 4);
	graph.add_edge(1, 2, 1);
	graph.add_edge(2, 1, -2);
	graph.add_edge(1, 3, 3);
	graph.add_edge(2, 3, 8);
	auto result = graph.solve(0);
	assert(result && result->cost == 5);
	assert((result->parent == std::vector<int>{0, 2, 0, 1}));
	assert((result->parent_edge == std::vector<int>{-1, 3, 1, 4}));
}
```

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DirectedMST<Cost,Sum>(n = 0)` | 頂点数を指定。既定値は `Cost=std::int64_t`、`Sum=Cost` | $$O(1)$$ |
| `add_edge(u,v,cost)` | 有向辺 `u -> v` を追加し、0始まりの辺 ID を `int` で返す | 償却 $$O(1)$$ |
| `solve(root = 0)` | 根を指定し、解を `std::optional<Result>` で返す。全頂点へ到達できなければ `std::nullopt` | $$O(n+m\log m)$$ |
| `result->cost` | 採用した辺の重みの総和（`Sum`） | $$O(1)$$ |
| `result->parent` | 各頂点の親を持つ `std::vector<int>`。根自身の親は根 | $$O(1)$$ |
| `result->parent_edge` | 各頂点への採用入辺 ID を持つ `std::vector<int>`。根は `-1` | $$O(1)$$ |
| `result->edges` | 採用した辺 ID の `std::vector<int>`。順序は任意 | $$O(1)$$ |
| `get_edge(id)` | 辺を `Edge{from,to,cost}` として取得 | $$O(1)$$ |
| `edges()` | 入力順の辺一覧を `const std::vector<Edge>&` で取得 | $$O(1)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で取得 | $$O(1)$$ |
| `reserve(m)` | 辺 `m` 本分の領域を予約 | $$O(m)$$ |
| `clear()` | 全ての辺を削除。頂点数は維持し、辺 ID は0から再開 | $$O(m)$$ |

頂点番号は `[0,n)`、辺 ID は `[0,edge_count())` です。非空グラフの `root` は `[0,n)` とします。
負の重み・多重辺・自己ループに対応します。自己ループと根への入辺は採用しません。同じ最小総和を持つ解が複数あれば、そのうち任意の1つを返します。

`result` に解があることを確認してから、そのメンバーへアクセスします。非空グラフで解がある場合、`parent`・`parent_edge` の長さは `n`、`edges` の長さは `n-1` です。根以外の頂点 `v` では、`get_edge(result->parent_edge[v])` の始点が `result->parent[v]`、終点が `v` です。
空グラフは `root = 0` または `-1` とし、総和0・空の配列を持つ解を返します。1頂点のグラフは、辺がなくても総和0の解を返します。

`solve` は入力を変更しません。根を変えた再実行、辺の追加後の再実行もできます。返された結果はグラフを参照しません。辺の追加・予約・削除・グラフの代入後は、以前の辺要素への参照が無効になることがあります。グラフの破棄後は `edges()` の返した参照を使えません。

`Cost` は64 bit以下の符号付き組み込み整数型、`Sum` は `Cost` の全ての値を表せる128 bit以下の符号付き組み込み整数型です。128 bitの総和には `DirectedMST<std::int64_t,__int128_t>` を使えます。浮動小数点型・符号なし整数型は指定できません。 辺の重みが32 bitに収まる場合は `DirectedMST<std::int32_t,std::int64_t>` のように、辺の重みと総和の型を分けられます。
計算途中は総和や重みの差を拡張して扱います。最終的な最小総和が `Sum` に収まらない場合は `std::overflow_error` を送出します。途中の和が `Sum` を超えても、最終値が収まれば返せます。

頂点数は `0 <= n <= INT_MAX/2`、辺数は `0 <= m <= INT_MAX` とします。頂点数を $$n$$、辺数を $$m$$ とすると、`solve` は $$O(n+m\log m)$$ 時間、入力・結果・作業領域を含めて $$O(n+m)$$ 空間です。`n=0,1` の `solve` は $$O(1)$$ 時間・追加空間です。
