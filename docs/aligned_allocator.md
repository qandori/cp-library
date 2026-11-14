---
title: 整列した配列のアロケータ
documentation_of: //components/aligned_allocator.hpp
library_category: components/util
---

`std::vector` などのメモリ確保・解放を担当する部品（アロケータ）です。確保する先頭アドレスを指定したバイト数の倍数に揃えられます。

```cpp
#include "lib/components/aligned_allocator.hpp"
#include <cstdint>
#include <vector>

std::vector<std::int64_t, AlignedAllocator<std::int64_t>> values(100);
```

`AlignedAllocator<T,Alignment=64>` は、確保する先頭アドレスを `Alignment` byte の倍数に揃えます。`Alignment` は `alignof(T)` 以上の 2 の冪です。`allocate(n)` は `n` 要素の未初期化領域を返し、`deallocate(p,n)` は同じアロケータで確保した領域を解放します。同じ `Alignment` のアロケータは互換です。

確保できない場合は `std::bad_alloc`、要素数からバイト数への変換がオーバーフローする場合は `std::bad_array_new_length` を送出します。

確保する空間は $$O(n)$$ です。確保と解放の時間は OS とアロケータに依存します。
