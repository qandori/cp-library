---
title: "D-ary Heap"
documentation_of: //data_structure/d_ary_heap.hpp
---

```cpp
#include "lib/data_structure/d_ary_heap.hpp"

DaryHeap<int> heap;
heap.push(3);
heap.push(1);
auto x = heap.pop_value();
```

`DaryHeap<T, Compare = std::less<T>, D = 2>` は最大ヒープです。最小ヒープは `Compare` に `std::greater<T>` を指定します。`D` は 2 以上です。`std::vector<T>` からも構築できます。

`top()` は先頭の要素、`pop()` は削除、`pop_value()` は取得と削除です。`replace_top(x)` は先頭を `x` に置き換え、元の値を返します。`push_pop(x)` は挿入後の先頭を取り出します。

`values()` は内部配列の const 参照です。`size()`・`empty()`・`reserve(n)`・`clear()` も使えます。

要素数を $$n$$、$$D$$ を定数として、配列からの構築・コピー・予約は $$O(n)$$、`push` は償却 $$O(\log n)$$、削除・置換は $$O(\log n)$$、`clear` は自明に破棄可能な型なら $$O(1)$$、それ以外は $$O(n)$$、その他は $$O(1)$$ です。空間は $$O(n)$$、比較・要素の移動・破棄は $$O(1)$$ とします。
