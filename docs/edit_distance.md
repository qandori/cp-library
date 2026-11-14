---
title: "Edit Distance"
documentation_of: //string/edit_distance.hpp
library_category: string/distance
---

```cpp
#include "lib/string/edit_distance.hpp"

int distance = edit_distance("kitten", "sitting"); // 3
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `edit_distance(s,t)` | 挿入・削除・置換のコストを各 1 とする編集距離を `int` で返す | $$O(n\lceil m/64\rceil)$$ |

`std::string_view` で 2 つの文字列を受け取り、挿入・削除・置換をそれぞれコスト 1 とする編集距離を返します。文字はバイト単位で扱い、NUL・128 以上も含む全 256 種類に対応します。各文字列の長さは `INT_MAX` 以下にしてください。入力は変更しません。

片方が空なら、もう片方の長さを返します。この場合は $$O(1)$$ 時間・追加空間です。

長い方の長さを $$n$$、短い方を $$m$$ として、時間は $$O(n\lceil m/64\rceil)$$、補助空間は $$O(m)$$ です。
