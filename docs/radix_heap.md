---
title: "Radix Heap"
documentation_of: //components/RadixHeap.hpp
redirect_from: /data_structure/RadixHeap.hpp.html
library_category: components/data_structure
---

```cpp
#include "lib/components/RadixHeap.hpp"

RadixHeap<std::uint64_t, int> heap;
heap.push({10, 3});
auto [key, value] = heap.pop_value();
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RadixHeap(minimum = 0)` | 初期下限を指定して空ヒープを構築 | $$O(w)$$ |
| `push({key,value})` | `key >= last()` を満たす要素を追加 | 償却 $$O(1)$$ |
| `top()` | 最小要素の const 参照。非空であること。下限も更新 | 償却 $$O(w)$$ |
| `pop()` / `pop_value()` | 最小要素を削除／取得して削除。非空であること | 償却 $$O(w)$$ |
| `last()` | 現在のキーの下限 | $$O(1)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `clear(minimum = 0)` | 空に戻し、キーの下限を設定 | $$O(n+w)$$ |

`Key` は 64 bit 以下の整数型です。`push({key, value})` のキーは `last()` 以上にします。初期下限は `RadixHeap(minimum)` で指定でき、既定値は 0 です。`top()` も下限を更新します。

`top()` は最小要素の参照、`pop()` は最小要素の削除、`pop_value()` は取得と削除です。`size()`・`empty()`・`clear(minimum = 0)` も使えます。

キーのビット数を $$w$$、現在の要素数を $$n$$ として、`push` は償却 $$O(1)$$、取り出しは償却 $$O(w)$$ 時間です。キーの比較・値の移動と破棄を定数時間とします。

各格納配列の確保容量の合計を $$C$$ とすると、保持空間は $$O(C+w)$$ です。削除や `clear` の後も確保済みの容量を保持するため、現在の要素数が 0 になっても領域は解放しません。同時に保持した要素数の最大を $$N$$ とすると、確保容量の上界は $$O(wN)$$ です。

償却計算量は、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。
