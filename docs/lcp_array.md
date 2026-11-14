---
title: LCP Array
documentation_of: //string/lcp_array.hpp
library_category: string/substring
---

```cpp
#include "lib/string/suffix_array.hpp"
#include "lib/string/lcp_array.hpp"
#include <cassert>

int main() {
	std::string_view s = "banana";
	auto lcp = lcp_array(s, suffix_array(s));
	assert((lcp == std::vector<int>{1, 3, 0, 0, 2}));
}
```

`lcp_array(s, sa)` は、[Suffix Array](suffix_array_public.md)（接尾辞を辞書順に並べた開始位置の配列）の隣り合う接尾辞の最長共通接頭辞長を `std::vector<int>` で返します。`lcp[i]` は `s[sa[i]..]` と `s[sa[i+1]..]` の共通接頭辞の長さです。

`s` は `std::string_view` または `std::vector<T>`、`sa` は `std::vector<int>` です。長さは `INT_MAX` 以下で、`sa` は同じ列の Suffix Array である必要があります。`T` は `==` に対応し、Suffix Array 構築時の等値関係と一致させてください。返り値の長さは、非空の列では `s.size()-1` です。空列・長さ 1 の列では空配列を返します。

長さ $$n$$ に対し $$O(n)$$ 時間・追加空間です。空列は定数時間・空間です。
