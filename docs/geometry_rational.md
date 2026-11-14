---
title: "幾何の任意精度整数・有理数座標"
documentation_of: //geometry/rational.hpp
library_category: geometry/exact
---

```cpp
#include "lib/geometry/rational.hpp"
#include <cassert>

int main() {
	using namespace geometry;
	Integer offset = Integer(1) << 100;
	BigPoint a{offset, offset}, b{offset + 3, offset}, c{offset, offset + 4};
	assert(cross(a, b, c) == 12);
	RPoint p{Rational(1) / 3, Rational(2) / 3};
	assert(norm2(p) == Rational(5) / 9);
	assert(to_rational(a).x == Rational(offset));
}
```

`geometry` 名前空間を使います。Boost.Multiprecision のヘッダーが必要で、追加のリンクライブラリは不要です。
有理数は、整数の分子・分母を持つ分数です。整数・有理数の演算と比較は、値の大きさに応じて必要な桁数を確保する任意精度で行います。固定幅整数の範囲を超える中間値や、整数にならない交点などを丸めずに扱えます。

| 型・操作 | 内容 |
| --- | --- |
| `Integer` | `boost::multiprecision::cpp_int`。任意精度の符号付き整数 |
| `Rational` | `boost::multiprecision::cpp_rational`。任意精度整数の分子・分母を持つ有理数 |
| `BigPoint` | `Point<Integer>` |
| `RPoint` | `Point<Rational>` |
| `ExactCoordinate<T>` | 符号付き組み込み整数型（`__int128_t` を含む）、`Integer`、`Rational` のいずれかかどうかを表す型の条件 |
| `to_rational(p)` | `Point<T>` を値を保った `RPoint` に変換。`T` は `ExactCoordinate` |

`Integer`・`Rational` は整数から構築でき、四則演算・比較・ストリーム入出力に対応します。除数は0以外にします。
有理数は `Rational(1) / 3` のように作ります。`1 / 3` はC++の整数除算なので0です。
`numerator(r)`・`denominator(r)` で `Rational r` の分子・分母を `Integer` として取り出せます。

点の操作は [point.hpp](geometry_point.md) と同じです。`wide_t<Integer>` は `Integer`、`wide_t<Rational>` は `Rational` で、内積・外積・距離二乗もこの型を保ちます。
向き・点の一致・偏角順などの判定は許容誤差を使いません。`BigPoint` の座標同士の除算は整数除算であり、分数を保つには先に `to_rational` で変換してください。

`RPoint` と `BigPoint` は、対応するヘッダーを追加すれば、線分の交差判定・多角形の内外判定・凸包・最近点対・最遠点対・直径・Minkowski和にも使えます。
同じ関数に渡す点の座標型は揃えます。異なる整数・有理数型を組み合わせる構築や距離二乗には、[厳密な直線・線分](geometry_exact_linear.md) を使います。
`DynamicConvexHull` は固定幅整数専用です。

`length`・`distance`・`argument`・`angle`・`rotate`・`unit`・`to_real` など実数を返す操作は近似計算です。
厳密な交点・射影・面積・重心・切断・円の判定は [exact.hpp](geometry_exact.md) から使えます。

`to_rational` は定数個の数値を変換・保持します。各演算の時間と数値1個の保存空間は、分子・分母のビット長によります。
