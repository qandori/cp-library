---
title: "整数幾何の凸包・最近点対・最遠点対"
documentation_of: //integer_geometry/convex.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/convex.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	std::vector<Point> points{ {0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1} };
	auto hull = convex_hull(points);
	assert(hull.size() == 4);
	assert(convex_contains(hull, Point{2, 1}) == Location::inside);
	auto near = closest_pair(points);
	auto far = furthest_pair(points);
	assert(near && near->squared_distance == 2);
	assert(far && far->squared_distance == 25);
	auto sum = minkowski_sum(hull, std::vector<Point>{ {0, 0}, {1, 0} });
	assert((sum.front() == Point{0, 0}));
}
```

`integer_geometry` 名前空間を使います。点群は `vector<BasicPoint<T>>` とし、`T` は64 bit以下の符号付き組み込み整数型です。外積や距離二乗は符号付き128 bit整数 `Wide` で計算します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convex_hull_indices(points,keep_collinear = false)` | 凸包上の点の元の添字を `vector<int>` で返す | $$O(n\log n)$$ |
| `convex_hull(points,keep_collinear = false)` | 凸包の頂点列を返す | $$O(n\log n)$$ |
| `convex_contains(convex,p)` | 凸多角形に対する点の位置を `Location` で返す | $$O(\log h)$$ |
| `convex_diameter(convex)` | 最遠頂点対を `optional<PointPair<T>>` で返す。添字は入力の凸多角形内 | $$O(h)$$ |
| `diameter_squared(convex)` | 凸多角形の直径二乗を `Wide` で返す。空・単点は0 | $$O(h)$$ |
| `furthest_pair(points)` | 一般の点群の最遠点対を `optional<PointPair<T>>` で返す | $$O(n\log n)$$ |
| `closest_pair(points)` | 一般の点群の最近点対を `optional<PointPair<T>>` で返す | $$O(n\log n)$$ |
| `minkowski_sum(a,b)` | 2つの凸多角形の Minkowski 和を頂点列で返す | $$O(n+m)$$ |
| `PointPair<T>::first`, `.second` | 点対の0始まりの添字 `int` | $$O(1)$$ |
| `PointPair<T>::squared_distance` | 点対の距離二乗 `Wide` | $$O(1)$$ |

元の点群の添字は `[0,n)`、凸多角形内の添字は `[0,h)` です。凸包・点対を求める操作では入力点数を `INT_MAX` 以下にします。同距離の候補が複数あれば任意の1組を返します。

凸包は辞書順最小の点から始まり、反時計回りに並びます。始点を末尾に再び追加しません。重複座標は1点にまとめ、返す添字はその座標を持ついずれか1点です。
`keep_collinear = false` では辺の途中の点を除き、`true` では残します。全点が同一直線上なら、既定では両端のみ、`true` では全ての異なる点を辞書順に返します。全点が同一なら1点、空入力なら空配列です。

`convex_contains`・`convex_diameter`・`diameter_squared` には、3頂点以上なら、同一直線上の連続3頂点を含まない反時計回りの凸多角形を渡します。先頭の頂点は任意です。`convex_hull(points)` の返り値をそのまま使えます。凸性の検査は省略します。0・1・2頂点も扱えます。

`closest_pair`・`furthest_pair` は2点未満なら `nullopt` を返します。2点以上なら、全点が同一でも異なる添字を返します。
`convex_diameter` は空なら `nullopt`、単点なら `first = second = 0`、距離0を返します。

Minkowski 和は、`a` の各点と `b` の各点を足して得られる点群の凸包です。入力は凸な境界順とし、時計回り・辺上の余分な点・末尾の始点の重複を認めます。返り値は余分な点を除いた反時計回りの凸包です。空集合との和は空、単点との和は平行移動です。非凸な点群は先に `convex_hull` に通してください。出力の座標の和は座標型 `T` に収めます。

全ての中間値を符号付き128 bit整数に収めて使います。凸包・点対・凸多角形の内外判定では、例えば各座標の絶対値が $$10^{18}$$ 以下なら十分です。Minkowski 和では面積の積算も含めて条件を満たす必要があり、例えば各入力の点数が `INT_MAX` 以下、各座標の絶対値が $$10^9$$ 以下なら十分です。

凸包・一般の点対は $$O(n)$$、凸多角形の直径は $$O(h)$$、Minkowski 和は $$O(n+m)$$ 追加空間です。凸多角形の内外判定は $$O(1)$$ 追加空間です。
