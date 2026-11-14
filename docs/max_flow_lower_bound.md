---
title: 下限付き最大流
documentation_of: //graph/max_flow_lower_bound.hpp
library_category: graph/flow
---

各辺の流量に下限・上限を指定できる最大流です。[MaxFlow](max_flow.md) と同じく、`flow(s,t,limit)` は `limit` 以下で可能な最大の追加流量を返します。自己辺・多重辺・負の下限・負の上限に対応します。費用も指定する場合は [MinCostBFlow](min_cost_b_flow.md) を使います。

```cpp
#include "lib/graph/max_flow_lower_bound.hpp"
#include <cassert>

int main() {
	MaxFlowLowerBound<> graph(3);
	graph.add_edge(0, 1, 1, 5);
	graph.add_edge(1, 2, 1, 3);

	assert(!graph.flow(0, 2, 0));
	auto flow = graph.flow(0, 2, 2);
	assert(flow && *flow == 2);
	assert(graph.get_edge(0).flow == 2);
	auto next = graph.flow(0, 2, 4);
	assert(next && *next == 1);
	assert(graph.get_edge(0).flow == 3);

	graph.reset_flow();
	assert(graph.get_edge(0).flow == 0);
}
```

返り値は `optional<Cap>` で、成功時には今回追加した流量が入ります。`limit` は非負で、省略すると `Cap` の最大値です。容量が足りなければ、流せた最大量を返します。全て流せたかは `*result == limit` で確認します。**指定した `limit` の範囲で、下限や頂点ごとの指定を満たす流れが存在しない場合**に `std::nullopt` を返し、保存した流れは変更しません。

成功した `flow` は辺ごとの流量を保存します。繰り返すとその続きになり、始点・終点を変えて呼ぶこともできます。`reset_flow()` は保存した流れと累計の追加量を初期状態に戻し、辺と頂点の指定は残します。

`add_edge(u,v,lower,upper)` は下限・上限を指定する辺、`add_edge(u,v,upper)` は下限 0 の辺、`add_edge_lower_bound(u,v,lower)` は上限 `std::numeric_limits<Cap>::max()` の辺を追加します。省略した上限は、この型で表せる最大の有限値です。辺 `u -> v` に負の流量を流すことは、その絶対値だけ `v -> u` に流すことを表します。

各頂点について「出る流量の合計 − 入る流量の合計」を `b[v]` で指定できます。初期値は全て 0 です。`b[v] = 3` は出る量を 3 多く、`b[v] = -3` は入る量を 3 多くします。例えば、頂点 0 から頂点 2 へ 5 流すには `set_supply(0,5)`、`set_supply(2,-5)` と設定します。`flow` で `s` から `t` へ `F` 追加すると、`s` の差を `F` 増やし、`t` の差を `F` 減らします。それまでの追加量も引き継ぎ、他の頂点の差は変えません。`supply(v)` と `supplies()` は設定した `b[v]` を返し、追加量は含みません。

保存した流れを使わず、設定した辺と `b[v]` から独立に計算するには `solve`・`min_flow`・`max_flow`・`flow_range` を使います。これらは `const` で、保存した流れを参照・変更しません。

```cpp
#include "lib/graph/max_flow_lower_bound.hpp"
#include <cassert>

int main() {
	MaxFlowLowerBound<> graph(3);
	graph.add_edge_lower_bound(0, 1, 2);
	graph.add_edge(1, 2, 7);
	graph.add_edge(0, 2, 0, 1);
	graph.set_supply(0, 5);
	graph.set_supply(2, -5);

	auto result = graph.flow(0, 2, 1);
	assert(result && *result == 1);
	auto fixed = graph.solve(0, 2, -3);
	assert(fixed && fixed->value == -3);
	assert(fixed->flow[0] == 2 && fixed->flow[1] == 2);
	assert(fixed->flow[2] == 0);
	auto maximum = graph.max_flow(0, 2);
	assert(maximum && maximum->value == 3);
	auto minimum = graph.min_flow(0, 2, -5, 5);
	assert(minimum && minimum->value == -3);
	auto interval = graph.flow_range(0, 2, -5, 5);
	assert(interval && interval->first == -3 && interval->second == 3);
	assert(graph.get_edge(0).flow + graph.get_edge(2).flow == 6);
}
```

`solve()` は設定した `b[v]` をそのまま満たします。`solve(s,t,F)` は、`s` で `b[s] + F`、`t` で `b[t] - F`、他の頂点で `b[v]` を満たす流れを返します。`F` は負でも構いません。`solve(s,t,L,R)` は閉区間 `[L,R]` で可能な追加量を 1 つ選びます。条件を満たす解がなければ `std::nullopt` です。全頂点の `b[v]` の総和が 0 でない場合も解はありません。

表の $$N$$ は頂点数、$$M$$ は辺数です。固定幅整数の演算を定数時間とした最悪時間と、返り値を含む追加空間を書きます。流量計算の式は $$N\geq2$$ の場合です。0 頂点の `solve()` は定数時間、1 頂点では $$O(M)$$ 時間です。始点・終点を指定する操作には 2 頂点以上が必要です。

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MaxFlowLowerBound<Cap>(n = 0)` | `n` 頂点で構築。`Cap` の既定は `std::int64_t` | $$O(N)$$ | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| `reserve(m)` | `m` 辺分の領域を確保 | 再確保時 $$O(M)$$、確保済みなら $$O(1)$$ | 再確保時 $$O(m)$$、確保済みなら $$O(1)$$ |
| `add_edge(from,to,lower,upper)` | 有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge(from,to,upper)` | 下限 0 の有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge_lower_bound(from,to,lower)` | 上限を `Cap` の最大値とする有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `flow(s,t,limit = Capの最大値)` | 最大追加流量を `optional<Cap>` で返し、流れを保存 | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `get_edge(i)` | 辺番号 `i` の設定と保存した流量を `Edge` で返す | $$O(1)$$ | $$O(1)$$ |
| `edges()` | 全辺の設定と保存した流量を追加順の `vector<Edge>` で返す | $$O(M)$$ | $$O(M)$$ |
| `reset_flow()` | 保存した流れと累計の追加量を初期状態に戻す。辺と `b[v]` は残す | $$O(1)$$ | $$O(1)$$ |
| `change_edge(i,lower,upper)` | 下限・上限を変更し、`reset_flow()`。両端は変えない | $$O(1)$$ | $$O(1)$$ |
| `supply(v)` | 指定値 `b[v]` を `Cap` で返す | $$O(1)$$ | $$O(1)$$ |
| `supplies()` | 指定値の配列を `const vector<Cap>&` で返す | $$O(1)$$ | $$O(1)$$ |
| `set_supply(v,b)` / `add_supply(v,delta)` | `b[v]` を代入／加算し、`reset_flow()` | $$O(1)$$ | $$O(1)$$ |
| `reset_supply()` | 全頂点の `b[v]` を 0 にし、`reset_flow()` | $$O(N)$$ | $$O(1)$$ |
| `clear()` | 辺を全て消し、`b[v]` と保存した流れを初期化。頂点数は変えない | $$O(N+M)$$ | $$O(1)$$ |
| `solve()` | 設定した辺と `b[v]` を満たす解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `solve(s,t,F)` | 追加流量をちょうど `F` とした解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `solve(s,t,L,R)` | `[L,R]` 内の追加流量を 1 つ選び、解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `min_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最小追加流量の解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `max_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最大追加流量の解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `flow_range(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最小・最大追加流量を `optional<pair<Cap,Cap>>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `Edge` | `int from,to`、`Cap cap,flow,lower,upper` をこの順に持つ。`cap == upper` | —（型） | —（型） |
| `Result` | 各辺の流量 `vector<Cap> flow`、追加流量 `Cap value` を持つ | —（型） | —（型） |
| `capacity_type` | `Cap` の別名 | —（型） | —（型） |

辺を追加した順に、0 始まりの辺番号が付きます。辺追加の「償却」は、連続して追加したときの 1 回あたりの平均です。`edges()` はコピーを返し、`supplies()` は元の配列への参照を返します。`clear()` 後は以前の辺番号を使えません。

辺の追加は、それまでの保存した流量を引き継ぎ、新しい辺の保存流量を 0 にします。新しい辺の流量 0 が上下限を満たさない場合は、次の `flow` で上下限も満たすように調整します。`change_edge`・`set_supply`・`add_supply`・`reset_supply` は保存した流れを初期状態に戻します。`reserve` と独立計算の操作は保存した流れを変更しません。

`flow` が一度も成功していないときと `reset_flow()` 後は、`get_edge(i).flow` は 0 です。この初期状態や辺の追加直後には、表示された流量が下限や頂点の条件を満たすとは限りません。成功した `flow` の直後には全ての条件を満たします。

`Result` は独立計算の結果を自分の配列で持ち、以降の操作では変わりません。`flow` は長さ $$M$$ で、元の辺の向きに対する符号付き流量です。`solve()` の `value` は 0、始点・終点を指定した独立計算では設定した `b[v]` に追加する流量です。解が複数ある場合、各辺の流量の選び方は未規定です。`solve(s,t,L,R)` が返す追加量は最小・最大とは限りません。`flow_range` が `[a,b]` を返す場合、その間の全ての整数の追加流量が可能です。

頂点番号は `[0,N)`、辺番号は `[0,M)` です。`s`・`t` は相異なる有効な頂点にします。辺・流量区間には `lower <= upper`、`L <= R` が必要です。上限だけを指定する辺は `0 <= upper` が必要です。0 頂点 0 辺の `solve()` は空の流量配列と `value = 0` を持つ解を返します。自己辺は、出る量と入る量の差に影響しません。

`Cap` は 64 bit 以下の標準の符号付き整数型です。下限・上限、追加流量の区間の両端には `Cap` の最小値から最大値まで使えます。内部の容量は同じ幅の符号なし整数型で持つため、`upper - lower` が `Cap` の最大値を超えても構いません。`add_supply` の加算結果は `Cap` に収めてください。

初回の `flow` と独立計算では各辺の下限を初期流量とし、各頂点の指定値から「初期流量で出る量 − 初期流量で入る量」を差し引きます。この残りの指定値について、各頂点の絶対値と正の値の総和が、`Cap` と同じ幅の符号なし整数型に収まる必要があります。既定の `Cap = std::int64_t` では、それぞれ $$2^{64}-1$$ 以下にします。指定値の集計には 128 bit 整数を使うため、集計途中だけこの上限を超えることは構いません。始点・終点を指定する操作では、指定した区間を下限・上限とする `t -> s` の追加辺も含めて同じ条件が必要です。

`flow` を続けるときは、保存した流量を新しい辺の上下限に収めた値を初期流量にします。各頂点の指定値には、それまでの追加量も加えます。累計の追加量は 128 bit 整数で保持し、累計自体が `Cap` の最大値を超えても構いません。新しい辺の下限などを満たすための補正値には、上と同じ符号なし整数の条件が必要です。

頂点数は `INT_MAX/2-3` 以下、`reserve(m)` の `m` と登録する辺数は `INT_MAX/2` 以下にします。全ての流量計算には、補助辺の予約分を含め $$M+N+2\leq\mathrm{INT\_MAX}/2$$ が必要です。

流量計算は、補助頂点・補助辺を加えた最大流を定数回求めます。各回の最悪時間は $$O(N^2(N+M))$$、作業領域は $$O(N+M)$$ です。`flow_range` の返り値自体は $$O(1)$$ 空間です。

本体は、辺・頂点の指定に加え、保存した辺流量と累計の追加量を保持します。通常の保持空間は $$O(N+M)$$ です。`reset_flow()` は保存した流れを無効にする操作で、配列の領域は解放しません。`clear()` やコピー代入で頂点数・辺数が減った場合も以前の領域が残ることがあります。全ての配列の確保済み容量の合計を $$B$$ とすると、保持空間は $$O(B)$$ です。共有する前計算はありません。

本体のコピー構築は $$O(N+M)$$ 時間・空間、`Result` のコピー構築は $$O(M)$$ 時間・空間です。コピー代入は、代入前に保持する配列の要素数の合計を $$K$$ として、本体は $$O(K+N+M)$$ 時間、`Result` は $$O(K+M)$$ 時間です。再確保する場合の追加空間は、コピーする配列の要素数に比例します。移動構築は $$O(1)$$ 時間・追加空間、移動代入は古い配列の破棄も含め $$O(K)$$ 時間・$$O(1)$$ 追加空間です。

参考：[hitonanode/cplib-cpp](https://github.com/hitonanode/cplib-cpp/blob/master/flow/maxflow_lowerbound.hpp)、[ei1333/library](https://github.com/ei1333/library/blob/master/graph/flow/maxflow-lower-bound.hpp)。
