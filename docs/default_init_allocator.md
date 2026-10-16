---
title: "Default Init Allocator"
documentation_of: //components/default_init_allocator.hpp
redirect_from: /data_structure/default_init_allocator.hpp.html
---

```cpp
#include "lib/components/default_init_allocator.hpp"
#include <vector>

std::vector<int, DefaultInitAllocator<int>> a(n);
```

要素を値初期化せずデフォルト初期化します。整数などの要素は読み出す前に代入してください。

$$n$$ 要素の初期化は自明な型なら $$O(1)$$、その他は $$O(n)$$、空間は $$O(n)$$ です。メモリ確保のコストは除きます。
