---
title: "Longest Common Subsequence"
documentation_of: //string/lcs.hpp
library_category: string/distance
---

```cpp
#include "lib/string/lcs.hpp"

int length = lcs_length("abcde", "ace"); // 3
std::string sequence = lcs("abcde", "ace"); // "ace"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `lcs_length(s,t)` | 最長共通部分列の長さを `int` で返す | $$O(n\lceil m/64\rceil)$$ |
| `lcs(s,t)` | 最長共通部分列を 1 つ `std::string` で返す | $$O(n\lceil m/64\rceil+n\log n)$$ |

`std::string_view` で 2 つの文字列を受け取ります。`lcs` は最長共通部分列を 1 つ返します。文字はバイト単位で扱い、NUL・128 以上も含む全 256 種類に対応します。各文字列の長さは `INT_MAX` 以下にしてください。入力は変更しません。部分列は文字を飛ばして選べる列です。例の `ace` は連続していませんが、両方の文字列に同じ順序で現れます。答えが複数ある場合はそのうち一つを返します。

片方が空なら、`lcs_length` は 0、`lcs` は空文字列を返します。この場合はともに $$O(1)$$ 時間・追加空間です。

長い方の長さを $$n$$、短い方を $$m$$ として、長さ計算は $$O(n\lceil m/64\rceil)$$、復元は $$O(n\lceil m/64\rceil+n\log n)$$、補助空間はともに $$O(n+m)$$ です。
