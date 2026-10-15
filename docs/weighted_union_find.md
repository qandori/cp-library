---
title: "重み付き Union-Find"
documentation_of: //data_structure/weighted_union_find.hpp
---

```cpp
#include "lib/data_structure/weighted_union_find.hpp"

WeightedUnionFind<std::int64_t> uf(3);
bool ok = uf.merge(0, 1, 5);
auto value = uf.diff(0, 1);
```

`merge(x, y, w)` は `potential[y] - potential[x] = w` を追加し、矛盾する場合に `false` を返します。`diff(x, y)` は連結な頂点間の `potential[y] - potential[x]` を返します。

`same(x, y)` で連結判定、`leader(x)` で代表元、`size(x)` で連結成分の頂点数を取得できます。頂点番号は 0 始まりです。

頂点数を `n` として、構築・空間は `O(n)`、各操作は償却 `O(α(n))` です。`α` は逆アッカーマン関数で、`T` の演算・コピーは `O(1)` とします。
