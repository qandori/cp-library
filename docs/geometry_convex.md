---
title: "凸包・凸多角形・最近点対・最遠点対"
documentation_of: //geometry/convex.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/convex.hpp"

using namespace geometry;
std::vector<IPoint> points{ {0, 0}, {4, 0}, {4, 3}, {0, 3}, {1, 1} };
auto hull = convex_hull(points);
auto where = convex_contains(hull, IPoint(2, 1)); // Location::inside
auto near = closest_pair(points);
if (near) {
    auto p = points[near->first];
    auto q = points[near->second];
    auto squared = near->squared_distance;
}
auto far = furthest_pair(points);
auto sum = minkowski_sum(hull, std::vector<IPoint>{ {0, 0}, {1, 0} });
```

点群には `vector<Point<T>>` を渡します。整数なら `IPoint`、実数なら `FPoint` を使えます。
[rational.hpp](geometry_rational.md) を追加すると、`BigPoint`・`RPoint` による任意精度整数・有理数の点群も同じ操作で扱えます。整数にならない切断後の頂点列にも使えます。
`convex_hull` が返す境界には内点・重複点を含めず、始点を末尾に再び追加しません。
左端、同じ横座標なら最も下の頂点から始まり、反時計回りに並びます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convex_hull_indices(points,keep_collinear = false,eps = epsilon)` | 凸包上の点の元の添字を `vector<int>` で返す。同じ座標が複数あれば、そのうちの1点を使う | $$O(n\log n)$$ |
| `convex_hull(points,keep_collinear = false,eps = epsilon)` | 凸包の頂点列を返す | $$O(n\log n)$$ |
| `convex_contains(convex,p,eps = epsilon)` | 辺上の中間点を除いた反時計回りの凸多角形の頂点列に対する `Location::outside`・`boundary`・`inside` | $$O(\log h)$$ |
| `convex_diameter(convex,eps = epsilon)` | 辺上の中間点を除いた反時計回りの凸多角形の頂点列の最遠頂点対を `optional<PointPair<T>>` で返す。添字は渡した頂点列内 | $$O(h)$$ |
| `diameter_squared(convex,eps = epsilon)` | 同じ凸多角形の直径の2乗を `wide_t<T>` で返す。空・単点は0 | $$O(h)$$ |
| `furthest_pair(points,eps = epsilon)` | 一般の点群の最遠点対。添字は元の点群内 | $$O(n\log n)$$ |
| `closest_pair(points)` | 一般の点群の最近点対。添字は元の点群内。重複点があれば距離0の異なる添字を返す | $$O(n\log n)$$ |
| `minkowski_sum(a,b)` | 2つの凸多角形の Minkowski 和の頂点列を返す | $$O(n+m)$$ |
| `PointPair<T>::first / second` | 点対の添字 `int`。同距離の候補が複数あれば任意の1組 | $$O(1)$$ |
| `PointPair<T>::squared_distance` | 点対の距離の2乗。整数なら符号付き128 bit、実数なら `Real` | $$O(1)$$ |

`keep_collinear = false` では、凸包の辺の途中の点を除きます。`true` なら辺上の点も残します。
全点が同一直線上の場合は、既定では両端のみ、`true` では全ての異なる座標を辞書順に返します。全点が同一なら1点、入力が空なら空配列です。

`convex_contains` と `convex_diameter` には、3頂点以上なら、同一直線上の連続する3頂点を含まない反時計回りの凸多角形を渡します。
前処理として `convex_hull(points)` を使えばこの形になります。`convex_hull(points,true)` の結果をそのまま渡せるとは限りません。
凸であることの検査は省略し、先頭の頂点は任意です。0・1・2頂点もそのまま扱えます。

`closest_pair`・`furthest_pair` は2点未満なら `nullopt` を返します。
`convex_diameter` は空なら `nullopt`、単点なら `first = second = 0`、距離0を返します。
2点以上の一般点群では、全て同じ座標でも、異なる添字の点対を返します。

Minkowski 和は、`a` の各点と `b` の各点を足して得られる点群の凸包です。
入力は凸な境界順であれば時計回り・辺上の余分な点・末尾の始点の重複を認めます。空集合との和は空、単点との和は平行移動です。
返り値は辺上の余分な点を除いた反時計回りの凸包で、始点の選び方は `convex_hull` と同じです。
各入力が凸かどうかは検査しません。非凸な点群は、先にそれぞれ `convex_hull` に通します。出力座標の和も `T` に収めてください。

組み込み整数では符号判定・距離の2乗を厳密計算し、途中の積・和を符号付き128 bitに収めます。
`BigPoint`・`RPoint` では途中の積・和も任意精度で、固定幅整数の範囲には制限されません。
ソートと重複点の除去には、浮動小数点でも誤差しきい値を使いません。Minkowski 和の向き・辺方向の比較と、辺上の点の除去も0との比較で行い、小さい凸多角形もそのまま扱います。
その他の実数の判定には `eps` を使いますが、最近点対には使わず距離の大小をそのまま比較します。
座標は有限値とし、実数演算の途中の値も有限に収めてください。

点群の大きさを $$n$$、凸多角形の頂点数を $$h$$ とします。
凸包・一般の最近点対・最遠点対は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間、凸多角形の内外判定は $$O(\log h)$$ 時間・$$O(1)$$ 追加空間、直径は $$O(h)$$ 時間・$$O(1)$$ 追加空間です。
Minkowski 和は入力の大きさを $$n,m$$ として $$O(n+m)$$ 時間・空間です。空・単点の判定は $$O(1)$$ 時間です。
これらの計算量は座標の四則演算・比較を1回として数えます。任意精度の型では、さらに各数値のビット長による演算・保存のコストがかかります。
