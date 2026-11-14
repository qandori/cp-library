---
title: 下限付き最小費用流（最小費用 b-flow）
documentation_of: //graph/min_cost_b_flow.hpp
library_category: graph/flow
---

各辺の流量に下限・上限を指定できる最小費用流です。[MinCostFlow](min_cost_flow.md) と同じく、`flow(s,t,limit)` は `limit` 以下で可能な最大の追加流量と、その最小費用を返します。自己辺・多重辺・負の下限・負の上限・負の費用・有限の負閉路に対応します。

```cpp
#include "lib/graph/min_cost_b_flow.hpp"
#include <cassert>

int main() {
	MinCostBFlow<> graph(3);
	graph.add_edge(0, 1, 1, 4, 2);
	graph.add_edge(1, 2, 1, 4, 3);
	graph.add_edge(0, 2, 3, 8);

	assert(!graph.flow(0, 2, 0));
	auto result = graph.flow(0, 2, 3);
	assert(result);
	auto [flow, cost] = *result;
	assert(flow == 3 && cost == 15);
	assert(graph.get_edge(0).flow == 3);

	auto next = graph.flow(0, 2, 9);
	assert(next && next->first == 4 && next->second == 29);
	assert(graph.get_edge(0).flow == 4);
	assert(graph.get_edge(2).flow == 3);
	assert(graph.potentials().size() == 3);

	graph.reset_flow();
	assert(graph.get_edge(0).flow == 0);
	assert(graph.potentials().empty());
}
```

返り値は `optional<pair<Cap,Total>>` です。成功した場合は、今回の追加流量と今回増えた費用が入ります。`limit` は非負で、省略すると `Cap` の最大値です。全て流せたかは `result->first == limit` で確認します。容量が足りなければ、流せた最大量を返します。**指定した `limit` の範囲で、下限や頂点ごとの指定を満たす流れが存在しない場合**に `std::nullopt` を返します。上の最初の呼び出しは、下限 1 の辺があるため追加量 0 では条件を満たせません。失敗した呼び出しは、保存した流れを変更しません。

成功した `flow` は辺ごとの流量を保存します。繰り返すとその続きになり、始点・終点を変えて呼ぶこともできます。初回の費用には、下限・頂点ごとの指定を満たすための費用や、負閉路を使って減らした費用も含みます。2 回目以降は、新しい流れの総費用から前回の総費用を引いた値を返します。追加流量が 0 でも、初回や辺の追加後には費用が変わることがあります。

`add_edge(u,v,upper,cost)` は下限 0、`add_edge_lower_bound(u,v,lower,cost)` は上限 `std::numeric_limits<Cap>::max()` の辺を追加します。省略した上限は、この型で表せる最大の有限値です。辺の `cost` は 1 流すときの費用で、総費用は各辺の `flow * cost` の和です。辺 `u -> v` の負の流量は、その絶対値だけ `v -> u` に流すことを表します。

各頂点について「出る流量の合計 − 入る流量の合計」を `b[v]` で指定できます。初期値は全て 0 です。`b[v] = 3` は出る量を 3 多く、`b[v] = -3` は入る量を 3 多くします。例えば、頂点 0 から頂点 2 へ 5 流すには `set_supply(0,5)`、`set_supply(2,-5)` と設定します。この指定を持つ最小費用流を「最小費用 b-flow」と呼びます。

`flow` の各呼び出しで `s` から `t` へ `F` 追加すると、`s` の「出る量 − 入る量」を `F` 増やし、`t` では `F` 減らします。それまでの追加量も引き継ぎ、他の頂点の差は変えません。`supply(v)` と `supplies()` が返すのは設定した `b[v]` で、`flow` の追加量は含みません。

保存した流れを使わず、設定した辺と `b[v]` から独立に計算するには `solve`・`min_flow`・`max_flow`・`flow_range` を使います。これらは `const` で、保存した流れを参照・変更しません。

```cpp
#include "lib/graph/min_cost_b_flow.hpp"
#include <cassert>

int main() {
	MinCostBFlow<> graph(3);
	graph.add_edge_lower_bound(0, 1, 2, 3);
	graph.add_edge(1, 2, 7, 1);
	graph.set_supply(0, 5);
	graph.set_supply(2, -5);

	auto result = graph.flow(0, 2, 1);
	assert(result && result->first == 1 && result->second == 24);
	assert(graph.get_edge(0).flow == 6);

	auto base = graph.solve();
	assert(base && base->cost == 20 && base->flow[0] == 5);
	auto fixed = graph.solve(0, 2, -3);
	assert(fixed && fixed->value == -3 && fixed->cost == 8);
	auto cheapest = graph.solve(0, 2, -3, 2);
	assert(cheapest && cheapest->value == -3 && cheapest->cost == 8);
	auto maximum = graph.max_flow(0, 2);
	assert(maximum && maximum->value == 2 && maximum->cost == 28);
	auto interval = graph.flow_range(0, 2, -5, 5);
	assert(interval && interval->first == -3 && interval->second == 2);
	assert(graph.get_edge(0).flow == 6);
}
```

`solve()` は `b[v]` をそのまま満たします。`solve(s,t,F)` は、`s` で `b[s] + F`、`t` で `b[t] - F`、他の頂点で `b[v]` を満たす最小費用解を返します。`F` は負でも構いません。`solve(s,t,L,R)` は閉区間 `[L,R]` 内で費用が最小の解を選びます。条件を満たす解がなければ `std::nullopt` です。全頂点の `b[v]` の総和が 0 でない場合も解はありません。

表の $$N$$ は頂点数、$$M$$ は辺数、$$U\geq2$$ は内部で使う容量・要求流量の最大値の上界です。固定幅整数の演算を定数時間として、最悪時間と返り値を含む追加空間を書きます。流量計算の式は $$N\geq2$$ の場合です。0 頂点の `solve()` は定数時間、1 頂点では $$O(M)$$ 時間です。始点・終点を指定する操作には 2 頂点以上が必要です。

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MinCostBFlow<Cap,Cost,Total>(n = 0)` | `n` 頂点で構築。既定は `std::int64_t`・`std::int64_t`・`__int128_t` | $$O(N)$$ | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| `reserve(m)` | `m` 辺分の領域を確保 | 再確保時 $$O(M)$$、確保済みなら $$O(1)$$ | 再確保時 $$O(m)$$、確保済みなら $$O(1)$$ |
| `add_edge(from,to,lower,upper,cost)` | 有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge(from,to,upper,cost)` | 下限 0 の有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge_lower_bound(from,to,lower,cost)` | 上限を `Cap` の最大値とする有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `flow(s,t,limit = Capの最大値)` | 最大追加流量と今回増えた費用を `optional<pair<Cap,Total>>` で返し、流れを保存 | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `get_edge(i)` | 辺番号 `i` の設定と保存した流量を `Edge` で返す | $$O(1)$$ | $$O(1)$$ |
| `edges()` | 全辺の設定と保存した流量を追加順の `vector<Edge>` で返す | $$O(M)$$ | $$O(M)$$ |
| `potentials()` | 保存した流れの補正値を `const vector<Cost>&` で返す | $$O(1)$$ | $$O(1)$$ |
| `reset_flow()` | 保存した流れと累計の追加量を初期状態に戻す。辺と `b[v]` は残す | $$O(1)$$ | $$O(1)$$ |
| `change_edge(i,lower,upper,cost)` | 下限・上限・単位費用を変更し、`reset_flow()`。両端は変えない | $$O(1)$$ | $$O(1)$$ |
| `set_supply(v,b)` / `add_supply(v,delta)` | `b[v]` を代入／加算し、`reset_flow()` | $$O(1)$$ | $$O(1)$$ |
| `supply(v)` | 指定値 `b[v]` を `Cap` で返す | $$O(1)$$ | $$O(1)$$ |
| `supplies()` | 指定値の配列を `const vector<Cap>&` で返す | $$O(1)$$ | $$O(1)$$ |
| `reset_supply()` | 全頂点の `b[v]` を 0 にし、`reset_flow()` | $$O(N)$$ | $$O(1)$$ |
| `clear()` | 辺を全て消し、`b[v]` と保存した流れを初期化。頂点数は変えない | $$O(N+M)$$ | $$O(1)$$ |
| `solve()` | 設定した辺と `b[v]` の最小費用解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `solve(s,t,F)` | 追加流量をちょうど `F` とした最小費用解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `solve(s,t,L,R)` | 追加流量を `[L,R]` から選び、費用が最小の解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `min_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最小追加流量を選び、その中で費用を最小にした解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `max_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最大追加流量を選び、その中で費用を最小にした解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `flow_range(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内の最小・最大追加流量を `optional<pair<Cap,Cap>>` で返す。費用は考慮しない | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `Edge` | `int from,to`、`Cap cap,flow`、`Cost cost`、`Cap lower,upper` をこの順に持つ。`cap == upper` | —（型） | —（型） |
| `Result` | `Total cost`、`vector<Cap> flow`、`vector<Cost> potential`、`Cap value` を持つ | —（型） | —（型） |
| `capacity_type` / `cost_type` / `total_type` | それぞれ `Cap`・`Cost`・`Total` の別名 | —（型） | —（型） |

辺を追加した順に、0 始まりの辺番号が付きます。辺追加の「償却」は、連続して追加したときの 1 回あたりの平均です。`edges()` はコピーを返し、`supplies()` は元の配列への参照を返します。`clear()` 後は以前の辺番号を使えません。

辺の追加は、それまでの保存した流量を引き継ぎ、新しい辺の保存流量を 0 にします。新しい辺の流量 0 が上下限を満たさない場合は、次の `flow` で上下限も満たすように調整します。辺の追加直後は `potentials()` が空になります。`change_edge`・`set_supply`・`add_supply`・`reset_supply` は保存した流れを初期状態に戻します。`reserve` と独立計算の操作は保存した流れを変更しません。

`flow` が一度も成功していないときと `reset_flow()` 後は、`get_edge(i).flow` は 0、`potentials()` は空です。この初期状態や辺の追加直後には、表示された流量が下限や頂点の条件を満たすとは限りません。成功した `flow` の直後には全ての条件を満たします。補正値の参照は、次の変更操作まで有効です。

`Result` は独立計算の結果を自分の配列で持ち、以降の操作では変わりません。`flow` は長さ $$M$$、`potential` は長さ $$N$$ で、`flow[i]` は元の辺 `i` の向きに対する符号付き流量です。`solve()` の `value` は 0、始点・終点を指定した独立計算では設定した `b[v]` に追加する流量です。`flow_range` が `[a,b]` を返す場合、その間の全ての整数の追加流量が可能です。同じ費用の解が複数ある場合、どれを返すかは未規定です。既定の `Total = __int128_t` は [FastOutput](fast_io.md) で出力できます。

辺 `u -> v` に流量 `f` があるとき、元の向きには `upper - f` だけ追加でき、逆向きには `f - lower` だけ取り消せます。追加・取り消しが可能な辺を集めたものを残余グラフと呼び、逆向きの費用は元の費用の符号を反転します。補正値 `p[v]` は、残余容量が正の辺の `cost + p[u] - p[v]` を非負にする値です。元の辺で `f < upper` ならこの値が 0 以上、`lower < f` なら 0 以下となり、最小費用であることを確認できます。

頂点番号は `[0,N)`、辺番号は `[0,M)` です。`s`・`t` は相異なる有効な頂点にします。辺・流量区間には `lower <= upper`、`L <= R` が必要です。上限だけを指定する辺は `0 <= upper` が必要です。0 頂点 0 辺の `solve()` は費用 0、空の配列を持つ解を返します。自己辺は、出る量と入る量の差に影響しませんが、費用には含まれます。

`Cap` と `Cost` は 64 bit 以下の標準の符号付き整数型、`Total` は符号付き整数型です。`Cost` の最小値は単位費用に使えません。`add_supply` の加算結果は `Cap` に収めます。辺の下限・上限、流量区間の両端には、`Cap` の最小値から最大値まで使えます。内部の容量は同じ幅の符号なし整数型で持つため、`upper - lower` が `Cap` の最大値を超えても構いません。

指定した追加流量の区間は、まず始点・終点に接続する辺の上下限から得られる範囲と交差させます。空になれば解はありません。始点・終点の条件から不可能と分かる流量は、この段階で除きます。

内部で扱う流量の範囲には、次の条件があります。各辺に仮の流量を置き、各頂点に指定された値から「仮の流量で出る量 − 仮の流量で入る量」を差し引きます。この残りの値について、各頂点の絶対値と正の値の総和が、`Cap` と同じ幅の符号なし整数型に収まる必要があります。既定の `Cap = std::int64_t` では、それぞれ $$2^{64}-1$$ 以下です。仮の流量は、次のいずれかの選び方でこの条件を満たせば使えます。

- 費用が非負の辺は下限、負の辺は上限を選ぶ。
- 各辺の `[lower,upper]` のうち、`flow` で保存した流量に最も近い値を選ぶ。保存した流れを使わない計算では 0 に最も近い値を選びます。区間が基準の値を含むならその値、基準の値より大きい区間なら下限、小さい区間なら上限です。

`flow` では `b[v]` に前回までの追加量を加えた値を使います。累計の追加量は 128 bit 整数で保持し、累計自体が `Cap` の最大値を超えても構いません。指定値の集計にも 128 bit 整数を使うため、集計途中だけこの上限を超えることは構いません。`solve` などで流量区間を指定する場合は、区間を下限・上限とする `t -> s` の追加辺も含めて同じ条件が必要です。最大・最小流量を調べる処理では、元の辺の費用を 0、追加辺の費用を ±1 にした場合にも、上のいずれかの選び方で条件を満たす必要があります。

最短距離・補正値・補正後の費用と、それらの計算途中は `Cost` に、費用の積・合計とその計算途中は `Total` に収めてください。`flow` では、各回に返す費用の差分と、保存する累計の総費用の両方を `Total` に収めてください。有限の最短距離は `Cost` の最大値未満にします。この値は到達できないことを表すために使います。補助頂点の分も含め、$$N\leq\mathrm{INT\_MAX}-2$$ が必要です。`reserve(m)` の `m` と登録する辺数は `INT_MAX/2` 以下にします。補助辺と内部計算用の辺を含め、`solve()` は $$M+N+1$$、始点・終点を指定する操作は $$M+N+2$$ を `INT_MAX/2` 以下にします。

`flow`・`min_flow`・`max_flow` は流量の決定と費用の最小化、`flow_range` は両端の決定のためにそれぞれ 2 回計算しますが、定数回なので表の計算量は変わりません。作業領域と返す解はともに $$O(N+M)$$、`flow_range` の返り値自体は $$O(1)$$ 空間です。

本体は、辺・頂点の指定に加え、保存した辺流量・累計の追加量・補正値を保持します。通常の保持空間は $$O(N+M)$$ です。`reset_flow()` は保存した流れを無効にする操作で、配列の領域は解放しません。`clear()` やコピー代入で頂点数・辺数が減った場合も以前の領域が残ることがあります。全ての配列の確保済み容量の合計を $$B$$ とすると、保持空間は $$O(B)$$ です。共有する前計算はありません。

本体のコピー構築は $$O(N+M)$$ 時間・空間、`Result` のコピー構築は $$O(N+M)$$ 時間・空間です。コピー代入は、代入前に保持する配列の要素数の合計を $$K$$ として、本体は $$O(K+N+M)$$ 時間、`Result` は $$O(K+N+M)$$ 時間です。再確保する場合の追加空間は、コピーする配列の要素数に比例します。移動構築は $$O(1)$$ 時間・追加空間、移動代入は古い配列の破棄も含め $$O(K)$$ 時間・$$O(1)$$ 追加空間です。

参考：[Library Checker の Min Cost b-Flow](https://judge.yosupo.jp/problem/min_cost_b_flow)、[ACL の最小費用流](https://atcoder.github.io/ac-library/production/document_ja/mincostflow.html)、[hitonanode/cplib-cpp](https://github.com/hitonanode/cplib-cpp/blob/master/flow/b-flow.hpp)、[sotanishy/cp-library-cpp](https://github.com/sotanishy/cp-library-cpp/blob/main/docs/flow/min_cost_b_flow.md)。
