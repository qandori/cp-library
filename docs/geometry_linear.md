---
title: "幾何の直線・線分"
documentation_of: //geometry/linear.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/linear.hpp"
using namespace geometry;

Segment<std::int64_t> s{IPoint{0, 0}, IPoint{4, 0}};
Segment<std::int64_t> t{IPoint{2, -1}, IPoint{2, 1}};
bool hit = intersects(s, t); // true
auto points = intersection(s, t); // (2,0) の1点
Real d = distance(s, IPoint{5, 1}); // sqrt(2)
```

`Line<T>{a,b}` は a・b を通る無限直線、`Segment<T>{a,b}` は両端を含む線分です。a・b は `Point<T>` です。引数同士の座標型は揃えて使います。
始点と終点が同じものは点として扱い、ここでは「退化直線」「退化線分」と呼びます。組み込み整数の交差判定は128 bitの整数で行い、交点・射影・距離は `Real` または `FPoint` で返します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Line<T>{a,b}`, `Segment<T>{a,b}` | 直線・両端を含む線分の構築。端点は `.a,.b` | $$O(1)$$ |
| `parallel(l,m,eps = epsilon)` | 方向ベクトルの外積が0か | $$O(1)$$ |
| `orthogonal(l,m,eps = epsilon)` | 方向ベクトルの内積が0か | $$O(1)$$ |
| `on_line(l,p,eps = epsilon)` | p が直線上か | $$O(1)$$ |
| `on_segment(s,p,eps = epsilon)` | p が線分上か。端点も含む | $$O(1)$$ |
| `intersects(a,b,eps = epsilon)` | 直線・線分の任意の組で共通点があるか | $$O(1)$$ |
| `projection(l,p)` | p から直線へ下ろした垂線の足（射影）。退化直線ならその点 | $$O(1)$$ |
| `reflection(l,p)` | 直線について p と対称な点。退化直線ならその点を中心に点対称 | $$O(1)$$ |
| `intersection(l,m,eps = epsilon)` | 直線同士の唯一の交点を `optional<FPoint>` で返す。平行・一致直線は `nullopt` | $$O(1)$$ |
| `intersection(s,t,eps = epsilon)` | 線分同士の共通部分を `vector<FPoint>` で返す。0点・1点・重なりの両端2点 | $$O(1)$$ |
| `intersection(l,s,eps = epsilon)`, `intersection(s,l,eps)` | 直線と線分の共通部分。重なる場合は線分の両端 | $$O(1)$$ |
| `distance(a,b)` | 点・直線・線分の任意の組の最短距離。対称な引数順も使用可 | $$O(1)$$ |
| `distance_squared(a,b)` | 同じ組の距離二乗。点同士は `wide_t<T>`、それ以外は `Real` | $$O(1)$$ |
| `perpendicular_bisector(a,b)` | 垂直二等分線を `optional<Line<Real>>` で返す。同一点なら `nullopt` | $$O(1)$$ |

線分同士の重なりの両端は辞書順です。直線と線分の重なりは線分の a・b の順です。退化した入力でも共通部分が1点なら、その点を返します。
`parallel`・`orthogonal` は方向ベクトルに対する判定なので、零ベクトルを含む場合は両方 `true` です。幾何学的な交差判定には `intersects` を使います。

EPS は、実数の比較で0とみなす差の上限（許容誤差）です。整数の場合は EPS を無視します。実数の交差判定では外積・内積に対する絶対 EPS を使い、線分の座標範囲判定には座標単位の EPS を使います。
直線同士・直線と線分・線分同士の距離では、必要なら第3引数に EPS を渡せます。点への距離・射影では EPS で位置を動かしません。
128 bitの中間値と実数精度については[点の説明](geometry_point.md)の前提に従います。
交点・射影・反射・距離二乗・垂直二等分線を丸めずに求める場合は、[exact_linear.hpp](geometry_exact_linear.md) の `_exact` 関数を使います。整数にならない結果も任意精度有理数で返します。
判定だけなら [rational.hpp](geometry_rational.md) の `BigPoint`・`RPoint` をこのヘッダーの関数に渡せます。その場合は中間値も任意精度で、EPS は使いません。

固定幅整数・実数では全ての操作の時間・追加空間は $$O(1)$$ です。任意精度の型では、さらに各数値のビット長による演算・保存のコストがかかります。
