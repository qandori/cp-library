---
title: FPS の作業領域
documentation_of: //components/fps_workspace.hpp
library_category: components/fps
---

FPS 内部で使う、先頭アドレスが 64 byte の倍数となるように配置した `unsigned int` の作業領域です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::allocate_fps_words(n)` | n 個の 32 bit 整数の作業領域の所有権を返す | OS のメモリ確保処理に依存 |
| `storage[i]` | i 番目の 32 bit 整数への参照 | $$O(1)$$ |
| `storage.get()` | 先頭の 32 bit 整数へのポインタ | $$O(1)$$ |

```cpp
#include "lib/components/fps_workspace.hpp"

auto storage = fps_detail::allocate_fps_words(1024);
storage[0] = 1;
```

`FpsWords` は `std::move` で別の変数に移せる配列で、破棄時に領域を自動で解放します。要素の初期値は未規定で、使用前に書き込む必要があります。確保できなければ `std::bad_alloc` を送出します。

**計算量**: 作業領域は $$O(n)$$ です。確保と解放の時間は OS のメモリ確保処理に依存します。
