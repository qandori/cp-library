---
title: "Static Top Tree の構造"
documentation_of: //components/static_top_tree_structure.hpp
library_category: components/tree
---

`static_top_tree_detail::Structure` は Static Top Tree のクラスタ構造を作る内部部品です。クラスタは、木の一部をまとめて計算する単位を指します。DP の値には依存しません。構築済みの HLD を渡します。

| 操作・メンバ | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Structure()` | 空の構造を作る | $$O(1)$$ |
| `Structure(tree)` / `build(tree)` | HLD から構築／再構築 | $$O(n\log n)$$ |
| `Kind` | `Vertex`・`Compress`・`Rake` の 3 種類 | —（型） |
| `Node` | `parent`・`left`・`right`・`index`・`kind` を持つクラスタ | —（型） |
| `nodes` | クラスタ配列。先頭 `n` 個は頂点番号に対応する `Vertex` | $$O(1)$$ |
| `order` | 子から親への評価順。頂点番号順とは限らない | $$O(1)$$ |
| `parent[v]` | 元の木の親。根では `-1` | $$O(1)$$ |
| `edge_child[e]` | 入力した辺 `e` の子側の頂点 | $$O(1)$$ |
| `root` / `root_vertex` | 最終クラスタ／元の木の根。空なら `-1` | $$O(1)$$ |
| `paths` / `points` | `Vertex`・`Compress` の合計数／`Rake` の数 | $$O(1)$$ |
| `height` | クラスタ木の最大深さ。空・1 頂点では 0 | $$O(1)$$ |

各頂点で、部分木の頂点数が最大になる子を 1 つ選んで重い子とします。`Vertex` の `left` は、重い子以外の子からの情報をまとめたもので、存在しない場合は `-1` です。`right` は使いません。`Compress` は上側・下側の順に、`Rake` は同じ親に接続する枝として `left`・`right` を併合します。`parent` はクラスタ木の親で、最終クラスタのみ `-1` です。`index` は `Vertex`・`Compress` 用の配列、または `Rake` 用の配列に保存する際の添字です。

構築・走査には再帰を使いません。

構築済みの HLD の頂点数を $$n$$ として、構築は $$O(n\log n)$$ 時間、空間は $$O(n)$$、クラスタ木の高さは $$O(\log n)$$ です。
