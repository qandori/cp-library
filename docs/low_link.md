# Low-Link

```cpp
#include "lib/graph/low_link.hpp"

LowLink graph(3);
int id = graph.add_edge(0, 1);
graph.add_edge(1, 2);
graph.build();
```

無向グラフを扱います。頂点番号は 0 始まり、`add_edge(u, v)` は辺 ID を返します。

`build()` 後に `articulation` から関節点、`bridges` から橋の辺 ID、`edges[id]` から辺の両端を取得できます。`is_bridge(id)` で橋か判定できます。辺を追加した後は `build()` を再実行します。

`n` 頂点・`m` 辺として、`build()` は時間・空間ともに `O(n+m)`、`add_edge` は償却 `O(1)`、`is_bridge` は `O(1)` です。
