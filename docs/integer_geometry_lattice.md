---
title: "整数幾何の格子点数・方向ベクトルの約分"
documentation_of: //integer_geometry/lattice.hpp
library_category: integer_geometry
---

```cpp
#include "lib/integer_geometry/lattice.hpp"
#include <cassert>
#include <vector>

int main() {
	using namespace integer_geometry;
	assert((primitive_direction(Point{6, -9}) == Point{2, -3}));
	assert(segment_lattice_points(Point{0, 0}, Point{6, 4}) == 3);
	std::vector<Point> polygon{ {0, 0}, {4, 0}, {4, 3}, {0, 3} };
	assert(boundary_lattice_points(polygon) == 14);
	assert(interior_lattice_points(polygon) == 6);
}
```

`integer_geometry` 名前空間を使います。格子点は両座標が整数の点、格子多角形は頂点が全て格子点の多角形です。このヘッダーの点・多角形には `Point` を渡します。

| 操作 | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `gcd(a,b)` | 2つの `Wide` の非負の最大公約数を `Wide` で返す。`gcd(0,0)` は0 | $$O(\log A)$$ |
| `primitive_direction(p)` | 座標を最大公約数で割った同方向の `Point`。零ベクトルなら零ベクトル | $$O(\log C)$$ |
| `segment_lattice_points(a,b)` | 両端を含む線分上の格子点数を `Wide` で返す。同一点なら1 | $$O(\log C)$$ |
| `boundary_lattice_points(polygon)` | 格子多角形の境界上の格子点数を `Wide` で返す | $$O(n\log C)$$ |
| `interior_lattice_points(polygon)` | 格子多角形の内部の格子点数を `Wide` で返す | $$O(n\log C)$$ |

多角形は境界順の `vector<Point>` とします。面積が0でなく自己交差しない多角形を時計回り・反時計回りのどちらでも渡せます。末尾での始点の反復と隣り合う同一点も認めます。自己交差の検査はしません。

空入力の境界点数は0、単点は1です。全点が同一直線上なら、その両端の間の線分に含まれる格子点数を返します。空・単点・線分・全点が同一直線上の場合、内部の格子点数は0です。

内部の点数は Pick の定理で求めます。面積を $$A$$、境界上の格子点数を $$B$$ とすると、内部の格子点数は $$A-B/2+1$$ です。

`primitive_direction`・`segment_lattice_points` は符号付き64 bit座標の全範囲を扱えます。`gcd` の引数は `Wide` の全範囲を使えますが、非負の結果が `Wide` に収まる必要があります。

多角形の判定・面積の積算値・点数を含む全ての中間値は、符号付き128 bit整数に収めてください。例えば、多角形の頂点数が `INT_MAX` 以下、各座標の絶対値が $$10^9$$ 以下なら、境界・内部の点数を厳密に計算できます。

`gcd(a,b)` では $$A=\max(\lvert a\rvert,\lvert b\rvert)$$、方向ベクトルでは成分の絶対値の最大値、線分・多角形では座標差の絶対値の最大値を $$C$$ とします。$$n$$ は多角形の頂点数です。$$A,C$$ が 0 または 1 の場合、対数の項は定数として扱います。各操作の追加空間は $$O(1)$$ です。
