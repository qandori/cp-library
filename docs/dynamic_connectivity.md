---
title: "Online Dynamic Connectivity"
documentation_of: //data_structure/dynamic_connectivity.hpp
---

```cpp
#include "lib/data_structure/dynamic_connectivity.hpp"

DynamicConnectivity<> graph(5);
graph.link(0, 1);
graph.link(1, 2);
graph.cut(0, 1);
bool connected = graph.same(0, 2);
```

無向グラフを扱い、頂点番号は 0 始まりです。並行辺・自己ループにも対応します。

| 操作 | 内容 |
| --- | --- |
| `link(u, v)` | 辺を 1 本追加。連結成分が合併した場合に `true` |
| `cut(u, v)` | 辺を 1 本削除。連結成分が分かれた場合に `true`。辺がなければ何もしない |
| `same(u, v)` | 連結判定 |
| `size(v)` | 連結成分の頂点数 |
| `component_count()` / `edge_count()` | 連結成分数／並行辺を含む辺数 |
| `count(u, v)` | 2 頂点間の辺数 |
| `reserve(m)` | 同時に保持する異なる辺の本数を予約 |

連結成分の集約が必要な場合は、可換かつ結合則を満たす演算 `op` と単位元 `e()` を指定します。

```cpp
std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }

DynamicConnectivity<std::int64_t, op, e> graph(std::vector<std::int64_t>{1, 2, 3});
graph.link(0, 1);
graph.set(1, graph.get(1) + 5);
auto value = graph.prod(0);
```

`get(v)`・`set(v, value)` は頂点値の取得・更新、`prod(v)` は連結成分の集約です。頂点数だけで構築した場合、初期値は単位元です。

`n` を頂点数、`m` を同時に保持する異なる辺数の最大値として、構築は `O(n)`、`link`・`cut` は期待償却 `O(log^2 n)`、`same`・`size(v)`・`set`・`prod` は償却 `O(log n)`、`count` は期待 `O(1)`、`get`・個数取得は `O(1)` です。`reserve(k)` は期待 `O(m+k)` です。

上位レベルを初めて使う際には、1 レベルあたり別途 `O(n)` の初期化が入ります。空間は `O(m + n log n)` です。`op`・`e` と値のコピーは `O(1)` とします。
