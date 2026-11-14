---
title: Kd-tree（最近傍・矩形範囲検索）
documentation_of: //data_structure/kd_tree.hpp
library_category: data_structure/rectangle_query
---

静的な点集合から最近傍、距離順の上位 `k` 点、軸に平行な矩形内の点を検索します。次元はコンパイル時に指定し、既定は 2 次元です。

```cpp
#include "lib/data_structure/kd_tree.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

int main() {
	using Tree = KDTree<>;
	std::vector<Tree::point_type> points{ {0, 0}, {2, 0}, {2, 0}, {5, 4} };
	Tree tree(points);

	auto near = tree.nearest({1, 0});
	assert(near && near->index == 0 && near->distance_squared == 1);
	auto other = tree.nearest(points[1], 1);
	assert(other && other->index == 2 && other->distance_squared == 0);

	auto closest = tree.k_nearest({1, 0}, 3);
	assert(closest.size() == 3);
	assert(closest[0].index == 0 && closest[1].index == 1 && closest[2].index == 2);

	auto ids = tree.range_report({0, -1}, {3, 1});
	std::sort(ids.begin(), ids.end());
	assert((ids == std::vector<int>{0, 1, 2}));
	assert(tree.range_count({0, -1}, {2, 1}) == 1);
	int count = 0;
	tree.range_visit({0, -1}, {3, 1}, [&](int id) {
		assert(tree.point(id)[1] == 0);
		++count;
	});
	assert(count == 3);
}
```

`KDTree<T = std::int64_t, Dimension = 2, Distance = ...>` を使います。`T` は座標型、`Dimension` は正の `int`、`Distance` は距離二乗の型です。`Distance` の既定値は整数座標では `__int128_t`、浮動小数点座標では `T` です。

`point_type` は `std::array<T, Dimension>`、`distance_type` は `Distance` です。`neighbor_type` は `int index` と `Distance distance_squared` を持ち、等値比較もできます。`distance_squared` は各軸の座標差の二乗を合計した値で、ユークリッド距離の二乗に当たります。平方根は取りません。

点の番号は構築時の入力順に `[0,size())` です。同じ座標の点も別々の番号で保持します。内部で点を並べ替えても番号は変わりません。入力はコピーして保持するため、構築後に元の配列を変更・破棄できます。

| 操作 | 内容・返り値 |
| --- | --- |
| `KDTree()` | 空の木。葉の上限点数は 16 |
| `KDTree(points, leaf_size = 16)` | `std::span<const point_type>` から構築。`vector<point_type>` や `array<point_type,N>` も渡せる |
| `build(points, leaf_size = 16)` | 点集合を置き換えて構築し直す。番号も入力順に振り直す。返り値は `void` |
| `size()` / `empty()` | 点数を `int`／空かを `bool` で返す |
| `leaf_size()` | 葉に保持する点数の上限を `int` で返す |
| `point(index)` | 入力番号 `index` の座標を `const point_type&` で返す。番号は `[0,size())` |
| `nearest(query, exclude = -1)` | 最近傍を `std::optional<neighbor_type>` で返す。対象点がなければ `std::nullopt` |
| `k_nearest(query, k, exclude = -1)` | 距離二乗と番号の昇順に、最大 `k` 点を `std::vector<neighbor_type>` で返す |
| `range_count(lower, upper)` | 各軸の半開区間 `[lower[d],upper[d])` に入る点数を `int` で返す |
| `range_report(lower, upper)` | 同じ半開矩形に入る点の番号を `std::vector<int>` で返す。順序は未規定 |
| `range_visit(lower, upper, visit)` | 同じ半開矩形の各点について `visit(int index)` を 1 回ずつ呼ぶ。返り値は `void`、順序は未規定 |

点数は `INT_MAX` 以下、`leaf_size` は正の `int` とします。空の入力も構築できます。`build` で `leaf_size` を省略すると、前回の設定によらず 16 になります。葉を大きくすると木のノード数が減り、葉で直接比較する点数が増えます。

`query`・`lower`・`upper` は `point_type` です。`nearest` と `k_nearest` は、距離二乗が同じなら入力番号の小さい点を優先します。`exclude` は `-1` または `[0,size())` の `int` で、指定した番号の 1 点だけを除外します。同じ座標にある他の点は検索対象です。`k` は非負の `int` で、対象点数より大きい場合は全対象点を返します。`k == 0` または対象点がない場合は空の列です。

矩形は全軸で下端を含み、上端を含みません。どれかの軸で `lower[d] >= upper[d]` なら空として扱います。該当する点がなければ `range_count` は 0、`range_report` は空の列、`range_visit` は呼び出しなしになります。`range_visit` のコールバックでは木を変更しないでください。番号順が必要なら、取得した番号列をソートします。

検索と取得は `const` な木から呼び出せます。点の追加・削除・座標変更は `build` で点集合全体を置き換えます。コピーした木は独立して使えます。`point` が返す参照は、再構築・代入・破棄で無効になります。

座標には `bool` を除く 64 bit 以下の符号付き・符号なし整数、または組み込み浮動小数点型を使います。整数座標では `Distance` も `bool` を除く 128 bit 以下の整数型、浮動小数点座標では `T` 以上の精度を持つ浮動小数点型を指定します。

整数では、問い合わせ点と各登録点の座標差の二乗、およびその全軸の和が `Distance` に収まる必要があります。減算は拡張した型で行いますが、距離二乗のオーバーフローは検出しません。既定の 128 bit 距離は 2 次元の 32 bit 整数座標の全範囲を扱えます。2 次元で登録点・問い合わせ点の各座標の絶対値が $$10^{18}$$ 以下なら、64 bit 座標と既定の距離型で収まります。距離の範囲が既知なら、例えば `KDTree<std::int64_t, 2, std::int64_t>` として距離型を狭くできます。この場合、2 次元で各座標の絶対値が $$10^9$$ 以下なら距離二乗は収まります。

浮動小数点では、登録点と最近傍の問い合わせ点の座標は有限値にします。座標差・距離二乗などの中間値も有限となる範囲を使ってください。矩形の上下端には正負の無限大も使えますが、NaN は扱いません。比較では許容誤差を設けず、浮動小数点型に丸めた値をそのまま比較します。

3 次元の浮動小数点座標でも同じ API を使えます。

```cpp
#include "lib/data_structure/kd_tree.hpp"
#include <cassert>
#include <limits>
#include <vector>

int main() {
	using Tree = KDTree<double, 3>;
	std::vector<Tree::point_type> points{ {1.0, 2.0, 3.0}, {-1.0, 0.0, 0.0} };
	Tree tree(points, 8);
	auto near = tree.nearest({0.0, 0.0, 0.0});
	assert(near && near->index == 1 && near->distance_squared == 1.0);
	double inf = std::numeric_limits<double>::infinity();
	assert(tree.range_count({-inf, -inf, -inf}, {inf, inf, inf}) == 2);
}
```

座標の広がりが最も大きい軸で中央値に分割し、点数が釣り合う木を作ります。点とノードを連続した配列に保持し、葉では点を直接比較します。部分木の外接矩形と最小の入力番号を使って、より良い答えを含まない部分木を枝刈りします。矩形全体が検索範囲に入る部分木は、個数をまとめて数えられます。

参考: Jon Louis Bentley, [Multidimensional binary search trees used for associative searching](https://doi.org/10.1145/361002.361007), Communications of the ACM, 1975.

点数を $$n$$、次元を $$D$$、返す近傍点数を $$k$$、矩形に入る点数を $$z$$ とします。`std::nth_element` による構築・`build` は平均 $$O(Dn\log n)$$ 時間で、保持空間は $$O(Dn)$$ です。葉の上限点数を超えない場合の構築は $$O(Dn)$$ です。

最近傍と矩形の個数取得は最悪 $$O(Dn)$$、矩形の列挙・コールバックは最悪 $$O(Dn+z)$$ 時間です。`k_nearest` は $$k\ge2$$ で最悪 $$O(Dn+n\log k)$$、$$k=1$$ では最近傍と同じです。次元を固定すると、空入力の検索と `k == 0` は定数時間です。軸を広がりで選ぶため、矩形検索に $$O(n^{1-1/D}+z)$$ の最悪計算量は保証しません。

検索の作業領域は再帰スタックに $$O(\log n)$$、`k_nearest` では候補ヒープにさらに $$O(k)$$ を使います。返す列の領域は別途 $$O(k)$$ または $$O(z)$$ です。`size`・`empty`・`leaf_size`・`point` は $$O(1)$$ 時間です。空集合・点が 1 個の場合の管理操作は定数時間です。座標の演算とコールバック 1 回を定数時間とします。
