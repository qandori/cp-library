---
title: "行列"
documentation_of: //math/matrix.hpp
---

```cpp
#include "lib/math/matrix.hpp"
#include "lib/math/mint.hpp"

Matrix<mint> a = { {1, 2}, {3, 4} };
auto det = a.det();
auto inverse = a.inverse();
auto power = a.pow(10);
auto solution = a.solve({5, 11});
auto polynomial = a.characteristic_polynomial();
```

`Matrix<T>` は行優先の連続領域に保存する可変サイズの行列です。添字は 0 始まり、区間は半開区間です。`bool` は使えません。

| 操作 | 内容・返り値 |
| --- | --- |
| `Matrix<T>()` / `Matrix<T>(n)` | 0 行 0 列／0 初期化した正方行列 |
| `Matrix<T>(n,m,value = T(0))` | `n` 行 `m` 列を同じ値で初期化 |
| `Matrix<T>{ {...},{...} }` / `Matrix<T>(vector<vector<T>>)` | 係数列から構築。各行の長さは同じにする |
| `rows()` / `cols()` / `empty()` | 行数／列数／行数または列数が 0 か |
| `a[i][j]` / `a(i,j)` | 要素の参照。const 版も利用可 |
| `a[i]` / `data()` | 行の `span`／行優先の先頭ポインタ |
| `Matrix<T>::identity(n)` | 単位行列 |
| `fill(x)` | 全要素を同じ値で上書き |
| `swap(b)` | 行列を交換 |
| `transpose()` | 転置した行列を返す |
| `submatrix(row_l,row_r,col_l,col_r)` | 指定した行・列区間をコピー |
| `-a` / `a + b` / `a - b` | 符号反転／同じ形の行列の加減算 |
| `a * b` | 行列積。`a.cols() == b.rows()` |
| `a * x` / `x * a` / `a / x` | スカラー倍／スカラー除算。除算は体の要素で非零のみ |
| `a += b` / `a -= b` / `a *= b` | 行列演算の結果を代入 |
| `a *= x` / `a /= x` | スカラー演算の結果を代入 |
| `a == b` / `a != b` | 形と全要素を比較 |
| `a * vector<T>` | 行列と列ベクトルの積。ベクトル長は列数 |
| `pow(k)` | 正方行列の非負整数乗。`k` は `uint64_t`、0 乗は単位行列 |
| `trace()` | 正方行列の対角成分の和 |
| `det(eps = default_epsilon)` | 行列式。特異なら 0、0 行 0 列なら 1 |
| `inverse(eps = default_epsilon)` | 逆行列を `optional<Matrix<T>>` で返す。特異なら `nullopt` |
| `rank(eps = default_epsilon)` | 行列の rank |
| `row_echelon(eps = default_epsilon)` | 行階段形とピボットの列番号の組。各ピボットは 1 |
| `rref(eps = default_epsilon)` | 簡約行階段形とピボットの列番号の組 |
| `solve(b,eps = default_epsilon)` | $$Ax=b$$ の解。解なしなら `nullopt` |
| `kernel(eps = default_epsilon)` | $$Ax=0$$ の解空間の基底。各要素が列ベクトル |
| `characteristic_polynomial(eps = default_epsilon)` | $$\det(xI-A)$$ の係数を定数項から返す |
| `adjugate(eps = default_epsilon)` | 余因子行列の転置。特異行列にも対応 |

`solve` は `optional<LinearSolution<T>>` を返します。`b` の長さは行数とします。

| フィールド | 内容 |
| --- | --- |
| `particular` | 解を 1 つ返す。自由変数は 0 |
| `basis` | 同次方程式の解空間の基底。自由変数の列番号順 |

全ての解は `particular` と `basis` の線形結合の和です。ピボットの列番号も昇順です。行階段形などを返す操作を含め、元の行列は変更しません。

加減算・乗算・累乗には環の要素を使えます。除算・逆行列・rank・掃き出し・方程式・核・特性多項式・余因子行列には、非零要素で割れる体を使います。素数 mod の modint や `Fraction<>` に対応します。通常の整数型をこれらの除算を伴う操作には使えません。

符号付き整数型の `det` は例外として Bareiss 法で厳密に計算します。途中の積・差・商は `__int128_t`、結果は `T` に収まる必要があります。整数の他の演算も途中の値が `T` に収まる範囲で使います。

浮動小数点では絶対値が最大のピボットを選び、絶対値が `eps` 以下なら 0 と判定します。既定は `1e-12L`、それ以外の型は 0 です。`eps >= 0` とし、必要に応じて各操作の引数で変更します。浮動小数点の結果には丸め誤差を含みます。

`mod()` がコンパイル時定数で $$1<\mathrm{mod}<2^{31}$$、`val()`・`raw()` を持つ modint は専用の処理を使います。AVX2 は実行時に判定し、使えない環境では通常処理に切り替えます。法の素数性は検査しません。

$$n\times m$$ 行列について、構築・加減算・スカラー演算・転置は $$O(nm)$$、行列積 $$n\times k$$ と $$k\times m$$ は $$O(nkm)$$、行列とベクトルの積は $$O(nm)$$、trace は $$O(n)$$ です。正方行列の累乗は $$O(n^3\log k)$$、行列式・逆行列・特性多項式・余因子行列は $$O(n^3)$$ です。長方行列の rank・掃き出し・方程式・核は、基底の出力も含めて $$O(nm\min(n,m)+m^2)$$ です。rank・掃き出しだけなら $$O(nm\min(n,m))$$ です。追加空間は行列式・rank・掃き出しが $$O(nm)$$、方程式・核は $$O(nm+m^2)$$、正方行列の各操作は $$O(n^2)$$ です。行列積は入力の変換と出力を含めて $$O(nk+km+nm)$$ です。要素の演算・コピーを $$O(1)$$ とします。
