---
title: "Small Vector"
documentation_of: //components/small_vector.hpp
---

```cpp
#include "lib/components/small_vector.hpp"
SmallVector<unsigned long long, 2> a;
a.push_back(123);
```

`SmallVector<T, N>` は先頭 $$N$$ 要素分を内部に持つ可変長配列です。`T` は trivially copyable かつ trivially destructible な型、$$N>0$$ が必要です。`size`・`capacity`・`empty`・`data`・`begin`・`end`・`[]`・`back`・`reserve`・`resize`・`clear`・`push_back`・`pop_back`・`assign`・`insert`・`erase` に対応します。範囲の `assign`・`insert` に自身の要素を渡さないでください。

`resize_for_overwrite(n)` は要素の初期化を省略してサイズを変更します。追加した要素は代入してから参照してください。

参照・末尾削除は $$O(1)$$、末尾追加は償却 $$O(1)$$、再確保・コピー・中間の挿入削除は $$O(n)$$、空間は $$O(\max(n,N))$$ です。
