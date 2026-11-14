---
title: Sliding Window Aggregation
documentation_of: //data_structure/sliding_window_aggregation.hpp
library_category: data_structure/range_query
---

末尾への追加・先頭の削除ができるキューです。キュー全体の値を、先頭から末尾の順に `op` でまとめた結果（集約値）を取得できます。文字列連結のように順番で結果が変わる演算にも対応します。

```cpp
#include "lib/data_structure/sliding_window_aggregation.hpp"
#include <string>

std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return ""; }

SlidingWindowAggregation<std::string, op, e> queue;
queue.push_back("ab");
queue.push_back("cd");
auto a = queue.prod(); // "abcd"
queue.pop_front();
auto b = queue.front(); // "cd"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SlidingWindowAggregation<S,op,e>()` | 空のキューを作成 | $$O(1)$$ |
| `SlidingWindowAggregation(values)` | `vector<S>` から順番に追加 | $$O(n)$$ |
| `push_back(value)` | 末尾に追加 | 償却 $$O(1)$$ |
| `pop_front()` | 先頭を削除 | 償却 $$O(1)$$、最悪 $$O(n)$$ |
| `front()` / `back()` | 先頭／末尾の値の `const S&` | $$O(1)$$ |
| `prod()` | 先頭から末尾まで集約。空なら `e()` | $$O(1)$$ |
| `apply(value, mapping)` | `mapping(prod(),value)` を計算した `T` を返す | $$O(1)$$ |
| `apply(value)` | `S` が関数として呼べるとき、`prod()(value)` を計算した `T` を返す | $$O(1)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `reserve(n)` | 領域を確保 | $$O(n)$$ |
| `clear()` | 全要素を削除 | $$O(n)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たします。これは要素の順番を保てば、まとめる括弧の位置を変えても結果が同じになる条件（結合法則）です。 `e()` は空区間の値で、`op(e(),x) == x` と `op(x,e()) == x` を満たします。この値を単位元と呼びます。`S` はコピー・移動できる型を使います。演算の引数は変更しないでください。`front`・`back`・`pop_front` はキューが空でないときに使います。取得操作は `const` オブジェクトから呼べます。

`apply(value, mapping)` は `mapping(op(f, g), x) == mapping(g, mapping(f, x))` と `mapping(e(), x) == x` を満たす更新の適用に使います。引数 `value` の型を `T` とし、`mapping` は `S` と `T` を受け取って `T` に代入できる値を返します。空のキューでは `value` をそのまま返します。`apply(value)` は `mapping(f, x) = f(x)` とした呼び出しで、同じ順序の条件を満たす関数合成に使えます。どちらも `const` オブジェクトから呼べます。

```cpp
#include "lib/data_structure/sliding_window_aggregation.hpp"
#include <utility>

using Function = std::pair<long long, long long>;
Function compose(Function f, Function g) {
	return {g.first * f.first, g.first * f.second + g.second};
}
Function identity() { return {1, 0}; }

SlidingWindowAggregation<Function, compose, identity> functions;
functions.push_back({2, 1});
functions.push_back({3, 4});
auto value = functions.apply(5LL, [](Function f, long long x) {
	return f.first * x + f.second;
}); // 37
```

`mapping` または関数としての `S` の呼び出しを定数時間とすると、`apply` は $$O(1)$$ 時間です。整数型の演算結果はその型の範囲に収まるようにしてください。

コピーは独立したキューを作り、移動元は再利用できる空のキューになります。参照は更新後に取得し直してください。`size_type` は `std::size_t` です。

「償却」は、1 回だけ時間のかかる操作があっても、一連の操作の合計時間から 1 回あたりの計算量を評価するという意味です。

要素のコピーと演算を定数時間としたとき、追加・削除は償却 $$O(1)$$、集約は $$O(1)$$ 時間です。確保した領域を $$C$$ として使用空間は $$O(C)$$、コピーは $$O(n)$$ 時間です。文字列結合のように演算自体に時間がかかる場合は、その費用が加わります。
