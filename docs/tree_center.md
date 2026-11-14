---
title: 木の中心
documentation_of: //tree/tree_center.hpp
library_category: tree
---

重みなし無向木で、ほかの頂点までの距離の最大値が最小になる頂点をすべて求めます。距離はパス上の辺数です。非空の木の中心は 1 頂点または隣接する 2 頂点で、直径の中央に一致します。頂点を取り除いたときに残る成分の最大頂点数を最小にする「重心」とは異なります。

```cpp
#include "lib/tree/tree_center.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<std::vector<int>> path{ {1}, {0, 2}, {1, 3}, {2} };
	assert((tree_center(path) == std::vector<int>{1, 2}));
	std::vector<std::vector<int>> star{ {1, 2, 3}, {0}, {0}, {0} };
	assert((tree_center(star) == std::vector<int>{0}));
	assert(tree_center({}).empty());
	assert((tree_center(std::vector<std::vector<int>>(1)) == std::vector<int>{0}));
}
```

`tree_center(const std::vector<std::vector<int>>& graph)` は中心の頂点番号を昇順に並べた `std::vector<int>` を返します。引数の既定値はありません。空の木では空列、1 頂点の木では `{0}`、2 頂点の木では `{0,1}` を返します。2 個の中心を重複して返すことはありません。返り値は入力から独立しています。

頂点数を $$n$$ として、頂点番号は `[0,n)`、頂点数は `INT_MAX` 以下です。`graph[v]` に頂点 `v` の隣接頂点を並べ、各辺は両方向にそれぞれ 1 回記録してください。隣接頂点の並び順は任意です。自己辺・多重辺を持たない連結な木を渡し、非空なら無向辺数を $$n-1$$ にします。森や不正なグラフ全般を検査する API ではありません。

入力を変更せず、繰り返し呼べます。重み付きの距離や辺の途中にある中心には対応しません。

葉を層ごとに取り除き、最後に残った 1 頂点または 2 頂点を返します。再帰は使いません。計算量は $$O(n)$$ 時間、追加空間は $$O(n)$$、返り値の空間は $$O(1)$$ です。空の木は定数時間・空間です。
