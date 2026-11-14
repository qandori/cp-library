---
title: 両端優先度付きキュー
documentation_of: //data_structure/double_ended_priority_queue.hpp
library_category: data_structure
---

最小値・最大値を取得し、どちら側からも削除できるキューです。挿入する値を事前に集める必要はありません。重複した値は別々の要素として保持します。

```cpp
#include "lib/data_structure/double_ended_priority_queue.hpp"

DoubleEndedPriorityQueue<int> queue(std::vector<int>{3, 1, 5, 5});
queue.push(2);
auto a = queue.min();     // 1
auto b = queue.max();     // 5
auto c = queue.pop_min(); // 1 を返して削除
auto d = queue.pop_max(); // 5 を 1 個返して削除
auto n = queue.size();    // 3
```

`DoubleEndedPriorityQueue<T,Compare>` の `Compare` の既定値は `std::less<T>` です。`Compare(a,b)` は `a` が `b` より前に来るかを `bool` で返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。`std::greater<T>` を指定すると、`min`・`pop_min` は数値の大きい側、`max`・`pop_max` は小さい側を扱います。比較上同値な要素どうしの取り出し順は未規定です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DoubleEndedPriorityQueue(compare = Compare{})` | 空のキューを構築 | $$O(1)$$ |
| `DoubleEndedPriorityQueue(values,compare = Compare{})` | `std::vector<T>` の全要素から構築。配列は値渡し | $$O(n)$$ |
| `push(value)` | `T` を値渡しで 1 個追加 | 償却 $$O(\log n)$$ |
| `min()` / `max()` | 比較順で先頭／末尾の値を `const T&` で取得。空でないことが必要 | $$O(1)$$ |
| `pop_min()` / `pop_max()` | 比較順で先頭／末尾の値を 1 個削除し、`T` で返す。空でないことが必要 | $$O(\log n)$$ |
| `size()` / `empty()` | 要素数を `std::size_t`／空判定を `bool` で返す | $$O(1)$$ |
| `reserve(n)` | 要素数 `n` の領域を予約。内容は維持 | 再確保時 $$O(n)$$ |
| `clear()` | 容量と比較関数を保って全要素を削除 | $$O(n)$$ |
| コピー／コピー代入 | 独立したキューとして複製 | $$O(n)$$ |
| 移動構築／`swap(other)` | 内容を移動／内容と比較関数を交換 | $$O(1)$$ |

`value_type` は `T`、`size_type` は `std::size_t`、`value_compare` は `Compare` です。`T` には移動構築・移動代入・交換と、`Compare` による比較が必要です。コピー操作と、左辺値を `push` する場合には `T` のコピー構築も必要です。コピーできない値を使う場合、構築用の配列と追加する値を `std::move` で渡してください。`Compare` にはコピー構築・交換が必要です。

`min()`・`max()` が返す参照はキューの変更で無効になることがあります。移動元は空になり、比較関数を保持して再利用できます。移動代入では移動先の旧要素の破棄時間も加わります。コピー代入の時間には移動先の旧要素数も加わります。要素数が 1 個以下なら、取得・追加・削除は定数時間です。

要素数を $$n$$、確保した容量を $$C$$ として、使用空間は $$O(C)$$、構築時の追加空間は受け取った配列を除いて $$O(1)$$ です。値の移動・コピー・比較・破棄と比較関数の操作を定数時間とした計算量です。順位・任意の値の削除・上位や下位の和が必要な場合は [Top K Sum](top_k_sum.md) を使えます。重複が多い場合も、同じ値を個数でまとめて保持する Top K Sum が適しています。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
