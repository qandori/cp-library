---
title: "強連結成分分解の共通部品"
documentation_of: //components/scc_core.hpp
library_category: components/graph
---

```cpp
#include "lib/components/scc_core.hpp"

std::vector<std::pair<int, int>> edges{ {0, 1}, {1, 0}, {1, 2} };
auto [count, id] = scc_detail::ids(3, edges);
```

通常は [SCCGraph](scc.md) または [TwoSAT](two_sat.md) を使います。この部品は有向辺の配列を `std::span<const std::pair<int,int>>` として受け取り、成分数と各頂点の成分 ID を返します。入力は変更しません。互いに到達できる頂点を同じ成分とし、端点の成分が異なる辺では ID が必ず増えるように番号を付けます。

頂点数は非負の `int`、辺数は `INT_MAX` 以下、端点は `[0,n)` が必要です。空グラフ・自己ループ・多重辺に対応し、DFS の再帰は使いません。

頂点数を $$n$$、辺数を $$m$$ として、時間・追加空間は $$O(n+m)$$ です。
