# Disjoint Sparse Table

```cpp
#include "lib/data_structure/disjoint_sparse_table.hpp"

int op(int a, int b) { return min(a, b); }

DisjointSparseTable<int, op> table(vector<int>{4, 2, 7});
auto value = table.prod(0, 2);
```

`op` は結合則を満たす演算です。`prod(l, r)` は `[l, r)` の集約、`all_prod()` は全体の集約です。

空区間も使う場合は単位元を返す関数 `e()` を定義し、`DisjointSparseTable<S, op, e>` とします。`build(a)` で再構築できます。
