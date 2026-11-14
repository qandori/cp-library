---
title: "任意精度による厳密な平面幾何"
documentation_of: //geometry/exact.hpp
library_category: geometry/exact
---

```cpp
#include "lib/geometry/exact.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	Integer offset = Integer(1) << 100;
	std::vector<BigPoint> points{ {offset, offset}, {offset + 3, offset}, {offset, offset + 4} };
	auto hull = convex_hull(points);
	assert(area_exact(hull) == 6);
	auto center = centroid_exact(hull);
	assert(center && center->x == Rational(offset) + 1);
	assert(center->y == Rational(offset) + Rational(4) / 3);
	auto circle = circumcircle_exact(points[0], points[1], points[2]);
	assert(circle && contains(*circle, points[0]) == Location::boundary);
}
```

任意精度整数・有理数による平面幾何の一括 include です。全て `geometry` 名前空間にあり、Boost.Multiprecision のヘッダーを使います。追加のリンクライブラリは不要です。
通常の [geometry.hpp](geometry.md) とは別に、厳密な計算が必要なときにこのヘッダーを選びます。個別のヘッダーだけを include することもできます。

| ヘッダー | 主な型・操作 |
| --- | --- |
| [rational.hpp](geometry_rational.md) | 任意精度整数 `Integer`・有理数 `Rational`、点 `BigPoint`・`RPoint` |
| [exact_linear.hpp](geometry_exact_linear.md) | 厳密な交点・射影・反射・距離二乗・垂直二等分線 |
| [exact_polygon.hpp](geometry_exact_polygon.md) | 厳密な面積・重心・凸多角形の切断 |
| [exact_circle.hpp](geometry_exact_circle.md) | 半径二乗で表す `ExactCircle`、円の内外・交差・位置関係、外接円・最小包含円 |
| [convex.hpp](geometry_convex.md) | 凸包・凸多角形の内外・最近点対・最遠点対・直径・Minkowski和 |

`BigPoint` を使うと、座標と中間値を任意精度整数で扱えます。分数座標も必要なら `RPoint` を使います。
交点・射影（点から直線へ下ろした垂線の足）・距離二乗・面積・重心・切断は、末尾が `_exact` の関数を使うと整数入力からでも有理数を返します。固定幅整数入力の差や積も、拡張してから計算します。
向き・交差・内外・凸包などの判定では、`BigPoint`・`RPoint` を既存の関数に渡せます。点の偏角ソートにも浮動小数点を使いません。

距離そのもの・一般の回転・円の交点座標・円周長などは、整数・有理数で厳密に表せるとは限りません。
実数を返す通常の関数は近似値を求めるためのもので、厳密版のヘッダーを include しても返り値の型は変わりません。
既存の固定幅整数専用 `DynamicConvexHull` は、各ページに記載した範囲で使います。

計算量は個別ページを参照してください。有理数・多倍長整数の四則演算や比較の回数に加え、分子・分母のビット長による各演算の時間・保存空間が必要です。
