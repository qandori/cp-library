---
title: "2-SAT"
documentation_of: //graph/two_sat.hpp
library_category: graph
---

`true`・`false` を取る変数について、`(x[i] == f) || (x[j] == g)` の形の条件を全て満たす値の組を求めます。この形の条件 1 つを節と呼びます。

```cpp
#include "lib/graph/two_sat.hpp"
#include <cassert>

TwoSAT sat(3);
sat.add_clause(0, true, 1, false); // x[0] または !x[1]
sat.add_clause(1, true, 2, true);
assert(sat.satisfiable());
auto answer = sat.answer(); // std::vector<bool>
assert(answer[0] || !answer[1]);
assert(answer[1] || answer[2]);

sat.add_clause(0, false, 0, false);
assert(sat.satisfiable());
assert(!sat.answer()[0]);
```

`TwoSAT(n = 0)` で 0 始まりの `n` 個の bool 変数を作ります。

| 操作 | 内容・返り値 |
| --- | --- |
| `size()` | 変数の個数を `int` で返す |
| `reserve(m)` | `m` 節を追加するための内部領域を予約する。返り値は `void` |
| `add_clause(i,f,j,g)` | `(x[i] == f) || (x[j] == g)` を追加する。返り値は `void` |
| `satisfiable()` | 全ての節を同時に満たせるなら `true`、そうでなければ `false` を `bool` で返す |
| `answer()` | 直前に求めた、全ての節を満たす変数の値を `std::vector<bool>` で返す |

`f`・`g` は bool 値です。`answer()` は、直前の `satisfiable()` が `true` で、以後に節を追加していない場合だけ呼べます。返す配列の長さは `size()`、各要素は対応する変数の値です。代入例は一意ではありません。

同じ変数を両辺に指定できます。`add_clause(i,f,i,f)` は `x[i] = f` への固定、`add_clause(i,true,i,false)` は値によらず常に満たす条件です。重複する節も指定できます。

変数数は `0 <= n <= INT_MAX / 2`、`i`・`j` は `[0,n)`、内部に格納する辺（1 節につき最大 2 本）は `INT_MAX` 以下である必要があります。予約する節数 `m` は `std::size_t` で、`INT_MAX / 2` 以下です。これらの範囲は `assert` の前提です。`TwoSAT(0)` は充足可能で、代入例は空配列です。節がなければ、全変数が `false` の代入例を返します。

充足可能・充足不可能のどちらの判定結果も保存します。節を追加せずに `satisfiable()` を再度呼ぶと、保存した結果を返します。`add_clause` は常に真になる条件の場合も保存結果を無効にするため、追加後は再び `satisfiable()` を呼んでください。`reserve` は保存結果を無効にしません。コピーは節・保存結果とも独立します。

参考: [Nyaan's Library](https://github.com/NyaanNyaan/library/blob/master/math/two-sat.hpp)、[maspypy library](https://github.com/maspypy/library/blob/main/graph/twosat.hpp)。

変数数を $$n$$、節数を $$m$$ として、構築は $$O(n)$$、`size()` は $$O(1)$$、節追加は償却 $$O(1)$$、初回または節追加後の判定は $$O(n+m)$$、変更のない問題の再判定は $$O(1)$$、代入例のコピーは $$O(n)$$、保持・作業空間は $$O(n+m)$$ です。`reserve(r)` は再確保がある場合に $$O(m)$$ 時間、ない場合に $$O(1)$$ 時間で、予約した容量 $$O(r)$$ も保持空間に含めます。空の問題は $$O(1)$$ 時間・空間です。
