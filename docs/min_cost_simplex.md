---
title: 最小費用流の Network Simplex
documentation_of: //components/min_cost_simplex.hpp
library_category: components/graph
---

[最小費用流](min_cost_flow.md)の内部部品です。通常は `MinCostFlow` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `flow_detail::simplex_flow<Cap,Cost,Total>(graph,costs,s,t,limit,budget=0)` | 残余グラフに追加した流量・費用を `optional<pair<Cap,Total>>` で返す。作業上限を超えたらグラフを変えず `nullopt` | 時間 $$O(N+M+B)$$、追加空間 $$O(N+M)$$ |

`graph.build()` を済ませ、`costs` には残余辺ごとの単位費用を指定します。正辺・逆辺の費用は互いに符号反転し、残余グラフに負閉路がないことが必要です。容量・流量上限は非負とし、`s != t` としてください。

$$N$$ は頂点数、$$M$$ は辺数、$$B$$ は作業上限です。`budget=0` はグラフの大きさに応じた上限を選びます。費用の補正値などの途中計算には128 bit整数を使います。`nullopt` の場合は呼び出し側で[容量を上位 bit から順に扱う方法](min_cost_circulation.md)などに切り替えて解を求めます。
