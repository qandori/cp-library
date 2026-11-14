---
title: 区間辺グラフ
documentation_of: //graph/range_edge_graph.hpp
library_category: graph
---

番号が連続する頂点集合への有向辺をまとめて追加します。頂点から区間、区間から頂点、区間から区間への辺を扱えます。

```cpp
#include "lib/graph/range_edge_graph.hpp"
#include "lib/graph/dijkstra.hpp"

int main() {
    RangeEdgeGraph<> graph(5);
    graph.add_to_range(0, 1, 4, 3); // 0 -> 1,2,3 にそれぞれ重み 3
    graph.add_from_range(1, 3, 4, 2); // 1,2 -> 4 にそれぞれ重み 2
    graph.add_range_to_range(3, 4, 1, 3, 1); // 3 -> 1,2 にそれぞれ重み 1
    graph.add_edge(0, 4, 10);

    auto compressed = graph.build();
    auto distance = dijkstra(compressed, 0);
    distance.resize(graph.size()); // 元の頂点への距離だけを残す
    // distance = {0,3,3,3,5}

    auto [cost, path] = dijkstra_path(compressed, 0, 4);
    auto vertices = graph.original_path(path);
    // cost = 5。vertices は {0,1,4} または {0,2,4}
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `RangeEdgeGraph<Cost=std::int64_t>(n=0)` | 元の頂点数を指定して構築 |
| `size()` | 元の頂点数を `int` で返す |
| `vertex_count()` | 補助頂点を含む頂点数を `int` で返す |
| `edge_count()` | 補助辺を含む辺数を `std::size_t` で返す |
| `reserve_edges(m)` | 圧縮後の辺を `m` 本まで格納できる容量を予約。返り値は `void` |
| `add_edge(from,to,weight=1)` | 頂点 `from` から頂点 `to` への辺を追加 |
| `add_to_range(from,l,r,weight=1)` | `from` から `[l,r)` の各頂点への辺を追加 |
| `add_from_range(l,r,to,weight=1)` | `[l,r)` の各頂点から `to` への辺を追加 |
| `add_range_to_range(l1,r1,l2,r2,weight=1)` | `[l1,r1)` の各頂点から `[l2,r2)` の各頂点への辺を追加 |
| `edges()` | 圧縮後の全辺を `const std::vector<Edge>&` で返す。`Edge` は `int from,to; Cost weight;` を持つ |
| `build()` | 圧縮後のグラフを `DirectedCSR<Cost>` で返す |
| `adjacency_list()` | 圧縮後のグラフを `std::vector<std::vector<std::pair<int,Cost>>>` で返す。各辺は `(行き先,重み)` |
| `original_path(path)` | 圧縮後の頂点列から補助頂点を取り除き、元の頂点列を `std::vector<int>` で返す |

4 種類の辺追加関数の返り値は `void` です。いずれも、指定した重みの辺が各始点・終点の組に 1 本ずつある場合と同じ経路と重みを表します。区間から区間への移動でも、指定した重みを払うのは 1 回です。

元の頂点番号は `[0,n)`、各区間は `0 <= l <= r <= n` の半開区間です。点を引数に取る辺追加関数には元の頂点番号を渡してください。自己ループ・多重辺・重なる区間に対応します。空区間への追加は何も行わず、区間から区間への追加も一方が空なら何も行いません。

`0 <= n <= INT_MAX/3` とし、補助頂点を含む頂点数は `INT_MAX` 以下にしてください。空のグラフも作成できます。

重みはコピー可能で `Cost(0)` を構築できる型を指定します。`build()` を使う場合は `Cost` のデフォルト構築も必要です。重みの省略には `Cost(1)` も構築できる必要があります。負の重みも追加できますが、[Dijkstra](dijkstra.md) を使う場合は全て非負にします。負の重みを含む最短経路には [Bellman–Ford](bellman_ford.md) など、重みに適したアルゴリズムを使ってください。

元の頂点番号は圧縮後も変わらず、補助頂点は `n` 以上の番号になります。`build()`・`adjacency_list()` の頂点数と、最短距離配列の長さは `vertex_count()` です。元の頂点への答えだけが必要な場合は、先頭の `n` 要素を使います。CSR 版の Dijkstra は 32 bit 以下の整数重みを 64 bit 整数の距離へ拡張します。隣接リスト版の距離は `Cost` と同じ型です。到達不能値や距離型・無限大の指定は、利用する最短経路ライブラリに従います。

`original_path` は `dijkstra_path` などが返す圧縮後の経路を渡して使います。到達不能な空の経路は空のまま返し、始点と終点が同じなら通常はその頂点だけが残ります。渡す番号は `[0,vertex_count())` とします。論理的な辺の ID は返しません。

隣接リストを使う場合は、例の `auto compressed = graph.build();` を `auto compressed = graph.adjacency_list();` に置き換えられます。グラフを取り出した後も辺を追加できますが、取り出し済みのグラフへは反映されません。再び `build()` または `adjacency_list()` を呼んでください。コピーは独立します。`edges()` の参照とその要素には、通常の `std::vector` と同じ有効期間の規則が適用されます。

元の頂点数を $$N$$、点から点の辺追加回数を $$P$$、区間を含む辺追加回数を $$Q$$、両区間が空でない区間間の辺追加回数を $$R$$ とします。圧縮後の頂点数は $$O(N+R)$$、辺数は $$O(N+P+Q\log N)$$ です。

構築は $$O(N)$$、点から点への追加は償却 $$O(1)$$、区間を含む追加は償却 $$O(\log N)$$ 時間です。対数を含む評価は $$N\ge2$$ に対するもので、単一頂点のグラフや空区間の追加は定数時間です。単一頂点のグラフでは辺数は $$O(P+Q)$$ です。圧縮後の頂点数・辺数を $$V,E$$ とすると、保持空間は $$O(V+E)$$、グラフの取り出しは $$O(V+E)$$ 時間・追加空間です。`original_path` は渡した列の長さに比例する時間・追加空間、`reserve_edges` は既存辺数に比例する時間、その他の取得は $$O(1)$$ 時間です。型のコピー・構築を定数時間とします。
