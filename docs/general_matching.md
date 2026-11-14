---
title: General Matching（一般グラフ最大マッチング）
documentation_of: //graph/general_matching.hpp
redirect_from:
- /graph/matching/general_matching.hpp.html
- /lightweight/graph/matching/general_matching.hpp.html
library_category: graph/matching
---

```cpp
#include "lib/graph/general_matching.hpp"
#include <cassert>

int main() {
	GeneralMatching graph(4);
	graph.add_edge(0, 1);
	assert(graph.matching_size() == 1);
	graph.add_edge(1, 2);
	graph.add_edge(0, 3);
	auto pairs = graph.maximum_matching();
	assert(graph.size() == 4 && graph.matching_size() == 2);
	assert(pairs.size() == 2);
	assert(graph.mate[0] == 3 && graph.mate[1] == 2);
}
```

Blossom 法で無向・重みなしグラフの最大マッチングを求めます。`matching_size()` は最新の最大マッチングの本数、`maximum_matching()` はその頂点対の一覧を返します。辺を追加した後も、そのまま呼び出せます。これらの引数・返り値の形は [二部グラフ（自動色分け）](bipartite_graph.md) の同名の操作と共通です。

以下では頂点数を $$n$$、追加した辺数を $$m$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `GeneralMatching(n)` | `n` 頂点の空グラフを構築 | $$O(n)$$ |
| `size()` | 頂点数を `int` で返す | 時間・追加空間とも $$O(1)$$ |
| `reserve(m)` | 入力辺の領域を予約 | $$O(m)$$ |
| `add_edge(u,v)` | 無向辺を追加 | 償却 $$O(1)$$ |
| `build()` | 最大マッチングを計算し、組数を返す | $$O(n^3+nm)$$ |
| `matching_size()` | 最新の最大マッチングの本数を `int` で返す | 時間 $$O(n^3+nm)$$、追加空間 $$O(n+m)$$ |
| `maximum_matching()` | 最新の最大マッチングの頂点対を `vector<pair<int,int>>` で返す | 時間 $$O(n^3+nm)$$、追加空間 $$O(n+m)$$ |
| `edges()` | 現在保持しているマッチングの頂点対を返す。最大かどうかの計算は行わない | $$O(n)$$ |
| `mate[v]` | `build` 後の相手の頂点。なければ -1 | $$O(1)$$ |

頂点は 0 始まりです。`build()` は `matching_size()` と同じ計算を行います。`maximum_matching()`・`edges()` の各組は `(u,v)` で `u < v` を満たし、`u` の昇順に並びます。返した配列は、その後の辺追加や計算によって書き換わりません。`mate[v]` は相手の頂点（なければ `-1`）です。`edges()`・`mate[v]` から最新の最大マッチングを読みたい場合は、先に `build()`・`matching_size()`・`maximum_matching()` のいずれかを呼んでください。`reserve(m)` は辺の事前確保で、省略できます。

頂点数は `0 <= n < INT_MAX`、追加する辺の端点は `[0,n)`、保持する辺数は `INT_MAX / 2` 以下にします。自己ループ・多重辺も渡せます。辺のないグラフの `build()`・`matching_size()` は 0、`maximum_matching()`・`edges()` は空配列を返します。辺の追加後に再び計算できます。`mate` は参照用です。

`build()`・`matching_size()`・`maximum_matching()` は、繰り返し呼び出す場合も 1 回あたり $$O(n^3+nm)$$ 時間です。辺追加がなくても計算し直す場合があります。ライブラリ全体の空間は $$O(n+m)$$ で、`edges()` は $$O(n)$$ 時間・追加空間です。0 頂点の場合はどの操作も定数時間です。
