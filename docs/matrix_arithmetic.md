---
title: "行列の内部演算"
documentation_of: //components/matrix_arithmetic.hpp
---

`math/matrix.hpp` が使う連続配列の行列積・消去処理です。通常は公開側の `Matrix<T>` を使います。

| 操作 | 内容 |
| --- | --- |
| `matrix_detail::integer_determinant(a,n)` | 符号付き整数の正方行列式を Bareiss 法で求める |
| `matrix_detail::multiply(a,b,n,k,m)` | 平坦な配列の $$n\times k$$ と $$k\times m$$ を掛ける |
| `matrix_detail::field(a,f)` | 要素型に応じて内部表現へ変換し、`f.template operator()<Ops>(work)` を呼ぶ。結果を元の配列に戻す |
| `matrix_detail::eliminate<Ops>(a,h,stride,columns,reduced,eps)` | 先頭 `columns` 列を対象に消去。ピボット列と、交換の符号を含むピボットの積を返す |
| `Ops::add(a,b)` / `Ops::mul(a,b)` / `Ops::neg(a)` / `Ops::inv(a)` | 内部表現での体の演算 |
| `Ops::zero(a,eps)` / `Ops::better(a,b)` | 0 判定／ピボットの比較 |
| `Ops::scale(a,n,x)` / `Ops::axpy(a,b,n,x)` | 連続領域の定数倍／`a += x*b` |

`Field<T>` は通常の体の演算、`ModularField<T>` は固定法の modint を標準表現の `uint32_t` に変換して扱います。後者の AVX2 処理は対応する CPU でのみ呼びます。

整数の行列式は時間 $$O(n^3)$$、追加空間 $$O(n^2)$$。行列積は時間 $$O(nkm)$$、追加空間 $$O(nk+km+nm)$$。消去は時間 $$O(h\cdot\mathrm{stride}\cdot\min(h,\mathrm{columns}))$$、ピボット列の空間は $$O(\min(h,\mathrm{columns}))$$。配列の変換は時間・追加空間ともに要素数に比例し、行の定数倍・加算は時間 $$O(n)$$、追加空間 $$O(1)$$ です。要素の演算・コピーを $$O(1)$$ とします。
