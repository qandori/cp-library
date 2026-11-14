---
title: Push-Relabel の内部演算
documentation_of: //components/push_relabel.hpp
library_category: components/graph
---

[最大流](https://qandori.github.io/cp-library/graph/max_flow.hpp.html) の内部部品です。始点以外の各頂点で一時的に余る流量（入る量から出る量を引いた値）は 128 bit で保持します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `flow_detail::push_relabel(graph,s,t)` | `Graph<Cap>` の残余グラフから最大流まで追加し、その流量を返す | $$O(N^2M)$$ |

公開ライブラリと同じ前提条件が必要です。頂点は異なる有効な番号を指定し、返す流量は容量型に収めます。

$$N$$ 頂点・$$M$$ 辺として、一般の上界は $$O(N^2M)$$ 時間、$$O(N+M)$$ 空間です。
