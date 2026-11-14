---
title: "平面幾何"
documentation_of: //geometry/geometry.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/geometry.hpp"
using namespace geometry;

std::vector<IPoint> points{IPoint{0, 0}, IPoint{3, 0}, IPoint{0, 4}};
auto hull = convex_hull(points);
Real a = area(hull); // 6
auto nearest = closest_pair(points);
auto circle = minimum_enclosing_circle(points);
```

必要なヘッダーだけ個別に include することもできます。全て `geometry` 名前空間です。
`IPoint` で整数判定、`FPoint` で実数の構築・計算を行います。既定の実数型は `long double` で、その精度は環境に依存します。

交点・射影（点から直線へ下ろした垂線の足）・面積・重心なども丸めずに計算する場合は、[exact.hpp](geometry_exact.md) を追加します。任意精度整数 `BigPoint` と任意精度有理数 `RPoint`、厳密な構築・計算の `_exact` 関数を使えます。

| ヘッダー | 主な操作 | 計算量 |
| --- | --- | --- |
| [point.hpp](geometry_point.md) | 点・ベクトル、内積・外積、向き、角度、偏角順 | 点の操作は $$O(1)$$、偏角ソートは $$O(n\log n)$$ |
| [linear.hpp](geometry_linear.md) | 直線・線分の交差、交点、距離、射影・反射 | 各操作 $$O(1)$$ |
| [polygon.hpp](geometry_polygon.md) | 面積、凸性、一般多角形の内外、半平面による切断 | 各操作 $$O(n)$$ |
| [convex.hpp](geometry_convex.md) | 凸包、凸多角形内外、直径、Minkowski和、最近・最遠点対 | 操作ごとに個別ページを参照 |
| [dynamic_convex_hull.hpp](dynamic_convex_hull.md) | 点追加による凸包の更新、面積・内外・内積の問い合わせ | 操作ごとに個別ページを参照 |
| [circle.hpp](geometry_circle.md) | 円の交点・接線、内外接円、共通部分の面積、最小包含円 | 操作ごとに個別ページを参照 |

表の $$n$$ は入力点数または多角形の頂点数です。点の内外判定は `Location::outside`・`boundary`・`inside` の3種類を返します。円は円周の交点と、円板の共通部分面積を区別します。
空・単点・同一点の入力での戻り値や、凸多角形の向き・頂点の並びの前提は各ページに記載しています。

組み込み整数でも、全ての中間値が符号付き128 bitに収まることが前提です。特に多角形の面積や点の加減算では、積だけでなく合計・座標型の範囲にも注意します。偏角ソートは符号付き64 bit以下の座標型の全範囲で使えます。
実数を比較するときに0とみなす差の上限（許容誤差、EPS）は、関数ごとに引数で変更します。点・直線・多角形の整数による判定では無視し、ソートにも EPS を使いません。通常の `Circle` は整数座標を渡しても実数で計算します。非常に大きい座標やほぼ同一直線・接する配置では、入力のスケールに応じて精度・許容誤差を選びます。

操作ごとの時間・追加空間は個別ページを参照してください。
