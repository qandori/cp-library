---
title: Shortest Path SCC
documentation_of: //components/shortest_path_scc.hpp
library_category: components/graph
---

負辺最短路で使う、始点から到達可能な部分の強連結成分分解です。`DirectedCSR<W>` または `(行き先,重み)` の隣接リストを受け取ります。重みは読みません。再帰を使わない Tarjan 法です。

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `negative_path_detail::reachable_components(g,s)` | 到達可能な部分を分解し、`Components` を返す | $$O(n+m)$$ |
| `result.id[v]` | 成分番号。到達不能は `-1`。異なる成分への辺は番号が大きい方から小さい方へ向かう | $$O(1)$$ |
| `result.vertices` | 成分ごとに並べた到達可能頂点 | 参照 $$O(1)$$ |
| `result.offset[c]` / `result.offset[c+1]` | `vertices` 内で成分 `c` の頂点が入る半開区間の両端 | $$O(1)$$ |

`offset.size()-1` が成分数です。成分間を辺の向きに沿って処理する場合は、番号の大きい方から処理します。頂点数を $$n$$、辺数を $$m$$ として、時間は $$O(n+m)$$、追加空間は $$O(n)$$ です。
