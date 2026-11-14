---
title: "条件を満たす添字の二分探索"
documentation_of: //components/binary_search.hpp
library_category: components/util
---

```cpp
#include "lib/components/binary_search.hpp"
#include <vector>

std::vector<int> a{1, 3, 3, 7};
int lower = binary_search_detail::partition_point(4, [&](int i) {
	return a[i] < 3;
}); // 1
int upper = binary_search_detail::partition_point(4, [&](int i) {
	return a[i] <= 3;
}); // 3
```

添字 `i` を受け取って条件を満たすかを `bool` で返す関数 `predicate(i)` を指定します。添字を増やすと結果が `true` から `false` へ変わり、一度 `false` になった後は `true` に戻らない場合に使えます。全て `true`・全て `false` の場合も扱えます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `binary_search_detail::partition_point(n,predicate)` | `0 <= i < n` で最初に `predicate(i)` が false になる添字。全て true なら `n` | $$O(\log n)$$ |

`n >= 0` とし、判定関数は同じ添字に対して常に同じ結果を返すものを使います。引数として渡す添字は必ず `[0,n)` 内です。`n == 0` の場合は判定関数を呼ばずに `0` を返します。

判定関数の計算を $$O(1)$$ として、時間は $$O(\log n)$$、追加空間は $$O(1)$$ です。空の範囲は $$O(1)$$ 時間です。
