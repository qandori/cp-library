---
title: 有向グラフの CSR
documentation_of: //components/directed_csr.hpp
library_category: components/graph
---

`DirectedCSR<T>` は、各頂点の出辺を一つの連続配列にまとめて保存する有向グラフです。辺の重みを `T` で保存します。

```cpp
#include "lib/components/directed_csr.hpp"
#include "lib/graph/dijkstra.hpp"
#include <cstdint>
#include <vector>

int main() {
	using Graph = DirectedCSR<std::uint32_t>;
	std::vector<Graph::Edge> edges = { {0, 2, 9}, {1, 2, 3}, {0, 1, 2}, {0, 1, 5} };
	Graph g(4, edges);
	auto dist = dijkstra(g, 0); // {0,2,5,-1}
	auto [cost, path] = dijkstra_path(g, 0, 2); // 5, {0,1,2}
}
```

入力の辺は `{始点, 終点, 重み}`、頂点番号は 0 始まりです。自己辺・多重辺も保存します。無向グラフとして使う場合は、両方向の辺を入力します。

`g[v]` は頂点 `v` から出る辺の範囲です。各要素は `{行き先, 重み}` なので、`for (const auto &[to, weight] : g[v])` で走査できます。同じ始点を持つ辺は入力された順に並びます。上の例の `g[0]` は `{2,9}, {1,2}, {1,5}` の順です。

| 操作・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DirectedCSR<T>(n=0)` | `n` 頂点・辺なしで構築 | $$O(n)$$ |
| `DirectedCSR<T>(n,edges)` | `std::span<const Edge>` から構築。`std::vector<Edge>` も渡せる | $$O(n+m)$$ |
| `DirectedCSR<T>(n,m,produce)` | `produce(emit)` を呼び、`emit(Edge)` で渡された `m` 本の辺から構築 | $$O(n+m)$$ と `produce` の実行 |
| `build(n,edges)` | 頂点数と辺を指定して再構築 | $$O(n+m)$$ と旧辺の破棄 |
| `build(n,m,produce)` | 辺を生成する関数を指定して再構築 | $$O(n+m)$$ と `produce` の実行・旧辺の破棄 |
| `size()` | 頂点数を `int` で返す | $$O(1)$$ |
| `empty()` | 頂点数が 0 なら `true` | $$O(1)$$ |
| `edge_count()` | 辺数を `std::size_t` で返す | $$O(1)$$ |
| `operator[](v)` | 出辺を `std::span<const Arc>` で返す | $$O(1)$$ |
| `Edge` | 入力の辺の型。`int from, to` と `T weight` を持つ | —（型） |
| `Arc` | 隣接辺の型。`std::pair<int,T>` | —（型） |
| `weight_type` | 保存する辺の重みの型 `T` | —（型） |
| `distance_type` | 最短経路で既定の距離型として使う型 | —（型） |

`distance_type` は、8 byte 未満の整数型では `std::int64_t`、それ以外では `T` です。例えば `DirectedCSR<std::uint32_t>` は辺を 32 bit で保存し、`dijkstra` の距離を 64 bit で計算します。64 bit 整数・浮動小数点型の距離型は変わりません。

辺を読みながら構築する場合は、次のように `produce` を渡します。`produce` は 1 回呼ばれ、受け取った `emit` を `m` 回呼んで辺を渡します。辺を受け取る処理と出次数の集計をまとめるため、辺配列を別に用意して走査する必要がありません。

```cpp
#include "lib/components/directed_csr.hpp"
#include <iostream>

int n, m;
std::cin >> n >> m;
using Graph = DirectedCSR<std::uint32_t>;
Graph g(n, m, [&](auto emit) {
	for (int i = 0; i < m; ++i) {
		Graph::Edge edge;
		std::cin >> edge.from >> edge.to >> edge.weight;
		emit(edge);
	}
});
```

`emit` の呼び出し順が辺の入力順です。辺数が `m` と異なる場合は `std::length_error` を送出します。`produce`・辺の格納中に例外が出た場合は、指定した頂点数の辺なしグラフに戻して例外を送出します。`emit` は `produce` の呼び出し中だけ使い、保存しないでください。一時的に $$O(m)$$ の辺配列を内部で確保し、構築後に解放します。

全頂点の出次数が高々 1 で、出辺のない頂点が高々 1 個なら、辺の格納位置を頂点番号から直接計算します。通常の CSR と同じ辺配列を使い、`g[v]` の返り値や辺の順序は変わりません。`dijkstra_path` もこの構造を自動で利用します。

入次数が全頂点で高々 1 かも構築中に判定します。辺数が頂点数以下の場合だけ、一時的に頂点当たり 1 bit の配列を使います。`dijkstra_path` はこの条件を満たすグラフでも優先度キューを省略できます。

入力の辺はコピーするため、構築後に入力配列を変更・破棄してもグラフは変わりません。辺の追加・変更には `build` で再構築します。`g[v]` の返す範囲やその要素への参照は辺配列を参照します。`build`・代入・移動の後は取得し直し、グラフの破棄後には使わないでください。

`T` はデフォルト構築・コピー・代入できる型とします。グラフの保存自体は重みの符号を制限しません。最短経路アルゴリズムに渡す場合は、そのアルゴリズムの重みの条件に従います。

$$n$$ 頂点・$$m$$ 辺として、重み 1 個の構築・コピー・代入を $$O(1)$$ とすると構築は $$O(n+m)$$ 時間・空間です。再構築時には旧辺の破棄も行い、確保した配列の容量は保持します。繰り返し再構築する場合の確保領域は、過去最大の頂点数・辺数に比例します。
