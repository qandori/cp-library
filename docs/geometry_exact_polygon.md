---
title: "厳密な多角形の面積・重心・切断"
documentation_of: //geometry/exact_polygon.hpp
library_category: geometry/exact
---

```cpp
#include "lib/geometry/exact_polygon.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	std::vector<IPoint> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
	assert(area_exact(polygon) == 12);
	auto center = centroid_exact(polygon);
	assert(center && center->x == 2 && center->y == Rational(3) / 2);
	auto left = convex_cut_exact(polygon, Line<std::int64_t>{ {2, -1}, {2, 4} });
	assert(area_exact(left) == 6);
	auto corner = convex_cut_exact(left, Line<std::int64_t>{ {5, 1}, {-1, 1} });
	assert(area_exact(corner) == 2);
}
```

`geometry` 名前空間を使います。[任意精度整数・有理数座標](geometry_rational.md) の型と Boost.Multiprecision を使用します。
多角形は境界順の `vector<Point<T>>` で渡し、末尾に始点を重ねる必要はありません。座標型は `ExactCoordinate`、つまり符号付き組み込み整数型・`Integer`・`Rational` のいずれかとします。
入力を任意精度有理数に変換してから計算し、許容誤差は使いません。整数入力の中間積・和も固定幅整数の範囲に制限されません。

| 操作 | 内容・返り値 | 演算回数 |
| --- | --- | --- |
| `area_exact(polygon)` | 面積の絶対値を `Rational` で返す | $$O(n)$$ |
| `centroid_exact(polygon)` | 面積についての重心を `optional<RPoint>` で返す。面積0なら `nullopt` | $$O(n)$$ |
| `convex_cut_exact(polygon,line)` | 凸多角形を有向直線 `line.a -> line.b` の左側で切り、残る頂点列を `vector<RPoint>` で返す。直線上も残す | $$O(n)$$ |

面積・重心は単純多角形（自己交差しない多角形）を時計回り・反時計回りのどちらでも渡せます。重心は頂点の平均ではなく領域の重心です。
空・単点・線分の面積は0で、重心は `nullopt` です。自己交差の検査はしません。
符号付き面積の2倍が必要なら、`vector<RPoint>` または `vector<BigPoint>` に [signed_area2](geometry_polygon.md) を使います。

切断には凸な境界順の頂点列と、異なる2点からなる直線を渡します。多角形と直線で異なる座標型を使えます。
返り値は空・単点・線分になることもあります。隣り合う同一点と末尾の始点の重複は除きます。
返された有理数座標の頂点列は、そのまま次の切断・面積・重心の計算に使えます。

周長は一般に無理数の和なので、このヘッダーでは返しません。近似値が必要なら [perimeter](geometry_polygon.md) を使います。

頂点数を $$n$$ として、各操作は有理数の四則演算・比較を $$O(n)$$ 回行います。
切断は返り値を含め $$O(n)$$ 個、面積・重心は定数個の数値を保持します。演算1回の時間と数値1個の保存空間は、分子・分母のビット長によります。
