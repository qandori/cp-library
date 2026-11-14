---
title: "厳密な円の判定・外接円・最小包含円"
documentation_of: //geometry/exact_circle.hpp
library_category: geometry/exact
---

```cpp
#include "lib/geometry/exact_circle.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	auto a = make_exact_circle(IPoint{0, 0}, Rational(5));
	auto b = make_exact_circle(IPoint{10, 0}, Rational(5));
	assert(contains(a, IPoint{3, 4}) == Location::boundary);
	assert(relation(a, b) == CircleRelation::external_tangent);
	assert(intersects(a, Line<std::int64_t>{ {-1, 5}, {1, 5} }));
	auto c = circumcircle_exact(IPoint{0, 0}, IPoint{4, 0}, IPoint{0, 3});
	assert(c && c->center.x == 2 && c->center.y == Rational(3) / 2);
	assert(c->radius_squared == Rational(25) / 4);
}
```

`geometry` 名前空間を使います。[任意精度整数・有理数座標](geometry_rational.md) の型と Boost.Multiprecision を使用します。
円は中心と半径の二乗を任意精度有理数で保持します。半径そのものが無理数でも、二乗が有理数なら表せます。
全ての判定は有理数の演算・比較で行い、平方根や許容誤差を使いません。円周とその内部を円板と呼びます。

| 型・操作 | 内容・返り値 |
| --- | --- |
| `ExactCircle{center,radius_squared}` | 中心 `RPoint` と非負の半径二乗 `Rational` で円を構築 |
| `c.center`, `c.radius_squared` | 中心、半径の二乗 |
| `make_exact_circle(center,radius)` | `Point<T>` と非負の半径 `Rational` から `ExactCircle` を構築。`T` は `ExactCoordinate` |
| `contains(c,p)` | 点 `p` が円板の外部・円周上・内部のどこかを `Location::outside`・`boundary`・`inside` で返す |
| `relation(a,b)` | 2円の位置関係を `CircleRelation` で返す |
| `intersects(a,b)` | 2つの円周に共通点があるか。一致する円も `true` |
| `intersects(c,line)`, `intersects(line,c)` | 円周と直線に共通点があるか |
| `intersects(c,segment)`, `intersects(segment,c)` | 円周と線分に共通点があるか |
| `circumcircle_exact(a,b,c)` | 3点の外接円を `optional<ExactCircle>` で返す。3点が同一直線上なら `nullopt`。重複点がある場合も含む |
| `minimum_enclosing_circle_exact(points,seed = 712367821)` | 全点を含む最小の円を `optional<ExactCircle>` で返す。空入力なら `nullopt`。入力の頂点列は変更しない |

既定構築は原点を中心とする半径0の円です。`.radius_squared` に半径そのものを渡さないでください。
判定へ渡す点・直線・線分の座標型は `ExactCoordinate`、つまり符号付き組み込み整数型・`Integer`・`Rational` のいずれかとします。外接円の3点は同じ座標型に揃えます。
`contains` は円板に対する判定、`intersects` は円周との共通点に対する判定です。線分全体が円板の内部なら `intersects` は `false` です。
直線・線分の両端が同じ場合は点として扱い、その点が円周上にある場合だけ交差します。

`CircleRelation` は [通常の円](geometry_circle.md) と同じ列挙型です。
中心・半径二乗が一致すれば `coincident`、外接・内接すれば `external_tangent`・`internal_tangent`、異なる2点で交われば `intersecting` です。
互いに外側で離れていれば `separate`、一方が他方を内包して円周に共通点がなければ `contained` です。
半径0の円が他方の円周上にある場合は `external_tangent` とします。

最小包含円には `vector<Point<T>>` を渡し、`T` は `ExactCoordinate` とします。単点・同一点だけならその点を中心とする半径0の円を返し、全点が同一直線上でも使えます。
返す中心と半径二乗は厳密値です。`seed` はシャッフル順を決め、同じ実行環境では同じ値を指定すると再現できます。

```cpp
#include "lib/geometry/exact_circle.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	std::vector<IPoint> points{ {-2, 0}, {2, 0}, {0, 1} };
	auto bound = minimum_enclosing_circle_exact(points);
	assert(bound && bound->center == RPoint{} && bound->radius_squared == 4);
	for (const auto &p : points) assert(contains(*bound, p) != Location::outside);
}
```

一般の円交点の座標・接点・円周長・円板の面積は、整数・有理数で表せるとは限りません。このヘッダーはそれらの厳密値を返しません。
実数の近似値が必要なら [circle.hpp](geometry_circle.md) を使います。

入力点数を $$n$$ として、最小包含円は有理数の四則演算・比較をシャッフル順に対して期待 $$O(n)$$ 回、最悪 $$O(n^3)$$ 回行い、$$O(n)$$ 個の数値を保持します。
それ以外の各操作は有理数の四則演算・比較を定数回行い、定数個の数値を保持します。演算1回の時間と数値1個の保存空間は、分子・分母のビット長によります。
