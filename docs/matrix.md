---
title: 行列
documentation_of: //math/matrix.hpp
redirect_from:
- /math/linear_algebra/matrix.hpp.html
- /lightweight/math/linear_algebra/matrix.hpp.html
library_category: math/linear_algebra
---

```cpp
#include "lib/math/matrix.hpp"
#include "lib/math/mint.hpp"
#include <cassert>
#include <optional>

int main() {
	Matrix<mint> a = { {1, 2}, {3, 4} };
	auto det = a.det();
	auto inverse = a.inverse();
	auto power = a.pow(10);
	std::optional<Matrix<mint>::Solution> solution = a.solve({5, 11});
	auto polynomial = a.characteristic_polynomial();
	assert(det == mint(-2));
	assert(inverse && a * *inverse == Matrix<mint>::identity(2));
	assert(power.rows() == 2 && power.cols() == 2);
	assert((solution && solution->particular == std::vector<mint>{1, 2}));
	assert((polynomial == std::vector<mint>{-2, -5, 1}));
}
```

`Matrix<T>` は可変サイズの行列です。各行を左から右へ、上の行から順に連続したメモリに保存します。添字は 0 始まり、区間は半開区間です。`bool` は使えません。

掃き出しでは、各行の計算の基準となる最初の 0 でない成分を「ピボット」と呼びます。ピボットが下の行ほど右にあり、0 だけの行が末尾に並ぶ形が「行階段形」です。さらに、各ピボットを 1 にし、その列の他の成分を全て 0 にした形が「簡約行階段形」です。

`rank`（階数）は、選んだ行のどれも他の選んだ行の定数倍の和では表せない、という条件で最大何本の行を選べるかを表します。逆行列が存在しない正方行列を「特異行列」と呼びます。

行数を $$n$$、列数を $$m$$、行列積では左が $$n\times k$$、右が $$k\times m$$ とします。行数・列数は非負の `int` です。累乗の $$k$$ は指数で、正方行列は $$n\times n$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Matrix<T>()` / `Matrix<T>(n)` | 0 行 0 列／0 初期化した正方行列 | 空: $$O(1)$$、正方: $$O(n^2)$$ |
| `Matrix<T>(n,m,value = T(0))` | `n` 行 `m` 列を同じ値で初期化 | $$O(nm)$$ |
| `Matrix<T>{ {...},{...} }` / `Matrix<T>(vector<vector<T>>)` | 係数列から構築。各行の長さは同じにする | $$O(nm)$$ |
| `rows()` / `cols()` / `empty()` | 行数／列数／行数または列数が 0 か | $$O(1)$$ |
| `a[i][j]` / `a(i,j)` | 要素の参照。const 版も利用可 | $$O(1)$$ |
| `a[i]` / `data()` | 行の `span`／行優先の先頭ポインタ | $$O(1)$$ |
| `Matrix<T>::identity(n)` | 単位行列 | $$O(n^2)$$ |
| `fill(x)` | 全要素を同じ値で上書き | $$O(nm)$$ |
| `swap(b)` | 行列を交換 | $$O(1)$$ |
| `transpose()` | 転置した行列を返す | $$O(nm)$$ |
| `submatrix(row_l,row_r,col_l,col_r)` | 指定した行・列区間をコピー | 切り出す要素数に比例 |
| `-a` / `a + b` / `a - b` | 符号反転／同じ形の行列の加減算 | $$O(nm)$$ |
| `a * b` | 行列積。`a.cols() == b.rows()` | $$O(nkm)$$ |
| `a * x` / `x * a` / `a / x` | 全要素を `x` 倍／全要素を `x` で割る。除算は体の要素で非零のみ | $$O(nm)$$ |
| `a += b` / `a -= b` / `a *= b` | 行列演算の結果を代入 | 加減算: $$O(nm)$$、積: $$O(nkm)$$ |
| `a *= x` / `a /= x` | 全要素を `x` 倍／`x` で割った結果を代入 | $$O(nm)$$ |
| `a == b` / `a != b` | 形と全要素を比較 | $$O(nm)$$ |
| `a * vector<T>` | 行列と列ベクトルの積。ベクトル長は列数 | $$O(nm)$$ |
| `pow(k)` | 正方行列の非負整数乗。`k` は `uint64_t`、0 乗は単位行列 | 固定の素数 modint で分解成功時: $$O(n^3+n^2\log k)$$、その他・最悪: $$O(n^3\log k)$$ |
| `pow(k,normalize)` | 単位行列・入力行列・各積の結果に `normalize(Matrix&)` を適用して累乗 | $$O(n^3\log k)$$ ＋ `normalize` の処理時間 |
| `trace()` | 正方行列の対角成分の和 | $$O(n)$$ |
| `det(eps = default_epsilon)` | 行列式。特異なら 0、0 行 0 列なら 1 | $$O(n^3)$$ |
| `inverse(eps = default_epsilon)` | 逆行列を `optional<Matrix<T>>` で返す。特異なら `nullopt` | $$O(n^3)$$ |
| `rank(eps = default_epsilon)` | 行列の rank | $$O(nm\min(n,m))$$ |
| `row_echelon(eps = default_epsilon)` | 行階段形とピボットの列番号の組。各ピボットは 1 | $$O(nm\min(n,m))$$ |
| `rref(eps = default_epsilon)` | 簡約行階段形とピボットの列番号の組 | $$O(nm\min(n,m))$$ |
| `solve(b,eps = default_epsilon)` | $$Ax=b$$ の解。解なしなら `nullopt` | $$O(nm\min(n,m)+m^2)$$ |
| `kernel(eps = default_epsilon)` | $$Ax=0$$ の解空間の基底。各要素が列ベクトル | $$O(nm\min(n,m)+m^2)$$ |
| `characteristic_polynomial(eps = default_epsilon)` | $$\det(xI-A)$$ の係数を定数項から返す | $$O(n^3)$$ |
| `adjugate(eps = default_epsilon)` | 余因子行列の転置。特異行列にも対応 | $$O(n^3)$$ |
| `Matrix<T>::Solution` | `LinearSolution<T>` の型別名 | 実行時処理・追加空間なし |

`solve` は `optional<Matrix<T>::Solution>` を返します。`Matrix<T>::Solution` は `LinearSolution<T>` と同じ型です。`b` の長さは行数とします。

| フィールド | 内容 |
| --- | --- |
| `particular` | 解を 1 つ返す。自由変数は 0 |
| `basis` | 同次方程式の解空間の基底。自由変数の列番号順 |

`basis` は、任意の同次方程式 `Ax=0` の解を、その定数倍の和で表せる最小本数のベクトルの組です。`solve` の全ての解は、`particular` に `basis` の各ベクトルを任意の定数倍して足すことで表せます。自由変数は、掃き出し後も値を自由に選べる変数です。ピボットの列番号は昇順です。行階段形などを返す操作を含め、元の行列は変更しません。

加減算・乗算・累乗には、加算の順序や括弧の付け方、乗算の括弧の付け方を変えても結果が同じで、乗算が加減算に分配される型を使います。整数・modint・多項式などが該当します。除算・逆行列・rank・掃き出し・方程式・核・特性多項式・余因子行列には、さらに 0 以外の全要素で割れる型（体）を使います。素数 mod の modint や `Fraction<>` に対応し、通常の整数型は使えません。ここで核とは、`Ax=0` を満たすベクトル全体のことです。

`adjugate()` は、成分 `(i,j)` が「元の行列から第 `j` 行と第 `i` 列を除いた行列の行列式」に `(-1)^(i+j)` を掛けた行列を返します。

符号付き整数型の `det` は例外として Bareiss 法で厳密に計算します。途中の積・差・商は `__int128_t`、結果は `T` に収まる必要があります。整数の他の演算も途中の値が `T` に収まる範囲で使います。

浮動小数点では絶対値が最大のピボットを選び、絶対値が `eps` 以下なら 0 と判定します。既定は `1e-12L`、それ以外の型は 0 です。`eps >= 0` とし、必要に応じて各操作の引数で変更します。浮動小数点の結果には丸め誤差を含みます。

固定の素数法を持つ modint では、専用の行列分解を利用できる場合に `pow` が高速になります。利用できない場合は繰り返し二乗法で計算します。合成数 mod・整数・浮動小数点でも `pow` を使えます。

FPS も要素型にでき、`Matrix<fps>` の単位行列の対角成分は定数 FPS の `{1}` になります。$$x^D$$ を法として計算するときは、`pow(k,normalize)` のコールバックで各要素を先頭 `D` 項に打ち切ってください。この累乗は繰り返し二乗法を使います。例は [軽量版の行列](matrix_light.md) にあります。FPS と行列を組み合わせて 64 KiB 制限で使う場合は、軽量版の FPS と行列のヘッダーを include し、同じ `Matrix<fps>` を使います。

$$n\times m$$ 行列について、構築・加減算・全要素の定数倍や除算・転置は $$O(nm)$$、行列積 $$n\times k$$ と $$k\times m$$ は $$O(nkm)$$、行列とベクトルの積は $$O(nm)$$、trace は $$O(n)$$ です。正方行列の累乗は固定の素数 modint で分解に成功すれば $$O(n^3+n^2\log k)$$、最悪や他の型で $$O(n^3\log k)$$、行列式・逆行列・特性多項式・余因子行列は $$O(n^3)$$ です。長方行列の rank・掃き出し・方程式・核は、基底の出力も含めて $$O(nm\min(n,m)+m^2)$$ です。rank・掃き出しだけなら $$O(nm\min(n,m))$$ です。追加空間は行列式・rank・掃き出しが $$O(nm)$$、方程式・核は $$O(nm+m^2)$$、正方行列の各操作は $$O(n^2)$$ です。行列積は入力の変換と出力を含めて $$O(nk+km+nm)$$ です。要素の演算・コピー・破棄を $$O(1)$$ とします。

サイズや要素列から構築する場合の保持領域と、コピー構築の時間・空間は $$O(nm)$$ です。コピー代入は代入前の要素数を $$q$$ として $$O(q+nm)$$ 時間です。小さい行列を代入しても、確保済みの領域は縮まらない場合があります。保持領域は確保済みの要素数を $$C$$ として $$O(C)$$ で、現在の行数・列数だけでは決まりません。要素参照・行の参照・寸法取得・比較・その場での加減算や定数倍の追加空間は $$O(1)$$ です。`k=0` の累乗は $$O(n^2)$$ 時間です。`pow(k,normalize)` は $$O(\log k)$$ 回の `normalize` を呼び、その時間・作業領域が別途必要です。`k=0` でも単位行列と入力行列に 1 回ずつ呼びます。
