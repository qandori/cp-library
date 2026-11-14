---
title: 二部マッチング
documentation_of: //graph/bipartite_matching.hpp
library_category: graph/matching
redirect_from:
- /graph/matching/bipartite_matching.hpp.html
- /lightweight/graph/matching/bipartite_matching.hpp.html
---

二部グラフの最大マッチングと、それを使う各種の集合・証明を求めます。左右の頂点番号はそれぞれ 0 始まりです。重み・容量は持ちません。無向隣接リストをそのまま渡し、元の頂点番号で結果を受け取りたい場合は [二部グラフ（自動色分け）](bipartite_graph.md) を使います。

```cpp
#include "lib/graph/bipartite_matching.hpp"
#include <cassert>

int main() {
	BipartiteMatching graph(3, 2);
	graph.add_edge(0, 0);
	graph.add_edge(1, 0);
	graph.add_edge(1, 1);
	assert(graph.matching_size() == 2);
	auto matching = graph.maximum_matching();
	assert(matching.size() == 2);
	auto cover = graph.minimum_vertex_cover();
	auto independent = graph.maximum_independent_set();
	auto witness = graph.hall_witness_left();
	assert(witness && witness->left.size() > witness->right.size());
	auto status = graph.edge_classification();
	assert(status[2] == BipartiteMatching::EdgeStatus::forced);
}
```

`matching_size()` は最新の最大マッチングの本数、`maximum_matching()` はその頂点対の一覧を返します。辺を追加した後も、そのまま呼び出せます。引数・返り値の形は [二部グラフ（自動色分け）](bipartite_graph.md) の同名の操作と共通です。

$$L,R$$ を左右の頂点数、$$N=L+R$$、$$M$$ を追加した辺数、$$K=\min(L,R)$$ とします。`matching_size()`・`maximum_matching()` の計算量には、最大マッチングの計算を含めています。それ以外の結果取得操作は、最新の最大マッチングを計算済みの場合の計算量です。結果取得時に未計算なら、先に `build()` の計算量が加わります。最大マッチング計算の式は $$K\geq1$$ の場合で、左右のいずれかが空の場合は計算不要です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BipartiteMatching(l,r)` | 左 `l` 頂点・右 `r` 頂点の空グラフ | $$O(N)$$ |
| `left_size()` / `right_size()` | 左／右の頂点数 | $$O(1)$$ |
| `edge_count()` | 入力辺の本数 | $$O(1)$$ |
| `reserve(m)` | 入力辺の領域を予約。省略可能 | $$O(M+m)$$ |
| `add_edge(u,v)` | 左 `u` と右 `v` を結び、辺 ID を返す | 償却 $$O(1)$$ |
| `get_edge(id)` | 入力辺の頂点対 `(左,右)` | $$O(1)$$ |
| `build()` | 最大マッチングの本数を返す。辺追加後も呼べる | $$O((N+M)\sqrt K)$$ |
| `matching_size()` | 最新の最大マッチングの本数を `int` で返す | 時間 $$O((N+M)\sqrt K)$$、追加空間 $$O(N+M)$$ |
| `maximum_matching()` | 最新の最大マッチングの頂点対 `(左,右)` を `vector<pair<int,int>>` で返す | 時間 $$O((N+M)\sqrt K)$$、追加空間 $$O(N+M)$$ |
| `left_mates()` / `right_mates()` | 相手の番号の `const vector<int>&`。未対応は -1 | $$O(1)$$ |
| `covers_left()` / `covers_right()` | 左／右の全頂点を対応付けられるか | $$O(1)$$ |
| `edges()` | 最大マッチングの頂点対 `(左,右)` の一覧 | $$O(L)$$ |
| `matching_edge_ids()` | 最大マッチングに採用した入力辺 ID の一覧 | $$O(N+M)$$ |
| `minimum_vertex_cover()` | 全辺を少なくとも片端で覆う最小の頂点集合 | $$O(N+M)$$ |
| `maximum_independent_set()` | 内部に辺がない最大の頂点集合 | $$O(N+M)$$ |
| `minimum_edge_cover()` | 全頂点を覆う最小の入力辺 ID の一覧。存在しなければ `nullopt` | $$O(N+M)$$ |
| `hall_witness_left()` | 左全体を対応付けられない Hall 条件の反例。可能なら `nullopt` | $$O(N+M)$$ |
| `hall_witness_right()` | 右全体を対応付けられない Hall 条件の反例。可能なら `nullopt` | $$O(N+M)$$ |
| `edge_classification()` | 各入力辺が最大マッチングに使えるか・必須かを返す | $$O(N+M)$$ |
| `clear()` | 頂点数を維持し、全辺・マッチング・解析結果を削除 | $$O(N+M)$$ |

`add_edge(u,v)` の ID は 0 から追加順です。多重辺も使えます。頂点数は非負で和が `INT_MAX` 未満、辺数は `INT_MAX` 以下にします。辺を追加してから結果取得の操作を呼ぶと、新しいグラフで最大マッチングを更新します。辺追加のない `build()`・`matching_size()` は $$O(1)$$ 時間・追加空間、計算済みの `maximum_matching()` は $$O(L)$$ 時間・返り値を含め $$O(K)$$ 追加空間です。左右のいずれかが空の場合も同じです。辺のないグラフも扱え、マッチングの本数は 0、一覧は空配列になります。

`maximum_matching()`・`edges()`・`matching_edge_ids()` は左頂点の昇順に対応します。`maximum_matching()` と `edges()` は同じ内容を返し、配列は以降の辺追加や計算によって書き換わりません。`left_mates()[u]` は左 `u` の相手、`right_mates()[v]` は右 `v` の相手です。左右の全頂点を覆う完全マッチングが存在する条件は、`covers_left() && covers_right()` です。

`VertexSet` は `left`・`right` という `vector<int>` を持ち、それぞれの側の頂点番号を昇順で返します。最小頂点被覆の頂点数は最大マッチングの本数と等しく、最大独立集合は返した最小頂点被覆の補集合です。

最小辺被覆は、孤立頂点がある場合に存在しません。存在すれば辺数は $$N-\lvert\text{matching}\rvert$$ です。左右とも 0 頂点の場合は、空の辺 ID の一覧を返します。

左の Hall 条件の反例では、`left` が左の集合 $$S$$、`right` がその隣接頂点全体 $$T$$ で、$$\lvert S\rvert>\lvert T\rvert$$ を満たします。つまり、`left` の全頂点を相異なる右頂点へ対応付けることはできません。右の反例では `right` が $$S$$、`left` が $$T$$ です。

`edge_classification()` は、入力辺 ID を添字とする `vector<EdgeStatus>` を返します。

| `EdgeStatus` | 意味 |
| --- | --- |
| `never` | どの最大マッチングにも含まれない |
| `optional` | 含む最大マッチングと含まない最大マッチングが両方ある |
| `forced` | 全ての最大マッチングに含まれる |

多重辺は ID ごとに判定します。同じ頂点対に複数の辺があれば、その頂点対を必ず使う場合でも、各辺は `optional` になることがあります。

構築は $$O(N)$$ 時間、辺追加後の `build()` は $$O((N+M)\sqrt K)$$ 時間、全体の保存領域は $$O(N+M)$$ です。辺追加後に何度も解く場合、各回にこの計算量がかかり得ます。最小被覆・Hall 条件・辺の分類は $$O(N+M)$$ 時間・空間で、解析結果は次の辺追加まで再利用します。
