---
title: "円・接線・最小包含円"
documentation_of: //geometry/circle.hpp
library_category: geometry
---

```cpp
#include "lib/geometry/circle.hpp"
using namespace geometry;

Circle c{FPoint(0, 0), 2};
Line<Real> line{FPoint(-3, 0), FPoint(3, 0)};
auto points = intersection(c, line); // (-2,0), (2,0)

std::vector<IPoint> p{ IPoint(0, 0), IPoint(4, 0), IPoint(0, 3) };
auto bound = minimum_enclosing_circle(p);
if (bound) {
	FPoint center = bound->center; // (2,1.5)
	Real radius = bound->radius; // 2.5
}
```

`geometry` 名前空間を使います。`Circle` は中心 `center` と半径 `radius` を持ち、どちらも `Real = long double` で計算します。
中心は `FPoint`、半径は非負とします。既定構築した円は原点を中心とする半径 0 の円です。
点・直線・線分・多角形の座標には符号付き整数型も渡せますが、円の計算結果は実数です。

円の内外・位置関係・交差判定や外接円を厳密に求める場合は、[exact_circle.hpp](geometry_exact_circle.md) の `ExactCircle` を使います。中心と半径の二乗を任意精度有理数で持ち、平方根や許容誤差を使わずに判定します。一般の円交点や接点の座標は、有理数で表せるとは限りません。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Circle{center,radius}` | 中心と非負の半径を持つ円を構築 | $$O(1)$$ |
| `area(c)` | 円周とその内部（円板）の面積 | $$O(1)$$ |
| `circumference(c)` | 円周の長さ | $$O(1)$$ |
| `contains(c,p,eps = epsilon)` | 点が円板の内部・円周上・外部のどこにあるかを `Location` で返す | $$O(1)$$ |
| `relation(a,b,eps = epsilon)` | 2円の位置関係を `CircleRelation` で返す | $$O(1)$$ |
| `intersection(c,line,eps = epsilon)` | 円周と直線の交点を `vector<FPoint>` で返す。0・1・2点 | $$O(1)$$ |
| `intersection(c,segment,eps = epsilon)` | 円周と線分の交点。線分が円板の内部だけにある場合は空 | $$O(1)$$ |
| `intersection(a,b,eps = epsilon)` | 2円周の交点を `optional<vector<FPoint>>` で返す | $$O(1)$$ |
| `tangent_points(c,p,eps = epsilon)` | 点 `p` から円への接点。内部なら空、円周上なら1点、外部なら2点 | $$O(1)$$ |
| `common_tangents(a,b,eps = epsilon)` | 共通接線を `optional<vector<Line<Real>>>` で返す。有限なら0～4本、無限なら `nullopt` | $$O(1)$$ |
| `circumcircle(a,b,c,eps = epsilon)` | 三角形の外接円を `optional<Circle>` で返す。3点がほぼ同一直線上なら `nullopt` | $$O(1)$$ |
| `incircle(a,b,c,eps = epsilon)` | 三角形の内接円を `optional<Circle>` で返す。3点がほぼ同一直線上なら `nullopt` | $$O(1)$$ |
| `overlap_area(a,b)` | 2つの円板の共通部分の面積 | $$O(1)$$ |
| `intersection_area(c,polygon)` | 円板と単純多角形（自己交差しない多角形）の共通部分の面積。頂点は外周順、凸でなくてもよく、時計回りでもよい | $$O(n)$$ |
| `minimum_enclosing_circle(points,seed = 712367821,eps = epsilon)` | 全点を含む最小の円を `optional<Circle>` で返す。空入力は `nullopt`。入力配列は変更しない | 期待 $$O(n)$$、最悪 $$O(n^3)$$ |

`Location::outside = -1`、`boundary = 0`、`inside = 1` です。
`contains` は円板に対する判定ですが、`intersection` が求めるのは円周との交点です。
点を返す関数は座標の辞書順で返し、接する場合は同じ点を重複させません。

| `CircleRelation` | 状態 |
| --- | --- |
| `separate` | 互いに外側で離れている |
| `external_tangent` | 外接する |
| `intersecting` | 円周が異なる2点で交わる |
| `internal_tangent` | 内接する |
| `contained` | 一方が他方を内包し、円周に共通点がない |
| `coincident` | 中心・半径が許容誤差内で一致する |

2円の `intersection` は、一致扱いで少なくとも一方の半径が正なら `nullopt` です。
交点がない場合は、値を持つ空の配列を返します。半径 0 の円同士が一致する場合は1点を返します。
半径 0 の円が正の半径の円周上にある場合、位置関係は `external_tangent` とします。

共通接線は、その直線上の異なる2点で表し、接線同士の順序は定めません。
一致する円は半径 0 でも無限本として `nullopt` を返します。異なる半径 0 の円2つは、その2点を通る1本を返します。
一方だけの半径が 0 なら、その点から他方の円への接線になります。
`tangent_points` では半径 0 の円の接点は常にその中心1点です。
直線・線分の端点が同じ場合は点として扱います。

`epsilon` の既定値は $$10^{-12}$$ です。`contains`・円の位置関係・交点・接線では、長さ単位の許容誤差として使います。
三角形がほぼ同一直線上にある場合、外接円・内接円は `nullopt` になります。
ソートには許容誤差を使いません。入力・中間計算が有限となる範囲で使い、座標や半径のスケール、要求される誤差に応じて `eps` を指定してください。
接する直前の交点や、ほぼ一直線上の3点の外接円は数値的に不安定です。

最小包含円の `seed` は、点をランダムに並べ替える順序を決めます。その順序に対する期待時間が線形で、同じ実行環境では同じ `seed` を指定すると再現できます。
同じ点だけなら半径 0、全点が一直線上でも処理できます。最後に全点との距離を確認して半径を補正します。
得られる中心・半径は実数の近似値で、点が円周上にあるかの厳密な判定には別の判定が必要です。

多角形の頂点数・入力点数を $$n$$ として、円と多角形の共通面積は時間 $$O(n)$$・追加空間 $$O(1)$$、最小包含円は期待時間 $$O(n)$$・最悪時間 $$O(n^3)$$・追加空間 $$O(n)$$ です。
それ以外は時間・追加空間ともに $$O(1)$$ です。
