---
title: 下限付き最大流
documentation_of: //graph/max_flow_lower_bound.hpp
library_category: graph/flow
---

各辺の流量に下限・上限を指定し、条件を満たす流れ、最小流量・最大流量、可能な流量の範囲を求めます。自己辺・多重辺・負の下限・負の上限に対応します。費用も指定する場合は [MinCostBFlow](min_cost_b_flow.md) を使います。

`add_edge(u,v,lower,upper)` は下限・上限を指定する辺、`add_edge(u,v,upper)` は下限 0 の辺、`add_edge_lower_bound(u,v,lower)` は上限 `std::numeric_limits<Cap>::max()` の辺を追加します。省略した上限は `Cap` で表せる最大の有限値です。数学的な無限大としては扱わず、流量が際限なく増やせるかどうかを判定する機能もありません。最大流量がこの既定の上限になった場合も、その有限値を返します。

各頂点について「出る流量の合計 − 入る流量の合計」を `b[v]` で指定します。`b[v] = 3` は出る量を 3 多く、`b[v] = -3` は入る量を 3 多くします。初期値は全て 0 で、入る量と出る量を等しくします。例えば、頂点 0 から頂点 2 へ 5 流すには `set_supply(0,5)`、`set_supply(2,-5)` と設定します。

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
	auto result = graph.solve();
	assert(result && result->value == 0);
	assert(result->flow[0] + result->flow[2] == 5);
	assert(result->flow[0] == result->flow[1]);
	auto fixed = graph.flow(0, 2, -3);
	assert(fixed && fixed->value == -3);
	assert(fixed->flow[0] == 2 && fixed->flow[1] == 2);
	assert(fixed->flow[2] == 0);
	auto maximum = graph.max_flow(0, 2);
	assert(maximum && maximum->value == 3);
	assert(maximum->flow[0] == 7 && maximum->flow[1] == 7);
	assert(maximum->flow[2] == 1);
	auto minimum = graph.min_flow(0, 2, -5, 5);
	assert(minimum && minimum->value == -3);
	auto interval = graph.flow_range(0, 2, -5, 5);
	assert(interval && interval->first == -3 && interval->second == 3);
}
```

`solve()` は設定した `b[v]` をそのまま満たす流れを返します。始点・終点を指定する操作の追加流量を `F` とすると、始点は `b[s] + F`、終点は `b[t] - F`、他の頂点は `b[v]` を満たします。上の例では、`flow(0,2,-3)` は設定済みの 5 に -3 を加え、合計 2 を流します。`flow(s,t,F)` はちょうど `F` の指定です。

全ての流量計算は `const` で、毎回、登録した辺と `b[v]` から計算します。前回の計算で流した量は引き継ぎません。返した結果は配列を自分で持ち、以降の計算・辺や `b[v]` の変更によって書き換わりません。条件を満たす流れがなければ `std::nullopt` を返します。全頂点の `b[v]` の総和が 0 でない場合も解はありません。

表の $$N$$ は頂点数、$$M$$ は辺数です。固定幅整数の演算を定数時間とした最悪時間と、返り値を含む追加空間を書きます。流量計算の式は $$N\geq2$$ の場合です。0 頂点の `solve()` は定数時間、1 頂点では $$O(M)$$ 時間です。始点・終点を指定する操作には 2 頂点以上が必要です。

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MaxFlowLowerBound<Cap>(n = 0)` | `n` 頂点で構築。`Cap` の既定は `std::int64_t` | $$O(N)$$ | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| `reserve(m)` | `m` 辺分の領域を確保 | 再確保時 $$O(M)$$、確保済みなら $$O(1)$$ | 再確保時 $$O(m)$$、確保済みなら $$O(1)$$ |
| `add_edge(from,to,lower,upper)` | 有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge(from,to,upper)` | 下限 0 の有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `add_edge_lower_bound(from,to,lower)` | 上限を `Cap` の最大値とする有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `get_edge(i)` | 辺番号 `i` の設定を `Edge` で返す | $$O(1)$$ | $$O(1)$$ |
| `edges()` | 全辺の設定を追加順の `vector<Edge>` で返す | $$O(M)$$ | $$O(M)$$ |
| `change_edge(i,lower,upper)` | 辺番号 `i` の下限・上限を変更。両端は変えない | $$O(1)$$ | $$O(1)$$ |
| `supply(v)` | 頂点の指定値 `b[v]` を `Cap` で返す | $$O(1)$$ | $$O(1)$$ |
| `supplies()` | 頂点ごとの指定値の配列を `const vector<Cap>&` で返す | $$O(1)$$ | $$O(1)$$ |
| `set_supply(v,b)` / `add_supply(v,delta)` | 「出る流量 − 入る流量」の指定値 `b[v]` を代入／加算 | $$O(1)$$ | $$O(1)$$ |
| `reset_supply()` | 全頂点の `b[v]` を 0 にする | $$O(N)$$ | $$O(1)$$ |
| `clear()` | 辺を全て消し、全頂点の `b[v]` を 0 にする。頂点数は変えない | $$O(N+M)$$ | $$O(1)$$ |
| `solve()` | 設定した辺と `b[v]` を満たす解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `flow(s,t,F)` | 追加流量をちょうど `F` とした解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `flow(s,t,L,R)` | 閉区間 `[L,R]` 内で可能な追加流量を 1 つ選び、解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `min_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で可能な最小の追加流量の解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `max_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で可能な最大の追加流量の解を `optional<Result>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `flow_range(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で可能な追加流量の最小値・最大値を `optional<pair<Cap,Cap>>` で返す | $$O(N^2(N+M))$$ | $$O(N+M)$$ |
| `Edge` | `int from,to`、`Cap lower,upper` を持つ | —（型） | —（型） |
| `Result` | 各辺の流量 `vector<Cap> flow`、追加流量 `Cap value` を持つ | —（型） | —（型） |
| `capacity_type` | `Cap` の別名 | —（型） | —（型） |

辺を追加した順に 0 始まりの辺番号が付きます。辺追加の「償却」は、連続して追加したときの 1 回あたりの平均です。`edges()` はコピーを返し、`supplies()` は元の配列への参照を返します。後者には、その後の `b[v]` の変更も反映します。`clear()` を呼ぶと、それまでの辺番号は使えなくなります。

`Result::flow` は長さ $$M$$ の配列で、`flow[i]` は元の辺 `i` の向きに対する符号付き流量です。必ず `lower <= flow[i] <= upper` を満たします。辺 `u -> v` に負の流量を流すことは、その絶対値だけ `v -> u` に流すことと同じです。`solve()` の `value` は 0、始点・終点を指定した操作では追加流量です。解が複数ある場合、各辺の流量の選び方は未規定です。`flow(s,t,L,R)` が返す追加流量も、最小・最大とは限りません。`flow_range` が `[a,b]` を返した場合、その間の全ての整数の追加流量が可能です。

頂点番号は `[0,N)`、辺番号は `[0,M)` です。`s`・`t` は相異なる有効な頂点にします。辺・流量区間は `lower <= upper`、`L <= R` を満たす必要があります。上限だけを指定する辺は `0 <= upper` が必要です。0 頂点 0 辺の `solve()` は空の流量配列と `value = 0` を持つ解を返します。自己辺は、出る流量と入る流量の差に影響しません。

`Cap` は 64 bit 以下の標準の符号付き整数型です。下限・上限、追加流量の区間の両端には `Cap` の最小値から最大値まで使えます。内部の容量は同じ幅の符号なし整数型で持つため、`upper - lower` が `Cap` の最大値を超えても構いません。`add_supply` の加算結果は `Cap` に収めてください。

計算では各辺の下限を初期流量とし、各頂点の指定値から「初期流量で出る量 − 初期流量で入る量」を差し引きます。この残りの指定値について、各頂点の絶対値と正の値の総和が、`Cap` と同じ幅の符号なし整数型に収まる必要があります。既定の `Cap = std::int64_t` では、それぞれ $$2^{64}-1$$ 以下にします。指定値の集計には 128 bit 整数を使うため、集計途中だけこの上限を超えることは構いません。始点・終点を指定する操作では、指定した区間を下限・上限とする `t -> s` の追加辺も含めて同じ条件が必要です。

頂点数は `INT_MAX/2-3` 以下、`reserve(m)` の `m` と登録する辺数は `INT_MAX/2` 以下にします。全ての流量計算には、補助辺の予約分を含め $$M+N+2\leq\mathrm{INT\_MAX}/2$$ が必要です。

流量計算は、補助頂点・補助辺を加えた最大流を定数回求めます。各回の最悪時間は $$O(N^2(N+M))$$、作業領域は $$O(N+M)$$ です。`flow_range` の返り値自体は $$O(1)$$ 空間です。

入力を保持する配列の確保済み容量を、頂点の指定値について $$S$$、辺について $$C$$ として、保持空間は $$O(S+C)$$ です。通常の構築では $$S=N$$ ですが、コピー代入で頂点数を減らしても以前の領域が残る場合があります。`clear()` も確保済み容量を減らしません。計算をまたいで前計算や作業配列を保持することはありません。

本体のコピー構築は $$O(N+M)$$ 時間・空間、`Result` のコピー構築は $$O(M)$$ 時間・空間です。コピー代入は、代入前の配列の要素数の合計を $$K$$ として、本体は $$O(K+N+M)$$ 時間、`Result` は $$O(K+M)$$ 時間です。再確保する場合の追加空間は、それぞれ $$O(N+M)$$、$$O(M)$$ です。移動構築は $$O(1)$$ 時間・追加空間、移動代入は古い配列の破棄も含め $$O(K)$$ 時間・$$O(1)$$ 追加空間です。

参考：[hitonanode/cplib-cpp](https://github.com/hitonanode/cplib-cpp/blob/master/flow/maxflow_lowerbound.hpp)、[ei1333/library](https://github.com/ei1333/library/blob/master/graph/flow/maxflow-lower-bound.hpp)。どちらも上下限付きの最大流を `MaxFlowLowerBound` と呼び、`add_edge(from,to,lower,upper)` で辺を追加します。
