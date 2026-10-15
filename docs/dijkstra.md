# Dijkstra

```cpp
#include "lib/graph/dijkstra.hpp"

std::vector<std::vector<std::pair<int, std::int64_t>>> g(3);
g[0].emplace_back(1, 5);
auto dist = dijkstra(g, 0);
```

辺は `{行き先, 重み}`、頂点番号は 0 始まりです。重みは非負とします。

`dijkstra(g, start, inf)` は始点からの距離を返し、到達不能は `-1` です。`inf` は扱う距離より大きい値を指定します。既定値は `min(10^18, 重みの型の最大値)` です。

`n` 頂点・`m` 辺、整数型のビット数を `w <= 64` として、整数重みでは `O(n + mw)`、その他では `O(n + m log(m))` 時間です。追加空間は `O(n + m)` です（整数幅は固定）。
