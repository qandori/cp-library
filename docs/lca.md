---
title: LCA
documentation_of: //tree/lca.hpp
redirect_from:
- /graph/lca.hpp.html
library_category: tree/query
---

根付き木の 2 頂点に対し、両方の祖先で最も深い頂点を求めます。`add_edge` で木の辺を全て追加し、`build` を呼んでから問い合わせます。

```cpp
#include "lib/tree/lca.hpp"

LCA tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
int v = tree.lca(0, 2); // 0
int distance = tree.dist(0, 2); // 2
auto [l, r] = tree.subtree(1);
int count = r - l; // 2
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `LCA(n = 0)` | 頂点数を指定 | $$O(n)$$ |
| `add_edge(u,v)` | 無向辺を追加し、辺 ID を返す | 償却 $$O(1)$$ |
| `build(root = 0)` | 指定した根で前処理 | $$O(n\log n)$$ |
| `build_from_parents(parents)` | 親配列から木を構築して前処理 | $$O(n\log n)$$ |
| `lca(u,v)` | 最小共通祖先 | $$O(1)$$ |
| `dist(u,v)` | 経路の辺数 | $$O(1)$$ |
| `is_ancestor(u,v)` | `u` が `v` の祖先か。自身も含む | $$O(1)$$ |
| `subtree(v)` | DFS の行きがけ順での部分木の半開区間 `[l,r)`。`l` は `v` の位置 | $$O(1)$$ |
| `parent[v]` / `depth[v]` | 親（根は -1）／根からの辺数 | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／辺の両端 | $$O(1)$$ |

無向木を扱い、`0 <= n <= INT_MAX/2`、頂点番号は `[0,n)` とします。`build(root = 0)` で根を指定します。空の木では頂点を指定する問い合わせはできません。公開の `n`・辺・親・深さは直接書き換えないでください。

親配列がある場合は `build_from_parents(std::vector<int> parents)` を使えます。頂点数は配列の長さとなり、根の親を `-1`、それ以外の親を `[0,n)` の頂点番号で指定します。根はちょうど 1 頂点で、全頂点が根につながる木である必要があります。頂点番号は任意の順序で構いません。空配列も渡せます。

```cpp
#include "lib/tree/lca.hpp"

LCA rooted;
rooted.build_from_parents({-1, 0, 1});
int ancestor = rooted.lca(1, 2); // 1
```

既存の頂点・辺は置き換わります。`edges` には根以外の頂点 `v` の `(parents[v], v)` が `v` の昇順で入り、その順番が辺 ID になります。配列を以降使わない場合は `build_from_parents(std::move(parents))` と渡せます。

`subtree(v)` の区間は、元の頂点番号の区間ではありません。根から DFS して初めて訪れる順に各頂点へ番号を付けたとき、`v` とその子孫が占める区間 `[l,r)` です。`l` が `v` 自身の DFS 番号、`r-l` が `v` を含む部分木の頂点数です。

辺を追加する段階で閉路のない連結な木にし、空でなければ辺数を `n-1` にしてください。辺や根を変えた後は `build` を呼び直します。距離は重みの合計ではなく辺数です。

前処理・空間計算量は $$O(n \log n)$$、各クエリは $$O(1)$$ です。
