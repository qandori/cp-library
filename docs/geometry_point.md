---
title: "幾何の点・ベクトル・偏角順"
documentation_of: //geometry/point.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/point.hpp"
using namespace geometry;

IPoint a{0, 0}, b{3, 0}, c{1, 2};
int turn = orientation(a, b, c); // 1
auto squared = distance_squared(b, c); // __int128_t の 8
FPoint p = rotate(b, pi / 2);
```

`geometry` 名前空間を使います。`Point<T>` は座標 `x,y` を持ち、`T` は符号付き組み込み整数型または浮動小数点型です。`IPoint` は `Point<std::int64_t>`、`FPoint` は `Point<long double>` です。`Real` は `long double`、`pi` は円周率、`epsilon` は実数を比較するときの許容誤差の既定値 `1e-12L` です。以下では許容誤差を EPS とも書きます。

[rational.hpp](geometry_rational.md) を追加すると、任意精度整数の `BigPoint` と任意精度有理数の `RPoint` も使えます。内積・外積・距離二乗や判定を、固定幅整数の範囲を超えて厳密に扱えます。

組み込み整数の内積・外積・距離二乗は `__int128_t`、実数の場合は `Real` で計算します。この返り値型を `wide_t<T>` として使えます。`integer<T>` は組み込み整数型かを表します。
`cross(a,b,c)` と `distance_squared(a,b)` は、座標を拡張してから減算します。`Point<T>` 自体の四則演算は `T` のままで、整数の除算は0方向に丸めます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Point<T>()` / `Point<T>(x,y)` / `Point<T>(p)` | 点の構築。既定値は原点。異なる座標型からの変換は明示的 | $$O(1)$$ |
| `p.x`, `p.y` | 座標 | $$O(1)$$ |
| `p == q`, `p < q` | 完全一致、x・y の辞書順。EPS は使用しない | $$O(1)$$ |
| `p + q`, `p - q`, `-p` | ベクトルの加減算 | $$O(1)$$ |
| `p * c`, `c * p`, `p / c` | 座標型 `T` の数値との演算。除数は0以外 | $$O(1)$$ |
| `p += q`, `p -= q`, `p *= c`, `p /= c` | 対応する代入演算 | $$O(1)$$ |
| `in >> p`, `out << p` | `x y` 形式の入出力。ストリームが座標型を扱える場合 | $$O(1)$$ |
| `sign(x,eps = epsilon)` | 負・0・正を `-1,0,1` で返す。整数では EPS を無視 | $$O(1)$$ |
| `dot(a,b)` | 内積 `a.x*b.x + a.y*b.y` | $$O(1)$$ |
| `cross(a,b)` | 外積 `a.x*b.y - a.y*b.x` | $$O(1)$$ |
| `cross(a,b,c)` | `(b-a)` と `(c-a)` の外積 | $$O(1)$$ |
| `norm2(p)` | 原点からの距離二乗 | $$O(1)$$ |
| `distance_squared(a,b)` | 2点間の距離二乗 | $$O(1)$$ |
| `orientation(a,b,c,eps = epsilon)` | a→b から見て c が左・同一直線・右なら `1,0,-1` | $$O(1)$$ |
| `ccw(a,b,c,eps = epsilon)` | 下記の5種類の位置関係 | $$O(1)$$ |
| `to_real(p)` | `FPoint` に変換 | $$O(1)$$ |
| `length(p)`, `distance(a,b)` | ベクトルの長さ、2点間の距離。`Real` を返す | $$O(1)$$ |
| `almost_equal(a,b,eps = epsilon)` | 実数は各座標差が EPS 以下、整数は完全一致 | $$O(1)$$ |
| `rotate90(p)` | 原点を中心に反時計回りに90度回転。型を保つ | $$O(1)$$ |
| `argument(p)` | 偏角。ラジアン、範囲は -π から π | $$O(1)$$ |
| `angle(a,b)` | a から b への符号付き角度。両方0以外とする | $$O(1)$$ |
| `rotate(p,theta)` | 原点を中心に反時計回りに theta ラジアン回転。`FPoint` | $$O(1)$$ |
| `unit(p)` | 長さ1の `optional<FPoint>`。零ベクトルなら `nullopt` | $$O(1)$$ |
| `PolarLess<T>{}(a,b)` | 0以上2π未満の偏角順。原点が最初、同方向は距離順 | $$O(1)$$ |
| `polar_sort(points)` | `vector<Point<T>>&` を `PolarLess<T>` の順に並べ替える。返り値は `void`。空列・重複を扱える | $$O(n\log n)$$ |

`ccw(a,b,c)` は、左が `1`、右が `-1`、a より後方が `2`、b より先が `-2`、線分上が `0` です。a と b が同じ場合、その点自身は `0`、それ以外は `-2` です。

偏角は、正のx方向からベクトルに向かう角度です。正の向きは反時計回りです。偏角ソートは `polar_sort(points);` とします。比較に EPS を入れると整合的なソート順にならないため、辞書順・偏角順とも EPS は使いません。

```cpp
#include "lib/geometry/point.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	std::vector<IPoint> points{ {0, -1}, {-1, 0}, {2, 0}, {0, 0}, {0, 1}, {1, 0} };
	polar_sort(points);
	assert((points == std::vector<IPoint>{ {0, 0}, {1, 0}, {2, 0}, {0, 1}, {-1, 0}, {0, -1} }));
}
```

符号付き64 bit以下の整数座標の偏角ソート・`PolarLess` は、その座標型の全範囲で使えます。途中の並べ替えも含め、浮動小数点には変換しません。

その他の組み込み整数による操作は、全ての中間値が符号付き128 bitに収まる範囲で使います。例えば各座標の絶対値が $$10^{18}$$ 以下なら、ここでの外積・距離二乗は収まります。点同士の加減算などは、別途 `T` に収まる必要があります。実数は有限値で、中間値も有限となる範囲とします。
実数の `sign`・`orientation`・`ccw` の EPS は、比較する値に対する絶対許容誤差です。`orientation` では外積の単位になります。必要なら引数で調整し、完全な比較には `eps = 0` を指定します。浮動小数点の丸め誤差そのものがなくなるわけではありません。

固定幅整数・実数では、`polar_sort` は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間です。それ以外の各操作の時間・追加空間は $$O(1)$$ です。任意精度の型では、さらに各数値のビット長による演算・保存のコストがかかります。
