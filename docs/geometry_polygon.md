---
title: "多角形の面積・内外判定・切断"
documentation_of: //geometry/polygon.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/polygon.hpp"

using namespace geometry;
std::vector<IPoint> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
auto twice = signed_area2(polygon); // 24
auto where = contains(polygon, IPoint(2, 1)); // Location::inside
auto center = centroid(polygon); // (2, 1.5)
auto left = convex_cut(polygon, Line<std::int64_t>{ {2, -1}, {2, 4} });
auto left_area = area(left); // 6
```

多角形は境界に沿った頂点列 `vector<Point<T>>` で渡します。末尾に始点を重複させる必要はありません。
通常は反時計回りを使いますが、面積・周長・重心・一般の内外判定は時計回りでも使えます。
`T` は符号付き整数または浮動小数点数とし、整数なら `IPoint`、実数なら `FPoint` が使えます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `signed_area2(polygon)` | 符号付き面積の2倍を `wide_t<T>` で返す。反時計回りが正 | $$O(n)$$ |
| `area(polygon)` | 面積の絶対値を `Real` で返す | $$O(n)$$ |
| `perimeter(polygon)` | 最後から最初への辺を含む周長を `Real` で返す | $$O(n)$$ |
| `is_convex(polygon,strict = false,eps = epsilon)` | 単純多角形（自己交差しない多角形）が凸か判定。`strict = true` では同一直線上の連続する3頂点も認めない | $$O(n)$$ |
| `contains(polygon,p,eps = epsilon)` | 一般の単純多角形に対する `Location::outside`・`boundary`・`inside` | $$O(n)$$ |
| `centroid(polygon,eps = epsilon)` | 面積についての重心を `optional<FPoint>` で返す。面積0は `nullopt` | $$O(n)$$ |
| `convex_cut(polygon,line,eps = epsilon)` | 凸多角形を有向直線 `line.a -> line.b` の左側で切り、残る頂点列を `vector<FPoint>` で返す。直線上も残す | $$O(n)$$ |

`is_convex` は入力が単純であることを前提にします。自己交差の検査はしません。0・1・2頂点は、`strict = false` なら凸、`true` なら凸でないものとします。
`contains` は頂点・辺上を `boundary` と区別します。空の多角形は常に `outside`、単点・線分には内部がなく、そこに乗る点だけが `boundary` です。

空・単点の面積と周長は0です。2点の周長は線分長の2倍です。
`centroid` は頂点の単純平均ではなく、囲まれた領域の重心です。
`convex_cut` には凸な境界順の頂点列と、異なる2点からなる直線を渡します。切断後が空・単点・線分になることもあります。

組み込み整数の符号判定と `signed_area2` は浮動小数点に変換せず計算します。計算途中の積・和は符号付き128 bitに収めてください。
`area`・`perimeter`・`centroid`・切断後の座標は実数です。浮動小数点の `eps` は絶対誤差のしきい値なので、座標の大きさに合わせて変更してください。整数では `eps` を使いません。`centroid` の面積と面積が0かどうかの判定も、整数入力なら128 bitで計算してから実数に変換します。

面積・重心・切断後の座標を厳密に保つ場合は [exact_polygon.hpp](geometry_exact_polygon.md) を使います。`area_exact`・`centroid_exact`・`convex_cut_exact` は任意精度有理数で計算します。
`signed_area2`・`contains`・`is_convex` には [rational.hpp](geometry_rational.md) の `BigPoint`・`RPoint` も渡せます。その場合、中間値も任意精度で判定に EPS は使いません。

頂点数を $$n$$ とすると、固定幅整数・実数では各操作は $$O(n)$$ 時間です。`convex_cut` は返り値を含め $$O(n)$$ 空間、それ以外は返り値を除き $$O(1)$$ 空間です。任意精度の型では、さらに各数値のビット長による演算・保存のコストがかかります。
