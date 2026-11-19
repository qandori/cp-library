---
title: Push-Relabel の内部演算
documentation_of: //components/push_relabel.hpp
library_category: components/graph
---

[最大流](https://qandori.github.io/cp-library/flow/max_flow.hpp.html) の内部部品です。容量型を超える一時的な流量にも対応します。

$$N$$ を頂点数、$$M$$ を辺数とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `flow_detail::push_relabel(graph,s,t)` | `Graph<Cap>` の残余グラフから最大流まで追加し、その流量を返す | $$O(N^2M)$$ |

公開ライブラリと同じ前提条件が必要です。頂点は異なる有効な番号を指定し、返す流量は容量型に収めます。

一般の上界は $$O(N^2M)$$ 時間、$$O(N+M)$$ 空間です。
