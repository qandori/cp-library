---
title: Prufer Code（木の符号化・復号）
documentation_of: //tree/prufer_code.hpp
library_category: tree
---

頂点番号が 0 始まりの無向木と、Prufer 符号を相互に変換します。符号化では、番号が最小の葉を取り除き、その隣接頂点を記録します。

```cpp
#include "lib/tree/prufer_code.hpp"

std::vector<std::vector<int>> graph{ {1}, {0, 2, 3}, {1}, {1} };
auto code = prufer_encode(graph); // {1, 1}
auto restored = prufer_decode(code);
auto singleton = prufer_decode({}, 1); // 頂点 0 だけの木
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prufer_encode(graph)` | 無向木の隣接リストから `vector<int>` の符号を返す。入力は変更しない | $$O(n)$$ |
| `prufer_decode(code)` | 頂点数を `code.size() + 2` とし、`vector<vector<int>>` の無向木を返す | $$O(n)$$ |
| `prufer_decode(code,n)` | 頂点数を明示して復号。0・1 頂点にも対応する | $$O(n)$$ |

`graph[v]` には頂点 `v` の隣接頂点を並べます。各辺は両方向にそれぞれ 1 回記録し、自己辺・多重辺のない連結な木を渡してください。隣接リストをソートする必要はありません。復号結果も各辺を両方向に持ち、隣接頂点の並び順は保証しません。往復変換では辺集合が一致します。

頂点数は `INT_MAX` 以下とします。$$n\geq2$$ では符号の長さは $$n-2$$、各要素は `[0,n)` です。明示した `n` と符号の長さは一致させてください。不正なグラフ全般を検査するAPIではありません。

0・1・2 頂点の符号は、いずれも空配列です。`prufer_decode({})` は 2 頂点を 1 辺で結んだ木を返します。0・1 頂点を復元する場合は `prufer_decode({},0)`・`prufer_decode({},1)` と頂点数を指定します。

頂点数を $$n$$ として、符号化・復号はともに $$O(n)$$ 時間、出力を含む $$O(n)$$ 空間です。空の木は $$O(1)$$ 時間・空間です。
