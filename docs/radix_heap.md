---
title: "Radix Heap"
documentation_of: //data_structure/RadixHeap.hpp
---

```cpp
#include "lib/data_structure/RadixHeap.hpp"

RadixHeap<std::uint64_t, int> heap;
heap.push({10, 3});
auto [key, value] = heap.pop_value();
```

`Key` は 64 bit 以下の整数型です。`push({key, value})` のキーは `last()` 以上にします。初期下限は `RadixHeap(minimum)` で指定でき、既定値は 0 です。`top()` も下限を更新します。

`top()` は最小要素の参照、`pop()` は最小要素の削除、`pop_value()` は取得と削除です。`size()`・`empty()`・`clear(minimum = 0)` も使えます。

キーのビット数を $$w$$、要素数を $$n$$ として、`push` は償却 $$O(1)$$、取り出しは償却 $$O(w)$$、空間は $$O(n+w)$$ です。
