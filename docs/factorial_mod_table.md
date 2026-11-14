---
title: Factorial Mod Table
documentation_of: //components/factorial_mod_table.hpp
library_category: components/math
---

`factorial_mod` が使う、法 998244353 専用の定数表です。`factorial_mod_detail::table` は長さ 15233 の `std::array<std::uint32_t,15233>` で、添字 `k` の値は `(65536*k)! mod 998244353` です。`0 <= k <= 15232` とし、先頭は 1、末尾は 998244352 です。実行時の初期化は行いません。

```cpp
#include "lib/components/factorial_mod_table.hpp"
#include <cassert>

int main() {
	assert(factorial_mod_detail::table.front() == 1);
	assert(factorial_mod_detail::table.back() == 998244352);
}
```

一般の利用では、この内部配列を直接参照せず `math/factorial_mod.hpp` の `factorial_mod(n)` を使います。

要素取得は $$O(1)$$ 時間、保持空間は 60932 バイトです。引数・動的な初期化・ヒープ領域はありません。
