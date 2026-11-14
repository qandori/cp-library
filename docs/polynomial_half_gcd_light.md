---
title: "多項式 Half-GCD（軽量版）"
documentation_of: //components/polynomial_half_gcd_light.hpp
library_category: components/fps
---

`fps_light_detail::PolynomialHalfGCD<T>` は、[軽量多項式 GCD](https://qandori.github.io/cp-library/fps/polynomial_gcd_light.hpp.html) が使う内部部品です。通常は公開する多項式 GCD のヘッダーを使います。`F` は `LightFormalPowerSeries<T>`、係数の法は 998244353 です。

| 型・関数 | 内容・返り値 |
| --- | --- |
| `Matrix` | 成分 `a` を行優先で持つ 2×2 多項式行列。初期値は単位行列 |
| `Matrix::identity` | 単位行列として処理を省略できるか |
| `high(a,k)` | 定数項側から `k` 係数を除いた多項式。`k >= 0` |
| `multiply(a,b)` | 行列積 `a*b` |
| `step(matrix,q)` | 互除法の行列 `[0,1;1,-q]` を左から掛ける |
| `apply(matrix,a,b)` | 行列を多項式ペアに掛けて上書きし、末尾の 0 を除く |
| `half(a,b)` | 後ろの多項式の係数数を元の `a.size()/2` 以下にする変換行列 |
| `reduce(a,b,matrix = nullptr)` | `b` が零多項式になるまで縮小。`matrix` を渡すと同じ変換を左から累積する |

`half` は末尾の 0 を除いた多項式について `a.size() > b.size()` を前提とします。`b` は空でも構いません。行列を掛けた後は先頭の多項式の係数数が元の半分より大きく、後ろは半分以下になります。行列式は ±1 です。`reduce` の入力も末尾の 0 を除きます。両方が空の入力も扱えます。

`Matrix::a` を直接書き換える場合は `identity` を正しく設定してください。NTT キャッシュや複数要素をまとめて計算する処理を持つ[高速実装はこちら](https://qandori.github.io/cp-library/components/polynomial_half_gcd.hpp.html)。

最大係数数を $$n$$、畳み込みの計算量を $$M(n)$$ として、`half`・`reduce` は $$O(M(n)\log n)$$、行列積・行列と多項式の組の積・`step` は $$O(M(n))$$、`high` は $$O(n)$$ 時間です。追加空間は $$O(n)$$ です。
