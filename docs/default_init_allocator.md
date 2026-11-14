---
title: "Default Init Allocator"
documentation_of: //components/default_init_allocator.hpp
redirect_from: /data_structure/default_init_allocator.hpp.html
library_category: components/util
---

`std::vector` などのメモリ確保と要素の構築を担当する部品（アロケータ）です。整数型では、領域を確保した後の 0 埋めを省きます。

```cpp
#include "lib/components/default_init_allocator.hpp"
#include <vector>

std::vector<int, DefaultInitAllocator<int>> a(100);
for (int i = 0; i < 100; ++i) a[i] = i;
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DefaultInitAllocator<T>()` | `std::vector` などに渡すアロケータを構築 | $$O(1)$$ |
| `construct(p)` | `p` の位置に要素をデフォルト初期化。通常はコンテナから呼ばれる | T のデフォルト構築 1 回 |
| `allocate(n)` / `deallocate(p,n)` | `std::allocator<T>` と同じ領域確保／解放 | メモリ確保・解放処理に依存 |

引数なしの `construct(p)` は、クラス型なら引数なしのコンストラクタを呼びます。`int` などの整数型では `T()` のような 0 初期化を行わず、初期値が決まらないため、必ず代入してから読み出してください。

$$n$$ 要素の初期化は `int` など初期化処理が不要な型なら $$O(1)$$、その他は $$O(n)$$、空間は $$O(n)$$ です。メモリ確保のコストは除きます。
