---
title: トポロジカルソート
documentation_of: //graph/topological_sort.hpp
library_category: graph
---

有向グラフの全頂点を、各辺の始点が終点より前になるように並べます。閉路がある場合は `std::nullopt` を返します。

```cpp
#include "lib/graph/topological_sort.hpp"
#include <iostream>

std::vector<std::vector<int>> graph{ {}, {}, {0}, {1} };
auto order = topological_sort(graph);       // {2, 3, 0, 1}
auto smallest = topological_sort<true>(graph); // {2, 0, 3, 1}
if (order) {
	for (int v : *order) {
		std::cout << v << ' ';
	}
}
graph[0].push_back(2);
auto cyclic = topological_sort(graph);      // std::nullopt
```

| 関数 | 返り値 | 時間計算量 |
| --- | --- | --- |
| `topological_sort(graph)` | トポロジカル順序を格納した `optional<vector<int>>` | $$O(n+m)$$ |
| `topological_sort<true>(graph)` | 辞書順最小のトポロジカル順序 | $$O(m+n\log n)$$ |

入力は `vector<vector<int>>` などの隣接リストです。`graph.size()` で頂点数を取得でき、`graph[v]` が行き先の `int` を列挙できれば、`array`・`span` なども渡せます。頂点番号は `0` 以上 `n` 未満、頂点数は `INT_MAX` 以下としてください。辺 `u -> v` は `graph[u].push_back(v)` で登録します。

非連結グラフや孤立点も含めて全頂点を返します。空グラフでは、空の `vector<int>` を格納した有効な `optional` を返します。多重辺はそのまま扱えます。自己辺は閉路と判定します。一部にだけ閉路がある場合も `nullopt` で、処理途中の順序は返しません。

通常版は入次数 0 の頂点を、候補に追加された順に処理します。最初の候補は頂点番号順に追加し、その後は入力の隣接リスト順に追加します。辞書順最小が必要な場合は `<true>` を指定してください。この場合は常に候補の最小頂点を選び、隣接リスト内の辺の順序に依存しません。

Kahn のアルゴリズムを使い、通常版では出力配列をキューとして兼用します。入力は変更せず、再帰は使いません。返された結果は入力を参照しないため、同じグラフに何度でも適用できます。

頂点数を $$n$$、辺数を $$m$$ とすると、通常版は $$O(n+m)$$ 時間、辞書順最小版は $$O(m+n\log n)$$ 時間です。どちらも出力を含めて $$O(n)$$ の追加空間を使います。
