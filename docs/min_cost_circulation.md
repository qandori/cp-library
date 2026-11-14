---
title: 最小費用流の容量スケーリング
documentation_of: //components/min_cost_circulation.hpp
library_category: components/graph
---

[最小費用流](https://qandori.github.io/cp-library/graph/min_cost_flow.hpp.html) の内部部品です。容量を 2 進数の上位 bit から順に扱う、容量スケーリング法の処理を共有します。通常は `MinCostFlow` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `flow_detail::scaling_flow<Cap,Cost,Total>(graph,costs,s,t,limit)` | 残余グラフに追加した流量と費用を `pair<Cap,Total>` で返す | $$O(M(N+M\log N)\log U)$$ |

`graph.build()` を済ませ、残余辺ごとの単位費用を `costs` に入れます。正辺・逆辺の費用は互いに符号反転した値です。公開ライブラリと同じ前提条件で使います。最短路と補助辺の費用は 128 bit、ヒープは 4 分木です。

$$N$$ 頂点・$$M$$ 辺、容量・流量上限の最大値を $$U\geq1$$ として、$$O(M(N+M\log N)\log U)$$ 時間、$$O(N+M)$$ 空間です。
