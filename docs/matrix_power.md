---
title: 行列累乗・特性多項式の内部演算
documentation_of: //components/matrix_power.hpp
library_category: components/math
---

固定の素数法を持つ modint 用の行列累乗・特性多項式です。通常は [行列](matrix.md) の `Matrix<T>::pow(k)`・`characteristic_polynomial()` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `matrix_detail::FrobeniusPower<T>(a,n).run(k)` | 行優先の正方行列の累乗を `optional<vector<T>>` で返す。分解を確認できなければ `nullopt` | $$O(n^3+n^2\log k)$$ |
| `matrix_detail::FrobeniusPower<T>(a,n).characteristic()` | 特性多項式を定数項から順に返す。別の手法を選ぶ場合や分解に失敗した場合は `nullopt` | $$O(n^3)$$ |

`a` の要素数は `n*n` とし、各行を左から右へ、上の行から順に並べます。`T` は `mod()` がコンパイル時定数の素数で $$2\le\mathrm{mod}<2^{31}$$、`val()`・`raw()` と四則演算に対応する modint です。

内部で行列を計算しやすい形へ変換し、変換が正しいと確認できた場合だけ結果を返します。変換の試行回数には上限があります。

時間は $$O(n^3+n^2\log k)$$、追加空間は $$O(n^2)$$ です。この部品が `nullopt` を返しても、公開側の `Matrix<T>::pow`・`characteristic_polynomial()` は別の方法で結果を計算します。
