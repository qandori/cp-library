---
title: "整数幾何の点・ベクトル・偏角順"
documentation_of: //integer_geometry/point.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/point.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	Point a{0, 0}, b{3, 0}, c{1, 2};
	assert(cross(a, b, c) == 6 && orientation(a, b, c) == 1);
	assert(distance_squared(b, c) == 8);

	std::vector<Point> points{ {0, -1}, {-1, 0}, {2, 0}, {0, 0}, {0, 1}, {1, 0} };
	polar_sort(points);
	assert((points == std::vector<Point>{ {0, 0}, {1, 0}, {2, 0}, {0, 1}, {-1, 0}, {0, -1} }));
	std::vector<Point> around{ {9, 10}, {10, 11}, {11, 10} };
	polar_sort(around, Point{10, 10});
	assert((around.front() == Point{11, 10}));
}
```

`integer_geometry` 名前空間を使います。

| 型 | 内容 |
| --- | --- |
| `Integer` | `std::int64_t` |
| `Wide` / `wide_t<T>` | `__int128_t`。内積・外積・距離二乗などの計算型 |
| `Coordinate<T>` | `T` が64 bit以下の符号付き組み込み整数型であることを表す型の条件 |
| `BasicPoint<T>` | 座標 `T x,y` を持つ点。`T` は `Coordinate` |
| `Point` | `BasicPoint<Integer>` |
| `Location` | `outside = -1`、`boundary = 0`、`inside = 1` |

| 操作 | 内容・返り値 |
| --- | --- |
| `BasicPoint<T>()` / `BasicPoint<T>(x,y)` | 原点／指定した座標で構築 |
| `BasicPoint<T>(p)` | 異なる整数座標型の点から明示的に変換。各座標を `T` に収める |
| `p.x`, `p.y` | 座標 |
| `p == q`, `p < q` | 完全一致、x・y の辞書順 |
| `p + q`, `p - q`, `-p` | 同じ座標型の点の加減算。型 `T` を保つ |
| `p * c`, `c * p` | 座標型 `T` の整数との積。型 `T` を保つ |
| `p += q`, `p -= q`, `p *= c` | 対応する代入演算 |
| `in >> p`, `out << p` | `x y` 形式の入出力 |
| `sign(x)` | 負・0・正を `-1,0,1` で返す |
| `dot(a,b)`, `cross(a,b)` | 内積 `a.x*b.x + a.y*b.y`／外積 `a.x*b.y - a.y*b.x` を `Wide` で返す |
| `cross(a,b,c)` | `(b-a)` と `(c-a)` の外積を `Wide` で返す |
| `norm2(p)`, `distance_squared(a,b)` | 原点／2点間の距離二乗を `Wide` で返す |
| `orientation(a,b,c)` | a→b から見て c が左・同一直線・右なら `1,0,-1` |
| `ccw(a,b,c)` | 下記の5種類の位置関係 |
| `rotate90(p)` | 原点を中心に反時計回りに90度回転。型 `T` を保つ |
| `PolarLess<T>{}(a,b)` / `PolarLess<T>{center}(a,b)` | `center` を基準とする偏角順の比較 |
| `polar_sort(points,center = {})` | `vector<BasicPoint<T>>&` を偏角順に並べ替える。返り値は `void` |

点の座標型は同じ操作内で揃えます。座標・点に掛ける値には整数を渡します。整数にならない値を切り捨てないよう、点の除算は提供しません。

`ccw(a,b,c)` は、左が `1`、右が `-1`、a より後方が `2`、b より先が `-2`、線分上が `0` です。a と b が同じ場合、その点自身は `0`、それ以外は `-2` です。

偏角順は正のx方向から反時計回りで、`center` 自身を最初に置きます。同方向の点は中心からの距離順です。空列・重複点も扱えます。

原点についての `PolarLess`・`polar_sort` は、符号付き64 bit以下の座標型の全範囲で使えます。それ以外の内積・外積・距離二乗・向きの判定と、原点以外の中心についての偏角順では、全ての中間値が `Wide` に収まることが条件です。例えば点と中心の各座標の絶対値が $$10^{18}$$ 以下なら、この条件を満たします。
点の加減算・整数倍・符号反転・90度回転は、別途それぞれの結果を座標型 `T` に収めてください。

点数を $$n$$ として、`polar_sort` は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間です。それ以外の各操作の時間・追加空間は $$O(1)$$ です。
