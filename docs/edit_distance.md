---
title: "Edit Distance"
documentation_of: //string/edit_distance.hpp
---

```cpp
#include "lib/string/edit_distance.hpp"

int distance = edit_distance(s, t);
```

`std::string_view` で 2 つの文字列を受け取り、挿入・削除・置換をそれぞれコスト 1 とする編集距離を返します。文字はバイト単位で扱います。

長い方の長さを $$n$$、短い方を $$m$$ として、時間は $$O(n\lceil m/64\rceil)$$、補助空間は $$O(m)$$ です。
