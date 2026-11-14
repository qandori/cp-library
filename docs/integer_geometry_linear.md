---
title: "整数幾何の直線・線分・交差判定"
documentation_of: //integer_geometry/linear.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/linear.hpp"
#include <cassert>

int main() {
	using namespace integer_geometry;
	Line<> diagonal{ {0, 0}, {3, 3} };
	Segment<> a{ {0, 0}, {3, 3} }, b{ {0, 3}, {3, 0} };
	assert(on_line(diagonal, Point{5, 5}));
	assert(on_segment(a, Point{1, 1}));
	assert(!on_segment(a, Point{5, 5}));
	assert(intersects(a, b));
	assert(orthogonal(diagonal, Line<>{ {0, 0}, {1, -1} }));
}
```

`integer_geometry` 名前空間を使います。座標型 `T,U` は64 bit以下の符号付き組み込み整数型です。異なる座標型の点・直線・線分も組み合わせられます。

| 型・操作 | 内容・返り値 |
| --- | --- |
| `Line<T = Integer>{a,b}` | 2点を通る直線 |
| `Segment<T = Integer>{a,b}` | 2点を両端とし、両端を含む線分 |
| `on_line(l,p)`, `on_segment(s,p)` | 点が直線／両端を含む線分上にあるかを `bool` で返す |
| `parallel(l,m)`, `orthogonal(l,m)` | 直線の方向が平行／直交かを `bool` で返す |
| `intersects(l,m)`, `intersects(l,s)`, `intersects(s,l)`, `intersects(s,t)` | 直線・線分同士に共通点があるかを `bool` で返す |

直線・線分の `.a,.b` は端点です。`Line(a,b)`・`Segment(a,b)` では同じ型の点から座標型を推論できます。2端点が同じ場合は点として扱います。共通の端点や線分の重なりも交差に含めます。

2端点が同じ直線の方向ベクトルは零なので、`parallel`・`orthogonal` はどちらも `true` です。判定は整数のまま行い、整数座標にならない交点を構築する操作は提供しません。

全ての中間値が符号付き128 bit整数に収まる範囲で使います。例えば、各端点と問い合わせ点の座標の絶対値が $$10^{18}$$ 以下なら、このページの判定を厳密に行えます。

各操作の時間・追加空間は $$O(1)$$ です。
