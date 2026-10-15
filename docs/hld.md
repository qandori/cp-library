---
title: "HLD (Heavy-Light Decomposition)"
documentation_of: //graph/hld.hpp
---

```cpp
#include "lib/graph/hld.hpp"

HLD tree(3);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.build();
auto segments = tree.path(2, 0);
auto [l, r] = tree.subtree(1);
```

無向木を扱い、`build(root = 0)` で根を指定します。頂点値は Segtree などの `in[v]` に置きます。

`path(u,v,edge = false)` は経路を `u` から `v` の順に並べた `Segment{l,r,reversed}` の列を返します。各区間は $$[l,r)$$ で、`reversed` が真なら降順に読みます。非可換な演算では、区間の正順・逆順の積を用意して使い分けます。`for_each_path(u,v,f,edge = false)` は同じ順序で `f(l,r,reversed)` を呼び、区間列のメモリ確保を省きます。

```cpp
long long answer = 0;
tree.for_each_path(u, v, [&](int l, int r, bool) { answer += seg.prod(l, r); });
```

辺値は `in[tree.edge_vertex(id)]` に置き、`edge = true` を指定します。`id` は `add_edge` の返り値です。`subtree(v,edge = false)` は部分木の区間を返し、辺の場合は `v` と親の辺を除きます。

`lca(u,v)`・`dist(u,v)`・`is_ancestor(u,v)`、`ancestor(v,k)`（`k` 個上の祖先）・`jump(u,v,k)`（経路の 0 始まり `k` 番目の頂点）が使えます。`ancestor`・`jump` は範囲外なら `-1` です。

`parent[v]` は親（根は `-1`）、`depth[v]` は深さ、`size[v]` は部分木サイズ、`heavy[v]` は重い子（なければ `-1`）、`head[v]` は連鎖の先頭です。`in[v]`・`out[v]` は部分木の区間、`order[i]` は添字 `i` の頂点です。

構築・空間は $$O(n)$$、`lca`・`dist`・`ancestor`・`jump`・パス分解は $$O(\log n)$$、それ以外の問い合わせは $$O(1)$$ です。分解される区間数は $$O(\log n)$$、Segtree と組み合わせたパスクエリは $$O(\log^2 n)$$ です。
