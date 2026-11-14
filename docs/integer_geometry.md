---
title: "整数幾何"
documentation_of: //integer_geometry/geometry.hpp
library_category: integer_geometry
---

固定幅整数で判定・計算する平面幾何の一括 include です。通常の [幾何](geometry.md) とは別の `integer_geometry` 名前空間を使います。

```cpp
#include "lib/integer_geometry/geometry.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	std::vector<Point> points{ {0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1} };
	auto hull = convex_hull(points);
	assert(doubled_area(hull) == 24);
	assert(convex_contains(hull, Point{2, 1}) == Location::inside);
	auto far = furthest_pair(points);
	assert(far && far->squared_distance == 25);
	assert(interior_lattice_points(hull) == 6);
}
```

`Point` の座標は符号付き64 bit整数、内積・外積・距離二乗などは符号付き128 bit整数 `Wide` で扱います。任意精度整数には依存しません。

| ヘッダー | 主な操作 |
| --- | --- |
| [point.hpp](integer_geometry_point.md) | 点・ベクトル、内積・外積、向き、任意の中心についての偏角順 |
| [linear.hpp](integer_geometry_linear.md) | 直線・線分上の判定、平行・直交、交差判定 |
| [polygon.hpp](integer_geometry_polygon.md) | 面積の2倍、内外判定、凸性の判定 |
| [convex.hpp](integer_geometry_convex.md) | 凸包、凸多角形の内外、最近・最遠点対、Minkowski 和 |
| [dynamic_convex_hull.hpp](integer_geometry_dynamic_convex_hull.md) | 点の追加、面積・内外・内積を最大にする頂点の問い合わせ |
| [circle.hpp](integer_geometry_circle.md) | 整数中心・整数半径の円の内外・交差・位置関係 |
| [lattice.hpp](integer_geometry_lattice.md) | 方向ベクトルの約分、線分・多角形の格子点数 |

判定に角度や許容誤差を使いません。距離は二乗、面積は2倍の値で表し、整数のまま比較・計算できます。

座標が64 bitに収まる場合でも、外積や積算値が128 bitを超えることがあります。各ページの座標範囲と中間値の条件を満たして使ってください。原点についての偏角ソートは、符号付き64 bit座標の全範囲に対応します。

計算量と個別の条件は各ページを参照してください。
