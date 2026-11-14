---
title: "整数幾何の多角形・面積・内外判定"
documentation_of: //integer_geometry/polygon.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/polygon.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	std::vector<Point> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
	assert(signed_area2(polygon) == 24 && doubled_area(polygon) == 24);
	assert(contains(polygon, Point{2, 1}) == Location::inside);
	assert(contains(polygon, Point{4, 1}) == Location::boundary);
	assert(is_convex(polygon, true));
}
```

`integer_geometry` 名前空間を使います。多角形は境界順の `vector<BasicPoint<T>>` で渡し、末尾に始点を重ねる必要はありません。`T` は64 bit以下の符号付き組み込み整数型です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `signed_area2(polygon)` | 符号付き面積の2倍を `Wide` で返す。反時計回りが正 | $$O(n)$$ |
| `doubled_area(polygon)` | 面積の2倍の絶対値を `Wide` で返す | $$O(n)$$ |
| `contains(polygon,p)` | 点の位置を `Location::outside`・`boundary`・`inside` で返す。点と多角形の座標型は異なっていても可 | $$O(n)$$ |
| `is_convex(polygon,strict = false)` | 境界順の頂点列が凸かを `bool` で返す。`strict` が真なら辺上の余分な頂点を認めない | $$O(n)$$ |

面積・内外判定には単純多角形（自己交差しない多角形）を時計回り・反時計回りのどちらでも渡せます。自己交差の検査はしません。
空・単点・線分の面積は0です。空集合の内外判定は `outside`、単点・線分ではその上の点だけが `boundary` です。`is_convex` は3頂点未満では `!strict` を返します。

面積が整数にならない場合も丸めず扱うため、面積そのものではなく2倍の値を返します。

全ての中間値と面積の積算値を符号付き128 bit整数に収めてください。例えば、頂点数が `INT_MAX` 以下、各頂点と問い合わせ点の座標の絶対値が $$10^9$$ 以下なら、このページの操作を厳密に行えます。

頂点数を $$n$$ として各操作は $$O(n)$$ 時間です。各操作の追加空間は $$O(1)$$ です。
