---
title: "整数幾何の動的凸包（点追加）"
documentation_of: //integer_geometry/dynamic_convex_hull.hpp
library_category: integer_geometry
---

整数座標の点を順不同に追加し、凸包を維持します。凸包の辺上の中間点・内部点・重複点は頂点列に残しません。点の削除には対応しません。

```cpp
#include "lib/integer_geometry/dynamic_convex_hull.hpp"
#include <cassert>

int main() {
	using namespace integer_geometry;
	DynamicConvexHull<> hull;
	hull.insert({0, 0});
	hull.insert({4, 0});
	hull.insert({0, 3});
	assert(!hull.insert({1, 1}));
	assert(hull.size() == 3 && hull.doubled_area() == 12);
	assert(hull.contains({1, 1}) == Location::inside);
	assert(*hull.max_dot({1, 1}) == 4);
	assert(*hull.min_dot({1, 1}) == 0);
}
```

`integer_geometry` 名前空間を使います。`DynamicConvexHull<T = Integer>` の点・方向ベクトルは `BasicPoint<T>` で、`T` は64 bit以下の符号付き組み込み整数型です。`point_type` は `BasicPoint<T>`、`value_type` は `Wide` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicConvexHull<T>()` | 空の凸包を作る | $$O(1)$$ |
| `DynamicConvexHull(points)` | `vector<BasicPoint<T>>` の全点を追加 | $$O(n\log n)$$ |
| `insert(p)` | 点を追加。凸包が変化すれば `true`、それ以外は `false` | 償却 $$O(\log n)$$ |
| `size()` / `empty()` | 凸包の頂点数を `size_t`／空かを `bool` で返す | $$O(1)$$ |
| `vertices()` | 辞書順最小の頂点から反時計回りの `vector<BasicPoint<T>>` を返す | $$O(h)$$ |
| `contains(p)` | `Location::outside`・`boundary`・`inside` を返す | $$O(\log h)$$ |
| `support(direction)` | 内積を最大にする頂点を `optional<BasicPoint<T>>` で返す。同点は任意 | $$O(\log h)$$ |
| `max_dot(direction)` / `min_dot(direction)` | 内積の最大／最小を `optional<Wide>` で返す | $$O(\log h)$$ |
| `doubled_area()` | 面積の2倍を `Wide` で返す | $$O(1)$$ |
| `clear()` | 全点を削除して空に戻す | $$O(h)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

空の場合、`vertices()` は空配列、`contains()` は `outside`、`support()`・`max_dot()`・`min_dot()` は `nullopt`、`doubled_area()` は0です。全点が一直線上なら頂点列は両端のみになり、線分上の点は `boundary` です。1点の場合もその点のみが `boundary` です。零ベクトルを方向に使うと内積は0で、非空なら `support` は任意の頂点を返します。

内部点は保持しないため、`size()` は追加した点数ではありません。取得した頂点列は独立したコピーです。コピーした構造は独立して更新でき、移動元は空になり再利用できます。

全ての座標と方向ベクトルの各成分は絶対値を $$10^{18}$$ 以下とし、面積の積算を含む全ての中間値を符号付き128 bit整数に収めてください。

表の「償却」は、一連の追加にかかる合計時間から評価した1回当たりの時間です。`clear()` 以降の追加回数を $$n$$、現在の凸包の頂点数を $$h$$ とします。表の時間に加え、コピーは $$O(h)$$ 時間、移動は $$O(1)$$ 時間です。使用空間は $$O(h)$$、頂点列の取得には $$O(h)$$ 追加空間を使います。
