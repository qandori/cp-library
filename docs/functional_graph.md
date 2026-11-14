---
title: Functional Graph
documentation_of: //graph/functional_graph.hpp
library_category: graph
---

各頂点から出る辺がちょうど 1 本の有向グラフを、閉路とその閉路へ流れ込む木に分解します。閉路の列挙、閉路までの距離、k 回移動した先、到達距離などを求められます。自己ループと複数の連結成分に対応します。

```cpp
#include "lib/graph/functional_graph.hpp"

std::vector<int> to{1, 2, 0, 1, 3, 3, 6};
FunctionalGraph graph(to);
int count = graph.cycle_count();        // 2
int entry = graph.entry[4];             // 1: 最初に到達する閉路上の頂点
int depth = graph.depth[4];             // 2: 4 -> 3 -> 1
int next = graph.jump(4, 3);            // 2: 4 -> 3 -> 1 -> 2
int distance = graph.dist(4, 2);        // 3
int unreachable = graph.dist(2, 4);     // -1
int reachable = graph.reachable_count(4); // 5: 自身を含む到達可能な頂点数
int merge = graph.lca(4, 5);            // 3
auto cycle = graph.cycle(graph.cycle_id[4]); // 0, 1, 2
auto forest = graph.forest();          // 閉路上の辺を除いた無向森
```

入力は `std::vector<int>` の移動先配列です。頂点番号は 0 始まりで、`v -> to[v]` を表します。頂点数は `INT_MAX` 以下とし、全ての `to[v]` を `[0,n)` に入れてください。行き止まりを表す `-1` は扱いません。空配列にも対応し、既定構築も空の状態です。

入力を変更せず、構築結果は入力を参照しません。`FunctionalGraph(std::move(to))` で移動先配列を渡せば、その配列の所有権を移せます。コピーして独立に保持することもできます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `FunctionalGraph(to)` | 移動先配列から構築 | $$O(n)$$ |
| `build(to)` | 指定した配列で構築し直す | $$O(n)$$ |
| `size()` / `empty()` | 頂点数／空かどうか | $$O(1)$$ |
| `cycle_count()` | 閉路の個数。辺の向きを無視した連結成分の個数に等しい | $$O(1)$$ |
| `cycle(id)` | 閉路を辺の向きに並べた `std::span<const int>` | $$O(1)$$ |
| `cycle_length(v)` | `v` が到達する閉路の長さ | $$O(1)$$ |
| `on_cycle(v)` | `v` 自身が閉路上か | $$O(1)$$ |
| `same_component(u,v)` | 辺の向きを無視すると同じ連結成分か | $$O(1)$$ |
| `reachable_count(v)` | 自身を含む到達可能な頂点数 | $$O(1)$$ |
| `jump(v,k)` | `v` から辺を k 回たどった先 | $$O(\log n)$$ |
| `dist(u,v)` | `u` から `v` へ移動する最小回数。到達不能なら `-1` | $$O(1)$$ |
| `lca(u,v)` | 閉路上の辺を除いた逆向きの木での LCA。異なる木なら `-1` | $$O(\log n)$$ |
| `subtree(v)` | 逆向きの木の部分木が占める半開区間 `[l,r)` | $$O(1)$$ |
| `vertex_at(position)` | `subtree` と同じ順序における頂点番号 | $$O(1)$$ |
| `forest()` | 閉路上の辺を全て除いた `vector<vector<int>>` の無向森 | $$O(n)$$ |

`cycle(id)` の ID は `[0,cycle_count())` です。閉路の最後の頂点から最初の頂点へも辺があります。自己ループは長さ 1 の閉路です。戻り値は本体の配列を参照するので、本体の再構築・代入・破棄後は使わないでください。

`jump` の k は `std::uint64_t` で、0 から `UINT64_MAX` まで指定できます。負の整数は渡さないでください。k が大きくても、閉路に到達した後は長さで剰余を取って O(1) で処理します。閉路に入る前だけ HLD の経路をたどります。

`same_component(u,v)` が真でも、`u` から `v` へ到達できるとは限りません。到達できるかは `dist(u,v) != -1` で判定できます。

| 公開配列 | 内容 |
| --- | --- |
| `to[v]` | 元の辺の移動先 |
| `cycle_id[v]` | 到達する閉路の ID。連結成分の ID としても使える |
| `cycle_position[v]` | 閉路内の位置。閉路上でなければ `-1` |
| `entry[v]` | 最初に到達する閉路上の頂点。閉路上では自身 |
| `depth[v]` | 閉路に到達するまでの辺数。閉路上では 0 |
| `parent[v]` | 閉路上では `-1`、それ以外では `to[v]` |
| `subtree_size[v]` | 閉路上の辺を除いた逆向きの木での、自身を含む部分木サイズ |

公開配列は構築結果を参照するためのものです。辺を変える場合は `build` を呼び直してください。

閉路上の辺を除くと、閉路上の各頂点を根とする森になります。元の辺は子から親へ向くので、逆向きに見て通常の根付き木として扱います。`lca(u,v)` は `entry[u] == entry[v]` の場合だけその木の LCA を返します。同じ閉路へ到達しても、最初に到達する閉路上の頂点が異なれば `-1` です。

`subtree(v)` の区間は元の頂点番号の区間ではありません。`vertex_at(l)` が `v` 自身で、`[l,r)` の各位置に対応する頂点がその子孫です。`r-l == subtree_size[v]` が成り立ちます。Fenwick Tree やセグメント木と組み合わせて、流れ込む頂点への部分木クエリを処理できます。`forest()` はこの森の各辺を両方向に入れた隣接リストを返します。閉路の長さが 2 の場合も、両方向の閉路辺を両方除きます。

前処理に再帰は使いません。

頂点数を $$n$$ として、構築・`build` は $$O(n)$$ 時間・空間です。`jump`・`lca` は $$O(\log n)$$ 時間、`forest()` は $$O(n)$$ 時間・返す隣接リストに $$O(n)$$ 空間、それ以外のクエリは $$O(1)$$ 時間です。
