---
title: Z algorithm（補助部品）
documentation_of: //components/z_algorithm.hpp
library_category: components/string
---

一致長を計算する関数を受け取り、Z 配列や接頭辞との一致長を列挙する内部部品です。Z 配列は、各位置から始まる列と列全体の先頭が何要素一致するかを並べた配列です。

```cpp
#include "lib/components/z_algorithm.hpp"
#include <string>
#include <vector>

std::string s = "ababa";
std::vector<int> z(s.size());
sequence_match_detail::z_table(int(s.size()), z.data(), [&](int i, int j, int limit) {
	int k = 0;
	while (k < limit && s[i + k] == s[j + k]) ++k;
	return k;
}); // z = {5,0,3,0,1}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `sequence_match_detail::z_table(n,z,match)` | 長さ `n` の列の Z 配列を `z` に書き込む。`z[0]=n` | $$O(n)$$ 回の要素比較 |
| `sequence_match_detail::prefix_matches(n,m,z,match,emit)` | 対象の `n` 個の開始位置について、長さ `m` の接頭辞との一致長を `emit(i,length)` に渡す | $$O(n+m)$$ 回の要素比較 |

`match(i,j,limit)` は、比較する 2 列の位置 `i,j` から高々 `limit` 要素の一致長を返します。`z_table` は同じ列同士、`prefix_matches` は接頭辞の列と対象の列を比較する関数を渡します。

`z_table` は `n>=0` とし、長さ `n` の書き込み領域を用意します。`n=0` では読み書きしません。`prefix_matches` は `m>0` とし、長さ `m` の接頭辞の Z 配列と、各開始位置から `m` 要素まで比較できる対象を渡します。

要素比較を $$O(1)$$ として、Z 配列は時間 $$O(n)$$、接頭辞一致の列挙は時間 $$O(n+m)$$、出力を除く追加空間はいずれも $$O(1)$$ です。
