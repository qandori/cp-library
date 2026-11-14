---
title: "整数幾何の円の内外・交差・位置関係"
documentation_of: //integer_geometry/circle.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/circle.hpp"
#include <cassert>

int main() {
	using namespace integer_geometry;
	Circle a{ {0, 0}, 5 }, b{ {10, 0}, 5 };
	assert(contains(a, Point{3, 4}) == Location::boundary);
	assert(relation(a, b) == CircleRelation::external_tangent);
	assert(intersects(a, Line<>{ {-1, 5}, {1, 5} }));
	assert(!intersects(a, Segment<>{ {0, 0}, {1, 0} }));
}
```

`integer_geometry` 名前空間を使います。中心座標と半径を整数で表し、平方根や許容誤差を使わずに判定します。

| 型・操作 | 内容・返り値 |
| --- | --- |
| `BasicCircle<T>{center,radius}` | 中心 `BasicPoint<T>` と非負の半径 `T` で構築 |
| `Circle` | `BasicCircle<Integer>` |
| `c.center`, `c.radius` | 中心、半径 |
| `make_circle(center,radius)` | 中心の座標型 `T` と非負の半径 `T` から `BasicCircle<T>` を返す |
| `contains(c,p)` | 円周とその内部（円板）に対する点の位置を `Location` で返す |
| `relation(a,b)` | 2円の位置関係を `CircleRelation` で返す |
| `intersects(a,b)` | 2つの円周に共通点があるかを `bool` で返す。一致する円も `true` |
| `intersects(c,line)`, `intersects(line,c)` | 円周と直線に共通点があるかを `bool` で返す |
| `intersects(c,segment)`, `intersects(segment,c)` | 円周と線分に共通点があるかを `bool` で返す |

座標型 `T` は64 bit以下の符号付き組み込み整数型です。既定構築は原点を中心とする半径0の円です。半径は常に非負にします。円・直線・線分・点の座標型は異なっていても組み合わせられます。

`contains` は円板、`intersects` は円周に対する判定です。線分全体が円板の内部なら `intersects` は `false` です。2端点が同じ直線・線分は点として扱い、その点が円周上にある場合だけ交差します。

`CircleRelation` は、一致する `coincident`、外接する `external_tangent`、内接する `internal_tangent`、異なる2点で交わる `intersecting`、互いに離れた `separate`、一方が他方を内包して円周に共通点がない `contained` です。半径0の円が他方の円周上にある場合は `external_tangent` とします。

全ての中間値が符号付き128 bit整数に収まる範囲で使います。円と直線・線分の判定には外積の二乗などを使うため、例えば各座標の絶対値と半径が $$10^9$$ 以下なら、このページの操作を厳密に行えます。

各操作の時間・追加空間は $$O(1)$$ です。
