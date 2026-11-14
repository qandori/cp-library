---
title: 行列（軽量実装）
documentation_of: //math/matrix_light.hpp
library_category: math/linear_algebra
redirect_from:
- /math/linear_algebra/matrix_light.hpp.html
- /lightweight/math/linear_algebra/matrix_light.hpp.html
---

通常版と同じ `Matrix<T>` と操作を使える軽量実装です。[行列](matrix.md) と同じ条件で利用できます。同じ機能は通常版か軽量版のいずれか一方を include してください。`LightMatrix<T>` でも軽量版を明示できます。

```cpp
#include "lib/math/matrix_light.hpp"
#include "lib/fps/fps_light.hpp"
#include <cassert>

int main() {
	using M = Matrix<fps>;
	M a{ {fps{1}, fps{0, 1}},
	     {fps{0, 1}, fps{1}} };
	int degree = 8;
	auto b = a.pow(1000000000000000000ULL, [degree](M &m) {
		for (int i = 0; i < m.rows(); ++i)
			for (auto &f : m[i])
				if (f.size() > std::size_t(degree)) f.resize(degree);
	});
	assert(b.rows() == 2 && b.cols() == 2);
	for (int i = 0; i < b.rows(); ++i)
		for (const auto &f : b[i]) assert(f.size() <= std::size_t(degree));
}
```

この例は $$x^8$$ を法として FPS を要素とする行列を累乗します。`pow(k,normalize)` は単位行列・入力行列と、各積の結果に `normalize` を呼びます。FPS の次数は積のたびに打ち切るため、大きな指数も扱えます。通常版でも `Matrix<fps>` と同じ書き方ができます。

行数を $$n$$、列数を $$m$$、行列積では左が $$n\times k$$、右が $$k\times m$$ とします。行数・列数は非負の `int`、添字は 0 始まり、区間は半開区間です。累乗の $$k$$ は非負の指数で、正方行列は $$n\times n$$ とします。以下の時間は、要素の演算・コピー・破棄を $$O(1)$$ としたものです。

掃き出しでは、各行の計算の基準となる最初の 0 でない成分を「ピボット」と呼びます。ピボットが下の行ほど右にあり、0 だけの行が末尾に並ぶ形が「行階段形」です。さらに、各ピボットを 1 にし、その列の他の成分を全て 0 にした形が「簡約行階段形」です。

`rank`（階数）は、選んだ行のどれも他の選んだ行の定数倍の和では表せない、という条件で最大何本の行を選べるかを表します。逆行列が存在しない正方行列を「特異行列」と呼びます。

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
| `a * x` / `x * a` / `a / x` | 全要素を `x` 倍／全要素を `x` で割る。除算では `x` は非零にする | $$O(nm)$$ |
| `a += b` / `a -= b` / `a *= b` | 行列演算の結果を代入 | 加減算: $$O(nm)$$、積: $$O(nkm)$$ |
| `a *= x` / `a /= x` | 全要素を `x` 倍／`x` で割った結果を代入 | $$O(nm)$$ |
| `a == b` / `a != b` | 形と全要素を比較 | $$O(nm)$$ |
| `a * vector<T>` | 行列と列ベクトルの積。ベクトル長は列数 | $$O(nm)$$ |
| `pow(k)` | 正方行列の非負整数乗。`k` は `uint64_t`、0 乗は単位行列 | $$O(n^3\log k)$$ |
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

`solve` の右辺 `b` は長さが行数の `vector<T>` です。返り値は `optional<Matrix<T>::Solution>` で、`Matrix<T>::Solution` は `LinearSolution<T>` と同じ型です。`particular` は解を 1 つ表す長さ $$m$$ のベクトル、`basis` は `Ax=0` を満たすベクトル全体をその定数倍の和で表せる基底です。全ての解は、`particular` に `basis` の各ベクトルを任意の定数倍して足すことで表せます。これらの操作は元の行列を変更しません。

`T` に `bool` は使えません。加減算・乗算・累乗には整数・modint・多項式などを使えます。除算・逆行列・階数・掃き出し・方程式・核・特性多項式・余因子行列では、0 以外の全要素で割れる型を使います。例えば素数法の modint に対応し、通常の整数型は使えません。例外として符号付き整数の `det` は厳密に計算しますが、途中の積・差・商は `__int128_t`、結果は `T` に収まる必要があります。浮動小数点の `eps` は非負とし、既定値は `1e-12L`、それ以外の型は 0 です。行階段形などの用語や、型に関する詳しい条件は [通常版](matrix.md) を参照してください。

軽量版の累乗は全ての要素型で繰り返し二乗法を使います。`k=0` の累乗は $$O(n^2)$$ 時間です。固定法 modint の行列で速度を重視する場合は通常版を使います。通常版の行列単独も 64 KiB 以内に収まります。

サイズや要素列から構築する場合の保持領域と、コピー構築の時間・空間は $$O(nm)$$ です。コピー代入は代入前の要素数を $$q$$ として $$O(q+nm)$$ 時間です。小さい行列を代入しても、確保済みの領域は縮まらない場合があります。保持領域は確保済みの要素数を $$C$$ として $$O(C)$$ で、現在の行数・列数だけでは決まりません。要素参照・行の参照・寸法取得・比較・その場での加減算や定数倍の追加空間は $$O(1)$$ です。行列を返す加減算・転置・定数倍・行列積は出力の要素数だけの追加空間を使い、行列とベクトルの積は $$O(n)$$、切り出しは切り出す要素数だけの空間を使います。累乗・行列式・逆行列・特性多項式・余因子行列は $$O(n^2)$$、階数・掃き出しは $$O(nm)$$、方程式・核は出力する基底も含め $$O(nm+m^2)$$ の追加空間です。

FPS など、要素の演算やコピーが定数時間でない型では、その演算・コピーの時間と領域も必要です。`pow(k,normalize)` は $$O(\log k)$$ 回の `normalize` を呼び、その時間と作業領域が別途必要です。`k=0` でも単位行列と入力行列に 1 回ずつ呼びます。
