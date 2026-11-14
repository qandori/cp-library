---
title: "多項式 Half-GCD"
documentation_of: //components/polynomial_half_gcd.hpp
library_category: components/fps
---

`fps_detail::PolynomialHalfGCD` は、多項式 GCD・拡張 GCD で共有する内部処理です。通常は [多項式 GCD](https://qandori.github.io/cp-library/fps/polynomial_gcd.hpp.html) のヘッダーを使います。係数型は内部の `mint`、法は 998244353 です。

| 型・関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `T` / `F` / `O` | 係数型／FPS 型／共通 FPS 演算 | —（型） |
| `direct_limit` | 小区間を直接処理する次数の閾値 | $$O(1)$$ |
| `Matrix` | 多項式を要素とする 2×2 行列。初期値は単位行列 | —（型） |
| `Matrix::a` | 成分を行優先で並べた配列 | $$O(1)$$ |
| `Matrix::cache` | 各成分の NTT キャッシュ。必要に応じて倍化する | $$O(1)$$ |
| `Matrix::shift` | 互除法の商の次数の総和。`half` では先頭多項式の次数の低下量に一致 | $$O(1)$$ |
| `Matrix::identity` | 単位行列の省略処理を使えるか | $$O(1)$$ |
| `high(a,k)` | `a` の定数項側から `k` 係数を除いた多項式。`k >= 0` | $$O(n)$$ |
| `fold(a,n)` | `a mod (x^n-1)` を長さ `n` の配列にする。`n` は正の 2 冪 | $$O(n)$$ |
| `transformed(matrix,index,n)` | 成分 `index` の長さ `n` の NTT への参照。返り値の先頭 `n` 個を使う | $$O(n\log n)$$ |
| `product_sum(a,b,c,d,n,cache = nullptr)` | 長さ `n` の NTT 済み配列から `a*b+c*d` の係数を復元し、末尾の 0 を除く。`cache` に逆変換前の配列を保存できる | $$O(n\log n)$$ |
| `multiply(a,b,first_only = false)` | 変換行列の積 `a*b`。`first_only` が真なら上段だけを計算 | $$O(n\log n)$$ |
| `multiply_column(a,b)` | `a*b` の第 1 列のみを計算。`a` は全成分、`b` は第 1 列を使う | $$O(n\log n)$$ |
| `step(matrix,q)` | 互除法の 1 ステップ `[0,1;1,-q]` を左から掛ける | $$O(n\log n)$$ |
| `step_column(matrix,q)` | `step` と同じ行列を第 1 列のみに掛ける | $$O(n\log n)$$ |
| `apply(matrix,a,b)` | `half` の変換を元の多項式ペアに掛けて上書き | $$O(n\log n)$$ |
| `apply_first(matrix,a,b)` | 変換後の先頭多項式を返す。行列の上段のみを使う | $$O(n\log n)$$ |
| `combine(out,a,b,n,x,y,z)` | `out[i] = x*a[i]+y*b[i-1]+z*b[i]` を計算 | $$O(n)$$ |
| `combine8(out,a,b,n,x,y,z)` | `combine` の AVX2（複数の整数をまとめて処理する CPU 命令）版。係数 `x,y,z` は通常の剰余表現 | $$O(n)$$ |
| `direct(a,b,target = -1)` | 小区間の互除法。`target = -1` は `a.size()/2` | $$O(n^2)$$ |
| `jump(a,b,k,first_only = false)` | 後ろの多項式の次数を `deg(a)-k` 未満にする変換行列 | $$O(n\log^2 n)$$ |
| `half(a,b)` | 互除法を進め、後ろの多項式の係数数を元の `a.size()/2` 以下にする変換行列 | $$O(n\log^2 n)$$ |

`half` は末尾の 0 を除いた `a`・`b` に対し `a.size() > b.size()` を前提とします。`b` は空でも構いません。返した行列を多項式の組に掛けた後は、先頭多項式の係数数が元の半分より大きく、後ろが半分以下になります。行列式は ±1 です。

`apply` は、この互除法の行列と対応する多項式ペアに使います。一般の多項式行列には使いません。NTT 長を結果の次数から決めるため、高次の係数が相殺されることを前提とします。`multiply` も `shift` が成分の次数を上から抑える変換行列用です。

`multiply_column`・`step_column` の結果は第 1 列のみ有効です。以降も第 1 列用の関数で扱います。

`jump` も `a.size() > b.size()` と末尾の 0 がないことを前提とし、$$0\le k\le\deg a$$ とします。`first_only = true` の返り値は上段だけが有効です。`multiply` で上段だけを求める場合、左の行列は上段、右の行列は全成分を用意します。

`direct` は `a.size() <= 2 * direct_limit + 1`、$$0\le\mathrm{target}<a.\mathrm{size}()$$ とし、後ろの多項式の次数が `target` 未満になるまで進めます。返す行列は、通常の互除法の各操作を表す行列を掛け合わせたものです。

`combine` の各領域は重ならず、`b[-1]` に 0 を置きます。`combine8` は x86 のみで定義され、AVX2 対応を確認してから呼び出します。行列の成分を直接書き換える場合は、対応する `cache` も空にします。キャッシュへの参照は以降の変換で無効になることがあります。

最大係数数を $$n$$ として、`half`・`jump` は $$O(n\log^2 n)$$、行列積・多項式の組への乗算・変換は $$O(n\log n)$$ 時間、追加空間は $$O(n)$$ です。`direct` は $$O(n^2)$$ 時間、`combine`・`combine8` は $$O(n)$$ 時間です。
