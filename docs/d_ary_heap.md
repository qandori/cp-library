---
title: "D-ary Heap"
documentation_of: //components/d_ary_heap.hpp
redirect_from: /data_structure/d_ary_heap.hpp.html
library_category: components/data_structure
---

値の追加と最大値の取り出しを行う優先度付きキューです。`D` は木の各節点が持つ子の最大数を表します。

```cpp
#include "lib/components/d_ary_heap.hpp"

DaryHeap<int> heap;
heap.push(3);
heap.push(1);
auto x = heap.pop_value();
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DaryHeap()` / `DaryHeap(compare)` / `DaryHeap(a,compare = {})` | 空／比較関数指定／配列から構築 | 空: $$O(1)$$、配列: $$O(n)$$ |
| `push(x)` | 要素を追加 | 償却 $$O(\log n)$$ |
| `top()` | 先頭要素の const 参照。非空であること | $$O(1)$$ |
| `pop()` / `pop_value()` | 先頭を削除／取得して削除。非空であること | $$O(\log n)$$ |
| `replace_top(x)` | 先頭を置換し、元の値を返す。非空であること | $$O(\log n)$$ |
| `push_pop(x)` | `x` を追加した場合の先頭を返し、残りを保持 | $$O(\log n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `values()` | 内部配列の const 参照 | $$O(1)$$ |
| `reserve(n)` / `clear()` | 領域予約／容量を残して空にする | reserve: $$O(n)$$、clear: 要素の破棄に処理が不要なら $$O(1)$$、その他 $$O(n)$$ |
| `swap(other)` | 内容と比較関数を交換 | $$O(1)$$ |

`DaryHeap<T, Compare = std::less<T>, D = 2>` は最大ヒープです。最小ヒープは `Compare` に `std::greater<T>` を指定します。`D` は 2 以上です。`std::vector<T>` からも構築できます。

`top()` は先頭の要素、`pop()` は削除、`pop_value()` は取得と削除です。`replace_top(x)` は先頭を `x` に置き換え、元の値を返します。`push_pop(x)` は挿入後の先頭を取り出します。

`values()` は内部配列の const 参照です。`size()`・`empty()`・`reserve(n)`・`clear()` も使えます。

要素数を $$n$$、$$D$$ を定数として、配列からの構築・コピー・予約は $$O(n)$$、`push` は償却 $$O(\log n)$$、削除・置換は $$O(\log n)$$、`clear` は `int` など要素の破棄に処理を必要としない型なら $$O(1)$$、それ以外は $$O(n)$$、その他は $$O(1)$$ です。比較・要素の移動・破棄は $$O(1)$$ とします。内部配列の確保容量を $$C$$ とすると保持空間は $$O(C)$$ です。削除や `clear()` では容量を解放せず、`reserve(n)` で要素数より大きい容量を確保することもできます。

償却計算量は、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。
