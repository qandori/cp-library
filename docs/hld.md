---
title: HLD (Heavy-Light Decomposition)
documentation_of: //tree/hld.hpp
library_category: data_structure/advanced
redirect_from:
- /graph/hld.hpp.html
- /data_structure/advanced/hld.hpp.html
- /lightweight/data_structure/advanced/hld.hpp.html
- /data_structure/hld.hpp.html
- /lightweight/data_structure/hld.hpp.html
---

木の経路を少数の連続区間へ分け、セグメント木などの配列用データ構造で扱えるようにします。子がある頂点では、部分木の頂点数が最大の子を 1 つ選んで「重い子」と呼び、その子をたどってできる経路を「連鎖」と呼びます。

```cpp
#include "lib/tree/hld.hpp"
#include <cassert>

int main() {
	HLD tree(3);
	tree.add_edge(0, 1);
	tree.add_edge(1, 2);
	tree.build();
	auto segments = tree.path(2, 0);
	auto [l, r] = tree.subtree(1);
	assert(!segments.empty() && r - l == 2);
	assert(tree.distance(0, 2) == 2);
}
```

以下の $$n$$ は頂点数です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `HLD(n = 0)` | 頂点数を指定 | $$O(n)$$ |
| `add_edge(u,v)` | 無向辺を追加し、辺 ID を返す | 償却 $$O(1)$$ |
| `build(root = 0)` | 指定した根で木を分解 | $$O(n)$$ |
| `build_from_parents(parents)` | 親配列から木を構築し、分解 | $$O(n)$$ |
| `lca(u,v)` | 最小共通祖先を `int` で返す | $$O(\log n)$$ |
| `dist(u,v)` / `distance(u,v)` | 経路の辺数を `int` で返す。どちらも同じ操作 | $$O(\log n)$$ 時間・$$O(1)$$ 追加空間 |
| `is_ancestor(u,v)` | `u` が `v` の祖先か。自身も含む | $$O(1)$$ |
| `ancestor(v,k)` | `k` 個上の祖先。範囲外は -1 | $$O(\log n)$$ |
| `jump(u,v,k)` | `u` を 0 番目とする経路上の頂点。範囲外は -1 | $$O(\log n)$$ |
| `subtree(v,edge = false)` | 部分木の添字区間。辺の場合は親への辺を除く | $$O(1)$$ |
| `edge_vertex(id)` | 辺 ID の深い側の頂点 | $$O(1)$$ |
| `path(u,v,edge = false)` | 経路順の `Segment{l,r,reversed}` の列 | $$O(\log n)$$ |
| `for_each_path(u,v,f,edge = false)` | 経路順に `f(l,r,reversed)` を呼ぶ | $$O(\log n)$$ |
| `parent[v]` / `depth[v]` / `size[v]` | 親／深さ／部分木の頂点数 | $$O(1)$$ |
| `heavy[v]` / `head[v]` | 重い子／連鎖の先頭 | $$O(1)$$ |
| `in[v]` / `out[v]` / `order[i]` | 部分木の左端／右端／添字に対応する頂点 | $$O(1)$$ |
| `n` / `edges[id]` | 全頂点数／辺の両端 | $$O(1)$$ |

頂点数は `0 <= n <= INT_MAX / 2`、頂点番号は `[0,n)` です。空でない入力は連結な無向木とし、辺をちょうど `n-1` 本追加してから `build(root = 0)` で `[0,n)` の根を指定します。空の木も構築できますが、頂点を指定する問い合わせはできません。頂点値は Segtree などの `in[v]` に置きます。

親配列がある場合は `build_from_parents(std::vector<int> parents)` を使えます。頂点数は配列の長さとなり、根の親を `-1`、それ以外の親を頂点番号で指定します。根はちょうど 1 頂点で、全頂点が根につながる木である必要があります。頂点番号は任意の順序で構いません。空配列も渡せます。

```cpp
#include "lib/tree/hld.hpp"

HLD rooted;
rooted.build_from_parents({-1, 0, 1});
auto root = rooted.parent[0]; // -1
auto distance = rooted.dist(0, 2); // 2
```

既存の頂点・辺は置き換わります。`edges` には根以外の頂点 `v` の `(parents[v], v)` が `v` の昇順で入り、その順番が辺 ID になります。配列を以降使わない場合は `build_from_parents(std::move(parents))` と渡せます。

`in[v]` は木を分解した後の配列上の位置で、元の頂点番号 `v` とは別です。元の頂点値が `value[v]` なら、長さ `n` の配列に `a[tree.in[v]] = value[v]` と並べ替えてから Segtree を構築します。`subtree(v)` の `[l,r)` と `path` が返す区間は、この配列に対する範囲です。`tree.order[i]` で位置 `i` に対応する元の頂点番号へ戻せます。

`path(u,v,edge = false)` は経路を `u` から `v` の順に並べた `Segment{l,r,reversed}` の列を返します。各区間は $$[l,r)$$ で、`reversed` が真なら降順に読みます。順序を変えると結果が変わる演算では、区間の正順・逆順の演算結果を用意して使い分けます。`for_each_path(u,v,f,edge = false)` は同じ順序で `f(l,r,reversed)` を呼び、区間列のメモリ確保を省きます。

次は `in[v]` 順の値で構築した区間和のセグメント木 `seg` を使う部分です。`u`・`v` は経路の両端で、和は足す順序によらないので `reversed` を使いません。

```cpp
long long answer = 0;
tree.for_each_path(u, v, [&](int l, int r, bool) { answer += seg.prod(l, r); });
```

辺値は `in[tree.edge_vertex(id)]` に置き、`edge = true` を指定します。`id` は `add_edge` の返り値です。`subtree(v,edge = false)` は部分木の区間を返し、辺の場合は `v` と親の辺を除きます。

`lca(u,v)`・`dist(u,v)`・`distance(u,v)`・`is_ancestor(u,v)`、`ancestor(v,k)`（`k` 個上の祖先）・`jump(u,v,k)`（経路の 0 始まり `k` 番目の頂点）が使えます。`ancestor`・`jump` は範囲外なら `-1` です。

構築後の各配列は参照用です。`parent[v]` は親（根は `-1`）、`depth[v]` は深さ、`size[v]` は部分木サイズ、`heavy[v]` は重い子（なければ `-1`）、`head[v]` は連鎖の先頭です。`in[v]`・`out[v]` は部分木の区間、`order[i]` は添字 `i` の頂点です。

頂点数を $$n$$ として、構築・空間は $$O(n)$$、`lca`・`dist`・`distance`・`ancestor`・`jump`・パス分解は $$O(\log n)$$、それ以外のクエリは $$O(1)$$ です。`for_each_path` は $$O(\log n)$$ 回 `f` を呼び、その処理時間を別に加えます。表では `f` の 1 回の実行を $$O(1)$$ とします。分解される区間数は $$O(\log n)$$、Segtree と組み合わせたパスクエリは $$O(\log^2 n)$$ です。
