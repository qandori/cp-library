---
title: Dijkstra の一意経路の探索
documentation_of: //components/dijkstra_unique.hpp
library_category: components/graph
---

[Dijkstra](https://qandori.github.io/cp-library/graph/dijkstra.hpp.html) が内部で使う、各頂点の入次数が高々 1 の場合の探索です。通常は `graph/dijkstra.hpp` を include し、[隣接辺を連続配列に保存する DirectedCSR](directed_csr.md) に対して `dijkstra_path(g,start,target)` を呼びます。条件の判定と使い分けは自動で行います。

各頂点へ入る辺が高々 1 本なら、始点から各頂点へ向かう単純経路も高々 1 本です。距離の大小による取り出し順を決める必要がないため、優先度付きキューを省けます。出辺が 1 本の区間は直接たどり、分岐した先だけを配列へ保存します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `shortest_path_detail::unique_path(g,start,target,inf,parent)` | 最短距離と始点・終点を含む頂点列を返す。到達不能は `{T(-1), {}}` | $$O(n+m)$$ |

内部関数の `g` は `size()` と `g[v]` を持ち、`g[v]` は `empty()`・`size()`・添字アクセスが使える `{行き先, 重み}` の範囲を返すものとします。辺への参照は探索中に有効である必要があります。全頂点の入次数は高々 1、重みは非負、`start != target`、`inf` は正とします。`parent` は長さ $$n$$ の配列を渡します。公開関数は必要な準備を行います。

始点に戻る辺を省けば、同じ頂点へ 2 回進むことはありません。始点以外で閉路へ合流するには、その頂点の入次数が 2 以上必要になるためです。目的地を最初に見つけた時点で経路が確定します。経路上の重みは始点からの順に加算し、`inf` 以上になる部分は探索しません。

頂点数を $$n$$、辺数を $$m$$ として $$O(n+m)$$ 時間・追加空間です。要素の演算・比較・コピーを $$O(1)$$ とします。
