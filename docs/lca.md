---
title: "LCA"
documentation_of: //graph/lca.hpp
---

```cpp
#include "lib/graph/lca.hpp"

LCA tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
int v = tree.lca(0, 2);
```

無向木を扱い、頂点番号は 0 始まりです。`build(root = 0)` で根を指定します。

- `lca(u, v)`：最小共通祖先
- `dist(u, v)`：経路の辺数
- `is_ancestor(u, v)`：$$u$$ が $$v$$ の祖先か（自身を含む）
- `parent[v]`：親（根は `-1`）
- `depth[v]`：根からの辺数

前処理・空間計算量は $$O(n \log n)$$、各問い合わせは $$O(1)$$ です。
