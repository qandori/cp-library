---
title: 三角形列挙
documentation_of: //graph/enumerate_triangles.hpp
library_category: graph
---

無向単純グラフに含まれる三角形を、それぞれ1回ずつ列挙します。三角形とは、相異なる3頂点の全ての組に辺がある部分グラフです。

```cpp
#include "lib/graph/enumerate_triangles.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <utility>
#include <vector>

int main() {
	std::vector<std::pair<int, int>> edges{ {0, 1}, {2, 0}, {1, 2}, {1, 3}, {3, 2} };
	auto triangles = enumerate_triangles(5, edges);
	std::sort(triangles.begin(), triangles.end());
	assert((triangles == std::vector<std::array<int, 3>>{ {0, 1, 2}, {1, 2, 3} }));

	int count = 0;
	enumerate_triangles(5, edges, [&](int u, int v, int w) {
		assert(u < v && v < w);
		++count;
	});
	assert(count == 2);
}
```

| 関数 | 内容・返り値 |
| --- | --- |
| `enumerate_triangles(n, edges, callback)` | 各三角形について `callback(u,v,w)` を呼ぶ。返り値は `void` |
| `enumerate_triangles(n, edges)` | 三角形の一覧を `std::vector<std::array<int,3>>` で返す |

`n` は頂点数を表す `int`、`edges` は `std::span<const std::pair<int,int>>` です。`std::vector<std::pair<int,int>>` もそのまま渡せます。既定引数はありません。

頂点番号は `[0,n)` とし、頂点数・辺数はそれぞれ0以上 `INT_MAX` 以下とします。各無向辺は1回だけ渡してください。辺の両端の順序は任意ですが、自己辺と多重辺には対応しません。`(u,v)` と `(v,u)` を両方渡す場合も多重辺です。

コールバックに渡す3頂点と、返される各配列の3要素は、元の頂点番号で `u < v < w` の順です。三角形どうしの列挙順は未規定です。辞書順が必要なら、上の例のように返り値をソートしてください。

孤立点や非連結グラフも扱えます。三角形がなければコールバックは呼ばれず、配列を返す形式では空配列になります。`n = 0` と空の辺列も渡せます。

入力は変更しません。コールバックは3個の `int` で呼び出せる必要があり、その返り値は使いません。コールバックから送出した例外は呼び出し元へ伝わります。コールバック内で入力の辺列やその領域を変更しないでください。配列の返り値は入力を参照せず、独立して使えます。

頂点数を $$n$$、辺数を $$m$$、三角形の数を $$t$$ とすると、列挙は $$O(n+m\sqrt{m})$$ 時間・$$O(n+m)$$ 作業空間です。コールバック版では呼び出しにかかる時間が加わります。配列を返す形式では、結果の格納に追加で $$O(t)$$ 空間を使います。
