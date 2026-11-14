---
title: 最大流
documentation_of: //graph/max_flow.hpp
redirect_from:
- /graph/flow/max_flow.hpp.html
- /lightweight/graph/flow/max_flow.hpp.html
library_category: graph/flow
---

有向グラフで、始点から終点へ送れる最大の流量を求めます。`flow(s,t,limit)` は `limit` 以下で可能な最大の追加流量を返します。辺に下限を付ける場合や、頂点ごとに出る量と入る量の差を指定する場合は [MaxFlowLowerBound](max_flow_lower_bound.md) を使います。

```cpp
#include "lib/graph/max_flow.hpp"
#include <cassert>

int main() {
	MaxFlow<> graph(3);
	graph.add_edge(0, 1, 5);
	graph.add_edge(1, 2, 3);

	auto flow = graph.flow(0, 2, 2);
	assert(flow == 2);
	auto next = graph.flow(0, 2);
	assert(next == 1);
	assert(graph.get_edge(0).flow == 3);
	auto cut = graph.min_cut(0);
	assert(cut[0] && !cut[2]);

	graph.reset_flow();
	assert(graph.get_edge(0).flow == 0);
}
```

表の $$N$$ は頂点数、$$M$$ は辺数です。時間は一般のグラフに対する最悪時間で、空間は返り値を含む追加空間です。

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MaxFlow<Cap>(n = 0)` | `n` 頂点で構築。`Cap` の既定は `std::int64_t` | $$O(1)$$ | $$O(1)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| `reserve(m)` | 元の辺 `m` 本分の領域を確保 | 再確保時 $$O(M)$$、確保済みなら $$O(1)$$ | 再確保時 $$O(m)$$、確保済みなら $$O(1)$$ |
| `add_edge(from,to,cap)` | 容量を指定して有向辺を追加し、0 始まりの辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `flow(s,t)` | 最大流まで流し、今回の追加流量を `Cap` で返す | $$O(N^2M)$$ | $$O(N+M)$$ |
| `flow(s,t,limit)` | `limit` 以下で可能な最大追加流量を `Cap` で返す | $$O(N^2M)$$ | $$O(N+M)$$ |
| `min_cut(s)` | 残余グラフで `s` から到達できるかを `vector<bool>` で返す | $$O(N+M)$$ | $$O(N+M)$$ |
| `get_edge(i)` | 辺番号 `i` の容量と現在の流量を `Edge` で返す | $$O(1)$$ | $$O(1)$$ |
| `edges()` | 全辺の情報を追加順の `vector<Edge>` で返す | $$O(M)$$ | $$O(M)$$ |
| `change_edge(i,cap,flow)` | 容量と現在の流量を変更。`0 <= flow <= cap` が必要 | $$O(1)$$ | $$O(1)$$ |
| `reset_flow()` | 全ての流量を 0 に戻す。頂点と辺は残す | $$O(M)$$ | $$O(1)$$ |
| `Edge` | `int from,to`、`Cap cap,flow` をこの順に持つ | —（型） | —（型） |
| `capacity_type` | `Cap` の別名 | —（型） | —（型） |

容量 `cap` の辺に流量 `f` を流すと、元の向きにさらに `cap - f`、逆向きに取り消すために `f` だけ流せます。この追加・取り消しに使える量を辺の容量として持つグラフを、残余グラフと呼びます。

頂点番号は `[0,N)`、辺番号は `[0,M)` です。自己辺・多重辺・容量 0 に対応します。`Cap` は 64 bit 以下の標準整数型で `bool` を除きます。容量・流量上限は非負、返す流量と各辺の容量は `Cap` に収めてください。内部で一時的に集める流量は 128 bit で持つため、始点から出る容量の総和が `Cap` を超える場合も扱えます。

`s`・`t` は相異なる有効な頂点にします。頂点数は `INT_MAX/2-1` 以下、元の辺数は `INT_MAX/2` 以下です。再帰を使わないため、長いパスも扱えます。

`flow` は現在の残余グラフから続け、返り値は今回の追加流量です。容量が足りなければ流せた最大量を返すため、全て流せたかは `flow == limit` で確認します。始点・終点を変えることも、辺を追加してから続けることもできます。`change_edge` で流量を変える場合は、続ける計算の始点・終点以外で入る量と出る量の差を保ってください。最小カットは、`flow(s,t)` で最大流まで流した後に `min_cut(s)` から求めます。

辺追加の「償却」は、連続して追加したときの 1 回あたりの平均です。辺追加後の最初の `flow`・`min_cut` には隣接配列の構築 $$O(N+M)$$ が加わります。辺がない場合の `flow` は $$O(N)$$ 時間です。保持空間と作業空間は $$O(N+M)$$ です。領域を余分に予約した場合や `reset_flow()` 後も、確保済みの配列領域は残ります。共有する前計算はありません。
