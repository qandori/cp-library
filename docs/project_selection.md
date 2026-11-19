---
title: Project Selection（燃やす埋める）
documentation_of: //flow/project_selection.hpp
library_category: flow
redirect_from:
- /graph/project_selection.hpp.html
- /lightweight/graph/project_selection.hpp.html
- /graph/flow/project_selection.hpp.html
- /lightweight/graph/flow/project_selection.hpp.html
---

各頂点を選ぶ・選ばないの 2 通りに分け、費用の総和を最小化します。選択を `true`（1、燃やす）、非選択を `false`（0、埋める）とします。利益は負の費用として指定します。例えば、選ぶと費用 4 なら `add_cost(v,4)`、選ぶと利益 10 なら `add_cost(v,-10)` です。

```cpp
#include "lib/flow/project_selection.hpp"
#include <cassert>

int main() {
	ProjectSelection<> problem(2);
	problem.add_cost(0, -10);             // 0 を選ぶと利益 10
	problem.add_cost(1, 4);               // 1 を選ぶと費用 4
	problem.forbid(0, 1, true, false);    // 0 を選ぶなら 1 も選ぶ
	auto result = problem.solve();
	assert(result && result->cost == -6);
	assert(-result->cost == 6);           // 最大純利益
	assert(result->selected[0] && result->selected[1]);
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `ProjectSelection<Cost = std::int64_t>(n = 0)` | `n` 頂点の問題を作る。頂点番号は `[0,n)` | $$O(n)$$ |
| `ProjectSelection(n,flipped)` | 長さ `n` の `vector<bool>` で、内部の選択を反転する頂点を指定する | $$O(n)$$ |
| `size()` | 指定した頂点数を `int` で返す。内部の補助頂点は含まない | $$O(1)$$ |
| `is_flipped(v)` | 頂点 `v` の反転指定を `bool` で返す | $$O(1)$$ |
| `add_constant(cost)` | 選択によらない費用を加える | $$O(1)$$ |
| `add_cost(v,cost)` | `v` を選ぶ場合に費用を加える | $$O(1)$$ |
| `add_cost(v,c0,c1)` | `v` を選ばない場合に `c0`、選ぶ場合に `c1` を加える | $$O(1)$$ |
| `add_cost_if(v,value,cost)` | `v` の選択が `value` の場合だけ費用を加える | $$O(1)$$ |
| `add_cost_if(u,v,a,b,cost)` | 選択 `(u,v)` が `(a,b)` の場合だけ費用を加える | 償却 $$O(1)$$ |
| `add_equal_cost(u,v,equal,cost)` | `equal == true` なら選択が同じ場合、`false` なら異なる場合に費用を加える | 償却 $$O(1)$$ |
| `add_pair_cost(u,v,c00,c01,c10,c11)` | 選択 `(u,v)` が `00,01,10,11` の各場合の費用を加える | 償却 $$O(1)$$ |
| `add_all_cost(vertices,value,cost)` | 指定した全頂点の選択が `value` と一致する場合だけ費用を加える | 償却 $$O(k)$$ |
| `add_any_cost(vertices,value,cost)` | 指定した頂点のうち少なくとも 1 つの選択が `value` と一致する場合に、費用を 1 回だけ加える | 償却 $$O(k)$$ |
| `force(v,value)` | `v` の選択を `value` に固定する | 償却 $$O(1)$$ |
| `forbid(u,v,a,b)` | 選択 `(u,v)` が `(a,b)` となることを禁止する | 償却 $$O(1)$$ |
| `force_equal(u,v,equal = true)` | `equal == true` なら選択を一致させ、`false` なら異ならせる | 償却 $$O(1)$$ |
| `solve()` | 最小費用と選択を `std::optional<Result>` で返す。実行不能なら `std::nullopt` | 最大流の計算量 |

頂点番号 `u,v` は `int`、状態 `value,a,b,equal` は `bool`、頂点列 `vertices` は `const std::vector<int>&` です。費用を追加する関数と制約を追加する関数の返り値は `void` です。表の `k` は重複を含めた頂点列の長さです。

`Cost` は符号付き 64 bit 以下の標準整数型で、`cost_type` から参照できます。金額の引数、集約中の費用、`Result::cost` は全て `total_type`（`__int128_t`）です。64 bit 整数の最小値などを符号反転する場合は、`-ProjectSelection<>::total_type(value)` のように先に型を広げます。呼び出した費用は全て加算され、既存の費用を上書きしません。

`Result::cost` は最小費用で、最大純利益は `-result.cost` です。`Result::selected` は長さ `n` の `std::vector<bool>` です。128 bit 整数の出力には [FastOutput](https://qandori.github.io/cp-library/components/fast_io.hpp.html) が使えます。

`solve()` は問題を変更しません。繰り返し呼べ、求解後に費用や制約を追加した場合は、次の `solve()` で再構築して解きます。コピーした問題は独立に変更できます。最適解が複数ある場合はそのうち 1 つを返します。頂点数 0 で何も追加しなければ、費用 0 と空の選択列を返します。

反転は構築時に指定し、問題の途中では変更しません。`flipped[v] == true` の頂点では、内部の変数を元の選択と逆向きに扱います。省略時は全て `false` です。費用・制約の引数と `Result::selected` は、全て元の選択の意味です。例えば `force(v,true)` は、反転指定によらず元の頂点を選ぶという制約です。

2 頂点にまたがる費用には、次の不等式が必要です。表は元の選択に対する費用で表した条件です。

| `u`・`v` の反転指定 | 2 頂点の費用の条件 |
| --- | --- |
| 同じ | $$c_{00}+c_{11}\leq c_{01}+c_{10}$$ |
| 異なる | $$c_{00}+c_{11}\geq c_{01}+c_{10}$$ |

`add_cost_if` の 2 頂点版にも同じ条件が適用されます。反転後の指定状態は、それぞれ `a != flipped[u]`・`b != flipped[v]` です。`cost > 0` なら反転後の状態が異なる場合、`cost < 0` なら同じ場合に使えます。`cost == 0` なら反転の制約はありません。

`add_equal_cost` では、正の費用を加える条件が「同じ」なら反転指定を異ならせ、「異なる」なら反転指定を同じにします。負の費用ではこの対応が逆になります。費用 0 なら反転の制約はありません。

例えば、次の例では `0,2` と `1,3` を別々の反転指定にしています。

```cpp
#include "lib/flow/project_selection.hpp"
#include <cassert>

int main() {
	ProjectSelection<> problem(4, {false, true, false, true});
	problem.add_cost_if(0, true, 3);           // 0 を燃やすと罰金 3
	problem.add_cost_if(1, false, 5);          // 1 を埋めると罰金 5
	problem.add_cost_if(0, 1, true, true, 7);   // 両方を燃やすと罰金 7
	problem.add_cost_if(2, 3, false, false, 4); // 両方を埋めると罰金 4
	problem.add_all_cost({0, 2}, true, -11);   // 両方を燃やすと利益 11
	problem.add_all_cost({1, 3}, false, -8);   // 両方を埋めると利益 8
	problem.forbid(0, 1, true, true);          // 同時には選べない
	problem.forbid(2, 3, false, false);        // 少なくとも一方を選ぶ
	auto answer = problem.solve();
	assert(answer);
}
```

燃やす・埋めるの代表的な条件は、以下の操作で表せます。金額 `P` は正とします。

| 条件 | 操作 | 反転指定の条件 |
| --- | --- | --- |
| `u` を燃やすと罰金 | `add_cost_if(u,true,P)` | 任意 |
| `u` を埋めると罰金 | `add_cost_if(u,false,P)` | 任意 |
| `u` を燃やすと利益 | `add_cost_if(u,true,-P)` | 任意 |
| `u` を埋めると利益 | `add_cost_if(u,false,-P)` | 任意 |
| `u` を燃やし `v` を埋めると罰金 | `add_cost_if(u,v,true,false,P)` | `u`・`v` で同じ |
| 両方を燃やすと罰金 | `add_cost_if(u,v,true,true,P)` | `u`・`v` で異なる |
| 両方を埋めると罰金 | `add_cost_if(u,v,false,false,P)` | `u`・`v` で異なる |
| 選択が同じなら罰金 | `add_equal_cost(u,v,true,P)` | `u`・`v` で異なる |
| 選択が異なるなら罰金 | `add_equal_cost(u,v,false,P)` | `u`・`v` で同じ |
| 選択が同じなら利益 | `add_equal_cost(u,v,true,-P)` | `u`・`v` で同じ |
| 選択が異なるなら利益 | `add_equal_cost(u,v,false,-P)` | `u`・`v` で異なる |
| 全員を燃やす／埋めると利益 | `add_all_cost(vertices,value,-P)` | 列内の反転指定が全て同じ |
| 一人でも燃やす／埋めると罰金 | `add_any_cost(vertices,value,P)` | 列内の反転指定が全て同じ |
| `u` を選ぶなら `v` も選ぶ | `forbid(u,v,true,false)` | `u`・`v` で同じ |
| 両方を選ぶことを禁止 | `forbid(u,v,true,true)` | `u`・`v` で異なる |
| 少なくとも片方を選ぶ | `forbid(u,v,false,false)` | `u`・`v` で異なる |

同じ問題に追加する全ての費用・制約が、同一の反転指定で条件を満たす必要があります。任意の 2 頂点の費用や、二部に分けられないグラフ上の「同じ選択なら罰金」を扱えるわけではありません。

`add_all_cost` は、全員を選ぶ場合と全員を選ばない場合の両方に使えます。空の頂点列では常に `cost` を加えます。`add_any_cost` は、誰も指定状態でなければ費用を加えず、1 人以上なら人数によらず 1 回だけ費用を加えます。空の頂点列では常に費用 0 です。

列に 2 項目以上あり、`cost != 0` の場合は、列内の反転指定を全て同じにします。この場合、`add_all_cost` は `cost < 0`、`add_any_cost` は `cost > 0` にしてください。空または 1 項目の列では、反転指定・費用の符号に制限はありません。重複頂点を渡しても条件の意味は変わりませんが、処理量と列の長さの条件には重複も数えます。補助頂点は最大 1 個、辺は最大 `k` 本追加します。

```cpp
#include "lib/flow/project_selection.hpp"
#include <cassert>

int main() {
	ProjectSelection<> model(3);
	model.add_all_cost({0, 1, 2}, true, -20); // 全員を選ぶと利益 20
	model.add_all_cost({0, 1}, false, -7);    // 両方を選ばないと利益 7
	model.add_cost_if(0, 2, true, false, 5);  // 0 だけを選ぶと費用 5
	model.force(1, false);
	auto answer = model.solve();
	assert(answer && answer->cost == -7);
}
```

`forbid(u,v,a,b)` は、別々の頂点では反転後の `a` と `b` が異なる場合に使えます。別々の頂点に対する `force_equal` は、`equal == true` なら両頂点の反転指定を同じに、`false` なら異なるようにします。対応しない費用・制約の追加は入力条件違反として `assert` で検査し、実行不能を返す `std::nullopt` とは区別します。

`u == v` の `add_pair_cost` は `c00` と `c11` だけを使い、2 頂点の費用の不等式は不要です。`add_cost_if` の 2 頂点版で同じ頂点へ異なる状態を指定した場合は、常に寄与 0 です。`add_equal_cost(v,v,true,cost)` は定数 `cost`、`add_equal_cost(v,v,false,cost)` は常に 0 です。`forbid(v,v,a,b)` も `a != b` なら制約なし、`a == b` ならその状態を禁止します。`force_equal(v,v,true)` は制約なし、`force_equal(v,v,false)` は実行不能です。これらの場合、反転指定の条件は不要です。

依存関係は閉路を持っていても構いません。必須・禁止などの矛盾は、制約辺だけで始点から終点へ到達できるかを調べて判定します。「無限大」の値は利用者が指定せず、有限容量の総和より 1 大きい値を内部で設定します。

1 頂点だけに依存する費用は差分へ集約します。その差分の絶対値と、変換後の有限容量辺の容量の総和を $$B$$ として、$$B<2^{64}-1$$ が必要です。2 頂点の費用から作る容量にも符号なし 64 bit を使います。費用の集約・符号反転・定数項・最終費用の計算途中は符号付き 128 bit に収めてください。補助頂点を含む変数数は `INT_MAX/2-3` 以下、始点・終点を含む全辺数は `INT_MAX/2` 以下とします。これらの条件は入力の前提で、実行不能を表す `std::nullopt` とは区別します。

元の頂点数を $$n$$、補助頂点・始点・終点を含む頂点数を $$V$$、1 頂点の費用の集約後に作る辺数を $$E$$ とします。構築は $$O(n)$$ 時間・空間、費用と制約の保持は $$O(V+E)$$ 空間です。`solve()` はグラフの構築・制約の矛盾判定・解の復元に $$O(V+E)$$ 時間、最大流に一般の上界で $$O(V^2E)$$ 時間、追加空間は $$O(V+E)$$ です。
