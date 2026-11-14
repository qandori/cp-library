---
title: 下限付き最小費用流（最小費用 b-flow）
documentation_of: //graph/min_cost_b_flow.hpp
library_category: graph/flow
---

各辺の流量に下限・上限を指定し、各頂点から出る量と入る量の差を満たす最小費用の流れを求めます。下限は負でも構いません。自己辺・多重辺・負の費用・負閉路にも対応します。全ての上限・下限は有限の整数です。

各頂点について、辺を通って出る流量の合計から、辺を通って入る流量の合計を引いた値を `b[v]` で指定します。`solve()` は次の条件を満たす流れを求めます。

`v から出る流量の合計 - v に入る流量の合計 = b[v]`

- `b[v] = 3`：入る量より出る量を 3 多くします。
- `b[v] = -3`：出る量より入る量を 3 多くします。
- `b[v] = 0`：入る量と出る量を等しくします。全頂点の初期設定はこれです。

例えば、頂点 0 から頂点 2 へ 3 流す場合は `set_supply(0, 3)`、`set_supply(2, -3)` と設定します。他の頂点の `b[v]` は 0 のままにします。各頂点にこの指定を設けた最小費用流を「最小費用 b-flow」と呼びます。`set_supply(v,b)` の `b` が、この `b[v]` です。

辺 `u -> v` に負の流量を流すことは、その絶対値だけ `v -> u` に流すことと同じです。辺の `cost` は 1 流すときの費用で、合計費用は各辺の `flow * cost` の和です。

```cpp
#include "lib/graph/min_cost_b_flow.hpp"
#include <cassert>

int main() {
	MinCostBFlow<> graph(3);
	graph.add_edge(0, 1, 1, 4, 2);
	graph.add_edge(1, 2, 1, 4, 3);
	graph.add_edge(0, 2, 0, 3, 8);
	graph.set_supply(0, 3);
	graph.set_supply(2, -3);
	auto result = graph.solve();
	assert(result && result->cost == 15 && result->value == 0);
	assert(result->flow[0] == 3 && result->flow[1] == 3);
	assert(result->flow[2] == 0);

	auto fixed = graph.flow(0, 2, 2);
	assert(fixed && fixed->value == 2 && fixed->cost == 28);
	auto cheapest = graph.flow(0, 2, -2, 4);
	assert(cheapest && cheapest->value == -2 && cheapest->cost == 5);
	auto maximum = graph.max_flow(0, 2, 0, 4);
	assert(maximum && maximum->value == 4 && maximum->cost == 44);
	auto interval = graph.flow_range(0, 2, -3, 5);
	assert(interval && interval->first == -2 && interval->second == 4);
}
```

`flow(s,t,F)` などの `F` は、設定済みの `b[v]` に**追加する**流量です。始点は `b[s] + F`、終点は `b[t] - F`、他の頂点は `b[v]` を満たします。上の例では `b[v]` の指定だけで 0 から 2 へ 3 流すため、`flow(0,2,2)` は合計 5 を流します。負の `F` も指定できます。

全ての計算は `const` で、呼び出すたびに設定した辺と `b[v]` から計算します。前回の流量への追加ではありません。返した結果は配列を自分で持ち、以降の計算・辺や `b[v]` の変更によって書き換わりません。条件を満たす流れがなければ `std::nullopt` を返します。全頂点の `b[v]` の総和が 0 でない場合も解はありません。

表の $$N$$ は頂点数、$$M$$ は辺数、$$U\geq2$$ は内部で使う容量・要求流量の最大値の上界です。固定幅整数の演算を定数時間として、最悪時間を書きます。空間は、返り値を含む追加空間です。辺を追加した順に、0 始まりの辺番号が付きます。

表の計算量は $$N\geq2$$ の場合です。0 頂点の `solve()` は定数時間、1 頂点では自己辺しかないため $$O(M)$$ 時間で計算できます。始点・終点を指定する操作には 2 頂点以上が必要です。

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MinCostBFlow<Cap,Cost,Total>(n = 0)` | `n` 頂点で構築。既定は `std::int64_t`・`std::int64_t`・`__int128_t` | $$O(N)$$ | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／辺数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| `reserve(m)` | `m` 辺分の領域を確保 | 再確保時 $$O(M)$$、確保済みなら $$O(1)$$ | 再確保時 $$O(m)$$、確保済みなら $$O(1)$$ |
| `add_edge(from,to,lower,upper,cost)` | 有向辺を追加し、辺番号を `int` で返す | 償却 $$O(1)$$ | 通常 $$O(1)$$、再確保時 $$O(M)$$ |
| `get_edge(i)` | 辺番号 `i` の設定を `Edge` で返す | $$O(1)$$ | $$O(1)$$ |
| `edges()` | 全辺の設定を追加順の `vector<Edge>` で返す | $$O(M)$$ | $$O(M)$$ |
| `change_edge(i,lower,upper,cost)` | 辺番号 `i` の下限・上限・単位費用を変更。両端は変えない | $$O(1)$$ | $$O(1)$$ |
| `set_supply(v,b)` / `add_supply(v,delta)` | 「出る量 − 入る量」の指定値 `b[v]` を代入／加算 | $$O(1)$$ | $$O(1)$$ |
| `supply(v)` | 指定値 `b[v]` を `Cap` で返す | $$O(1)$$ | $$O(1)$$ |
| `supplies()` | 頂点ごとの指定値の配列を `const vector<Cap>&` で返す | $$O(1)$$ | $$O(1)$$ |
| `reset_supply()` | 全頂点の `b[v]` を 0 にする | $$O(N)$$ | $$O(1)$$ |
| `clear()` | 辺を全て消し、全頂点の `b[v]` を 0 にする。頂点数は変えない | $$O(N+M)$$ | $$O(1)$$ |
| `solve()` | 設定した辺と `b[v]` に対する最小費用解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `flow(s,t,F)` | 追加流量をちょうど `F` とした最小費用解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `flow(s,t,L,R)` | 追加流量を閉区間 `[L,R]` から選び、費用が最小の解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `min_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で可能な最小の追加流量を選び、その中で費用を最小にした解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `max_flow(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で可能な最大の追加流量を選び、その中で費用を最小にした解を `optional<Result>` で返す | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `flow_range(s,t,L = 0,R = Capの最大値)` | `[L,R]` 内で条件を満たす追加流量の最小値・最大値を `optional<pair<Cap,Cap>>` で返す。費用は考慮しない | $$O((N+M)^2\log N\log U)$$ | $$O(N+M)$$ |
| `Edge` | `int from,to`、`Cap lower,upper`、`Cost cost` を持つ | —（型） | —（型） |
| `Result` | 費用合計 `cost`、各辺の流量 `flow`、頂点の補正値 `potential`、追加流量 `value` を持つ | —（型） | —（型） |
| `capacity_type` / `cost_type` / `total_type` | それぞれ `Cap`・`Cost`・`Total` の別名 | —（型） | —（型） |

辺追加の「償却」は、辺を連続して追加したときの 1 回あたりの平均です。`supplies()` は指定値を並べた元の配列への参照であり、その後の `b[v]` の変更も反映します。`edges()` はコピーを返します。`clear()` を呼ぶと、それまでの辺番号は使えなくなります。

`Result::cost` の型は `Total`、`flow` は長さ $$M$$ の `vector<Cap>`、`potential` は長さ $$N$$ の `vector<Cost>`、`value` は `Cap` です。`flow[i]` は元の辺 `i` の向きに対する符号付き流量で、必ず `lower <= flow[i] <= upper` を満たします。`solve()` の `value` は 0 です。`flow_range` が `[a,b]` を返した場合、その間の全ての整数の追加流量が可能です。費用が同じ解が複数ある場合、どれを返すかは未規定です。既定の `Total = __int128_t` は [FastOutput](fast_io.md) で出力できます。

辺 `u -> v` に流量 `f` があるとき、元の向きには `upper - f` だけ追加でき、逆向きには `f - lower` だけ取り消せます。これらの追加・取り消しが可能な辺を集めたものを残余グラフと呼びます。逆向きの辺の単位費用は元の費用の符号を反転した値です。

`potential` は、各頂点に付ける費用の補正値です。辺 `u -> v` の費用 `c` を `c + potential[u] - potential[v]` に置き換えると、残余容量が正の全ての辺で非負になります。この条件で最小費用であることを確認できます。元の辺については、`f < upper` なら補正後の費用が 0 以上、`lower < f` なら 0 以下です。従って下限と上限の間にある辺では 0 になります。これらの補正値は、追加流量を固定した場合の元の辺に対して有効です。

頂点と辺の番号は 0 始まりです。頂点数は非負、`s`・`t` は相異なる有効な頂点にします。辺・流量区間には `lower <= upper`、`L <= R` が必要です。0 頂点 0 辺の `solve()` は費用 0、空の配列を持つ解を返します。自己辺の流量は出る量と入る量に同じだけ加わるため、その差に影響しませんが、費用には含まれます。

`Cap` と `Cost` は 64 bit 以下の標準の符号付き整数型、`Total` は符号付き整数型です。`Cost` の最小値は単位費用に使えません。`add_supply` の加算結果は `Cap` に収めます。各辺の `upper - lower` は `Cap` の最大値以下にしてください。

指定した追加流量の区間は、まず始点・終点に接続する辺の上下限から得られる範囲と交差させます。空になれば解はありません。この絞り込み後の区間幅も `Cap` の最大値以下にします。始点・終点の条件から不可能と分かる流量は、この段階で除きます。

計算では、費用が非負の辺は下限、負の辺は上限を初期流量に選び、各頂点の指定値から「初期流量で出る量 − 初期流量で入る量」を差し引きます。この残りの指定値について、各頂点の絶対値と正の値の総和が `Cap` に収まる必要があります。指定値の集計には 128 bit 整数を使うため、集計途中だけ `Cap` を超えることは構いません。流量区間を指定する場合は、区間を下限・上限とする `t -> s` の追加辺も含めて同じ条件が必要です。最大・最小流量を調べる処理では、元の辺の費用を 0、追加辺の費用を ±1 にした場合にもこの条件が必要です。

最短距離・補正値・補正後の費用と、それらの計算途中は `Cost` に、費用の積・合計とその計算途中は `Total` に収めてください。有限の最短距離は `Cost` の最大値未満にします。この値は到達できないことを表すために使います。補助頂点の分も含め、$$N\leq\mathrm{INT\_MAX}-2$$ が必要です。補助辺の予約分も含め、`solve()` は $$M+N$$、始点・終点を指定する操作は $$M+N+1$$ を `INT_MAX/2` 以下にします。

`min_flow`・`max_flow` は流量の決定と費用の最小化、`flow_range` は両端の決定のためにそれぞれ 2 回計算しますが、定数回なので表の計算量は変わりません。作業領域と返す解はともに $$O(N+M)$$、`flow_range` の返り値自体は $$O(1)$$ 空間です。

入力の保持領域は、頂点ごとの指定値の配列・辺配列の確保済み容量をそれぞれ $$S$$・$$C$$ として $$O(S+C)$$ です。通常の構築・辺追加では $$S=N$$ ですが、コピー代入で頂点数を減らしても、以前の領域が残る場合があります。`clear()` も確保済み容量を減らしません。

本体と `Result` のコピー構築は $$O(N+M)$$ 時間・空間です。コピー代入は、代入前の配列の要素数の合計を $$K$$ として $$O(K+N+M)$$ 時間、再確保時は $$O(N+M)$$ の追加空間を使います。移動構築は $$O(1)$$ 時間・追加空間、移動代入は古い配列の破棄も含め $$O(K)$$ 時間・$$O(1)$$ 追加空間です。計算をまたいで前計算や作業配列を保持することはありません。

参考：[Library Checker の Min Cost b-Flow](https://judge.yosupo.jp/problem/min_cost_b_flow)、[hitonanode/cplib-cpp](https://github.com/hitonanode/cplib-cpp/blob/master/flow/b-flow.hpp)、[sotanishy/cp-library-cpp](https://github.com/sotanishy/cp-library-cpp/blob/main/docs/flow/min_cost_b_flow.md)。
