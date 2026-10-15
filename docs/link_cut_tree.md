# Link Cut Tree

```cpp
#include "lib/data_structure/link_cut_tree.hpp"

int64_t op(int64_t a, int64_t b) { return a + b; }
int64_t e() { return 0; }

LinkCutTree<int64_t, op, e, true> tree(vector<int64_t>{1, 2, 3});
tree.link(0, 1);
auto value = tree.prod(0, 1);
```

`op` は結合則を満たす演算、`e()` は単位元です。第 4 テンプレート引数は可換な場合のみ `true` にします。`tree(n)` では各頂点を単位元で初期化します。頂点番号は 0 始まりです。

| 操作 | 内容 |
| --- | --- |
| `link(u, v)` / `cut(u, v)` | 辺を追加／削除。成功時に `true` |
| `same(u, v)` | 連結判定 |
| `get(v)` / `set(v, value)` | 頂点の値の取得／更新 |
| `prod(u, v)` | `u` から `v` への順序で経路を集約 |
| `path_size(u, v)` / `distance(u, v)` | 経路の頂点数／辺数 |
| `jump(u, v, k)` | `u` を 0 番目とする経路上の頂点。範囲外は `-1` |
| `make_root(v)` / `root(v)` | 根の変更／取得 |
| `lca(r, u, v)` | `r` を根とした LCA |

経路操作の両端、および `lca` の 3 頂点は連結である必要があります。
