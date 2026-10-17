---
title: "多項式 Half-GCD"
documentation_of: //components/polynomial_half_gcd.hpp
---

`fps_detail::PolynomialHalfGCD` は、多項式 GCD・拡張 GCD で共有する内部処理です。通常は [多項式 GCD](https://qandori.github.io/cp-library/fps/polynomial_gcd.hpp.html) のヘッダーを使います。係数型は内部の `fps_detail::Mint`、法は 998244353 です。

| 型・関数 | 内容・返り値 |
| --- | --- |
| `T` / `F` / `O` | 係数型／FPS 型／共通 FPS 演算 |
| `Matrix` | 多項式を要素とする 2×2 行列。初期値は単位行列 |
| `Matrix::a` | 成分を行優先で並べた配列 |
| `Matrix::shift` | 互除法の商の次数の総和。`half` では先頭多項式の次数の低下量に一致 |
| `Matrix::identity` | 単位行列の省略処理を使えるか |
| `high(a,k)` | `a` の先頭 `k` 係数を除いた多項式。`k >= 0` |
| `fold(a,n)` | `a mod (x^n-1)` を長さ `n` の配列にする。`n` は正の 2 冪 |
| `product_sum(a,b,c,d,n)` | 長さ `n` の NTT 済み配列から `a*b+c*d` の係数を復元し、末尾の 0 を除く |
| `multiply(a,b)` | 変換行列の積 `a*b` |
| `step(matrix,q)` | 互除法の 1 ステップ `[0,1;1,-q]` を左から掛ける |
| `apply(matrix,a,b)` | `half` の変換を元の多項式ペアに作用させて上書き |
| `half(a,b)` | 互除法を進め、後ろの多項式の係数数を元の `a.size()/2` 以下にする変換行列 |

`half` は末尾の 0 を除いた `a`・`b` に対し `a.size() > b.size()` を前提とします。`b` は空でも構いません。返した行列の作用後は、先頭多項式の係数数が元の半分より大きく、後ろが半分以下になります。行列式は ±1 です。

`apply` は、この互除法の行列と対応する多項式ペアに使います。一般の多項式行列には使いません。NTT 長を結果の次数から決めるため、高次の係数が相殺されることを前提とします。`multiply` も `shift` が成分の次数を上から抑える変換行列用です。

最大係数数を $$n$$ として、`half` は $$O(n\log^2 n)$$、行列積・作用は $$O(n\log n)$$ 時間、追加空間は $$O(n)$$ です。
