---
title: "厳密な直線・線分の交点・射影・距離二乗"
documentation_of: //geometry/exact_linear.hpp
library_category: geometry/exact
---

```cpp
#include "lib/geometry/exact_linear.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	Line<std::int64_t> l{ {0, 0}, {3, 1} };
	Line<std::int64_t> m{ {0, 1}, {1, 0} };
	auto p = intersection_exact(l, m);
	assert(p && p->x == Rational(3) / 4 && p->y == Rational(1) / 4);
	assert(distance_squared_exact(l, IPoint{0, 1}) == Rational(9) / 10);
	assert(on_line(Line<Rational>{to_rational(l.a), to_rational(l.b)}, *p));
}
```

`geometry` 名前空間を使います。[任意精度整数・有理数座標](geometry_rational.md) の型と Boost.Multiprecision を使用します。
点・直線・線分の座標型は `ExactCoordinate` とし、符号付き組み込み整数、`Integer`、`Rational` を渡せます。二つの引数で座標型が異なっていても使えます。
ここでは2端点が同じ直線を「退化直線」と呼び、その端点だけからなる図形として扱います。
入力を任意精度有理数に変換してから計算するため、固定幅整数の差や積がその型に収まらない入力も扱えます。許容誤差の引数はありません。

| 操作 | 内容・返り値 |
| --- | --- |
| `projection_exact(l,p)` | 点 `p` から直線 `l` へ下ろした垂線の足（射影）を `RPoint` で返す。退化直線ならその点 |
| `reflection_exact(l,p)` | 直線について `p` と対称な点を `RPoint` で返す。退化直線ならその点を中心に点対称 |
| `intersection_exact(l,m)` | 直線同士の唯一の交点を `optional<RPoint>` で返す。平行・一致直線は `nullopt` |
| `intersection_exact(s,t)` | 線分同士の共通部分を `vector<RPoint>` で返す。0点・1点・重なりの両端2点 |
| `intersection_exact(l,s)`, `intersection_exact(s,l)` | 直線と線分の共通部分を `vector<RPoint>` で返す。重なる場合は線分の両端 |
| `distance_squared_exact(a,b)` | 点・直線・線分の任意の組の最短距離二乗を `Rational` で返す。対称な引数順も使用可 |
| `perpendicular_bisector_exact(a,b)` | 2点の垂直二等分線を `optional<Line<Rational>>` で返す。同一点なら `nullopt` |

直線・線分の端点が同じ場合は点として扱います。退化直線を含む交点でも、共通部分が1点ならその点を返します。
線分同士の重なりの両端は辞書順、直線と線分の重なりは線分の `.a,.b` の順です。重なりが1点なら重複させません。
`perpendicular_bisector_exact` の返す直線は、2点の中点と、そこから辺に垂直な方向へ進んだ点で表します。

整数入力から得た `RPoint` は、次の厳密な交点・射影・距離二乗の入力にも使えます。
向きや交差の判定だけが必要なら、[通常の直線・線分](geometry_linear.md) に `RPoint`・`BigPoint` を渡しても厳密に判定できます。
距離そのものは一般に無理数となるため、ここでは距離二乗を返します。

各操作は有理数の四則演算・比較を定数回行い、定数個の数値を保持します。演算1回の時間と数値1個の保存空間は、分子・分母のビット長によります。
