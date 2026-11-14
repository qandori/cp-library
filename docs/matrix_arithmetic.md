---
title: 行列の内部演算
documentation_of: //components/matrix_arithmetic.hpp
library_category: components/math
---

`math/matrix.hpp` が使う連続配列の行列積・消去処理です。通常は公開側の `Matrix<T>` を使います。体の演算・内積・配列の定数倍と加算は [体の内部演算](field_arithmetic.md) と共有しています。

ここでいう「体」は 0 以外の値での除算ができる型、「ピボット」は消去法で各行の計算の基準に選ぶ 0 でない成分です。配列は各行を左から右へ、上の行から順に並べます。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `matrix_detail::integer_determinant(a,n)` | 符号付き整数の正方行列式を Bareiss 法で求める | $$O(n^3)$$ |
| `matrix_detail::multiply(a,b,n,k,m)` | 平坦な配列の $$n\times k$$ と $$k\times m$$ を掛ける | $$O(nkm)$$ |
| `matrix_detail::field(a,f)` | 要素型に応じて内部表現へ変換し、`f.template operator()<Ops>(work)` を呼ぶ。結果を元の配列に戻す | 要素数に比例＋f の処理時間 |
| `matrix_detail::eliminate<Ops>(a,h,stride,columns,reduced,eps)` | 先頭 `columns` 列を対象に消去。ピボット列と、交換の符号を含むピボットの積を返す | $$O(h\cdot\mathrm{stride}\cdot\min(h,\mathrm{columns}))$$ |
| `Ops::add(a,b)` / `Ops::mul(a,b)` / `Ops::neg(a)` / `Ops::inv(a)` | 内部表現での体の演算 | $$O(1)$$ |
| `Ops::zero(a,eps)` / `Ops::better(a,b)` | 0 判定／ピボットの比較 | $$O(1)$$ |
| `Ops::scale(a,n,x)` / `Ops::axpy(a,b,n,x)` | 連続領域の定数倍／`a += x*b` | $$O(n)$$ |

`Field<T>` は型 `T` 自身の四則演算を使い、`ModularField<T>` は固定法の modint を 0 以上法未満の `uint32_t` に変換して扱います。後者の AVX2・AVX512 処理は対応する CPU でのみ呼びます。

行列積は対応 CPU で AVX512 を使い、非対応なら AVX2 または通常処理を使います。剰余は整数で補正するため、浮動小数点の近似商を使う経路でも結果は厳密です。`MATRIX_DISABLE_AVX2` を定義すると両方の SIMD 経路を無効にできます。

整数の行列式は時間 $$O(n^3)$$、追加空間 $$O(n^2)$$。行列積は時間 $$O(nkm)$$、追加空間 $$O(nk+km+nm)$$。消去は時間 $$O(h\cdot\mathrm{stride}\cdot\min(h,\mathrm{columns}))$$、ピボット列の空間は $$O(\min(h,\mathrm{columns}))$$。配列の変換は時間・追加空間ともに要素数に比例し、行の定数倍・加算は時間 $$O(n)$$、追加空間 $$O(1)$$ です。要素の演算・コピーを $$O(1)$$ とします。
