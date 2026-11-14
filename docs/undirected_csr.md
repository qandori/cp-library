---
title: "無向グラフの CSR"
documentation_of: //components/undirected_csr.hpp
library_category: components/graph
---

各頂点の隣接辺を 1 本の配列に続けて格納し、頂点ごとの開始位置を別の配列で持つ形式（CSR）です。

```cpp
#include "lib/components/undirected_csr.hpp"

std::vector<std::pair<int, int>> edges;
edges.emplace_back(0, 1);
UndirectedCSR graph(2, edges);
for (auto [to, id] : graph[0]) {
}
```

| 関数・メンバ | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `UndirectedCSR(n,edges)` | 無向グラフの隣接配列を構築。辺一覧は `span<const pair<int,int>>` | $$O(n+m)$$ |
| `UndirectedVertexCSR(n,edges)` | 辺 ID を保持せず、行き先だけの隣接配列を構築 | $$O(n+m)$$ |
| `graph[v]` | 頂点 `v` の出辺を `span<const Arc>` で返す | $$O(1)$$ |
| `Arc::to` / `Arc::id` | 行き先／入力順の辺 ID | $$O(1)$$ |
| `n` | 頂点数 | $$O(1)$$ |
| `start[v]` / `start[v+1]` | 頂点 `v` の出辺を格納した半開区間 | $$O(1)$$ |
| `adj[i]` | 全頂点の隣接辺を並べた配列の要素 | $$O(1)$$ |

頂点番号は 0 始まりで、多重辺・自己ループ・孤立点・空グラフに対応します。各辺を両方向に保存するため、自己ループも 2 個の出辺になります。同じ頂点からの出辺は入力順です。

辺 ID が不要なら `UndirectedVertexCSR` を使えます。`n`・`start` の意味は同じで、`adj` の要素は行き先の `int`、`graph[v]` は `span<const int>` を返します。

入力の辺一覧は構築後に破棄して構いません。構築後にメンバを変更しないでください。コピーはできず、ムーブはできます。頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下としてください。

頂点数を $$n$$、辺数を $$m$$ として、構築は $$O(n+m)$$ 時間・空間、`graph[v]` の取得は $$O(1)$$ 時間です。
