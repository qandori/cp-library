---
title: Link Cut Tree
documentation_of: //tree/link_cut_tree.hpp
redirect_from:
- /data_structure/link_cut_tree.hpp.html
library_category: tree/dynamic
---

頂点に値を持たせる Link Cut Tree です。辺の値を扱う場合や、辺を通る方向ごとに異なる値を使う場合は [辺の値を扱う Link Cut Tree](edge_link_cut_tree.md) を使います。

```cpp
#include "lib/tree/link_cut_tree.hpp"

std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }

LinkCutTree<std::int64_t, op, e, true> tree(std::vector<std::int64_t>{1, 2, 3});
tree.build_forest(std::vector<std::pair<int, int>>{ {0, 1}, {1, 2} });
auto value = tree.prod(0, 2); // 6
```

経路上の頂点値を `op` で順にまとめた結果を取得します。`op(op(a,b),c) == op(a,op(b,c))` を満たし、`op(e(),x)`・`op(x,e())` がどちらも `x` になるように定義してください。第 4 テンプレート引数は、`op(a,b) == op(b,a)` が常に成り立つ場合のみ `true` にします。`tree(n)` では各頂点を `e()` で初期化します。頂点番号は 0 始まりです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `LinkCutTree(n = 0)` / `LinkCutTree(values)` | 頂点数／頂点値の配列から、辺のない森を構築 | $$O(n)$$ |
| `size()` | 全頂点数 | $$O(1)$$ |
| `build_forest(edges)` | 頂点値を保持し、指定した無向辺の森で構築し直す | $$O(n+m)$$ |
| `link(u,v)` / `cut(u,v)` | 辺を追加／削除。成功時に true | 償却 $$O(\log n)$$ |
| `same(u,v)` | 連結判定 | 償却 $$O(\log n)$$ |
| `get(v)` / `set(v,value)` | 頂点値の取得／更新 | get: $$O(1)$$、set: 償却 $$O(\log n)$$ |
| `prod(u,v)` | `u` から `v` への順序で経路を集約 | 償却 $$O(\log n)$$ |
| `path_size(u,v)` / `distance(u,v)` | 経路の頂点数／辺数 | 償却 $$O(\log n)$$ |
| `jump(u,v,k)` | `u` を 0 番目とする経路上の頂点。範囲外は -1 | 償却 $$O(\log n)$$ |
| `make_root(v)` / `root(v)` | 根の変更／取得 | 償却 $$O(\log n)$$ |
| `lca(r,u,v)` | `r` を根とした LCA | 償却 $$O(\log n)$$ |

`build_forest` は `vector<pair<int,int>>` など、辺の両端の配列を渡します。自己ループ・多重辺・閉路のない森とし、孤立点や空の森も渡せます。既存の辺は置き換わります。

経路操作の両端、および `lca` の 3 頂点は連結である必要があります。

頂点数を $$n$$、一括構築する辺数を $$m$$ として、構築・`build_forest` は $$O(n+m)$$ 時間、空間は $$O(n)$$ です。`get`・`size()` は $$O(1)$$、その他の操作は償却 $$O(\log n)$$ です。`op`・`e` と値のコピーは $$O(1)$$ とします。
