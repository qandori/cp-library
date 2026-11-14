---
title: mod 2 専用行列
documentation_of: //math/matrix_mod2.hpp
library_category: math/linear_algebra
---

要素が 0・1 の行列を、全ての計算を mod 2 として扱います。加算・減算は XOR、要素同士の乗算は AND です。例えば行列積の成分は、対応する行と列で両方が 1 の位置の個数が奇数なら 1、偶数なら 0 になります。丸め誤差はありません。

C++23 で使用します。

```cpp
#include "lib/math/matrix_mod2.hpp"
#include <cassert>
#include <string>
#include <vector>

int main() {
	MatrixMod2 a{ {1, 1, 0}, {0, 1, 1}, {1, 1, 1} };
	assert(a.rank() == 3 && a.det());
	auto inverse = a.inverse();
	assert(inverse && a * *inverse == MatrixMod2::identity(3));

	DynamicBitset b(3);
	b[0] = true;
	b[2] = true;
	auto solution = a.solve(b);
	assert(solution && solution->basis.empty());
	assert(a * solution->particular == b);

	MatrixMod2 text(std::vector<std::string>{"110", "011"});
	assert(text(0, 0) && text(0, 1) && !text(0, 2));
	assert(text.row_string(1) == "011");
	assert(text.kernel().size() == 1);
}
```

`MatrixMod2` は行数・列数を持ち、添字は 0 始まりです。`a(i,j)` と `a[i][j]` で要素を読み書きできます。書き込む値は `bool`、数値の初期値列では 0 または 1 を使います。`a(i,j) = true`・`a[i][j] = false` のように代入し、`a(i,j).flip()` で 0 と 1 を入れ替えられます。行列全体の形を変えるときは、新しい行列を代入します。要素を自分の式で計算するときも、加算には `^`、乗算には `&` を使います。

行を文字列で渡す場合は、**左端の文字が列 0** です。例えば `"100"` は列 0 だけが 1 の行です。[DynamicBitset](dynamic_bitset.md) の文字列構築は右端を bit 0 とするので、`DynamicBitset("100")` と同じ並びにはなりません。`DynamicBitset` と行をやり取りするときは、bit `j` を列 `j` として扱います。連立方程式の右辺も、bit `i` が第 `i` 行に対応します。

他の行を XOR する操作などで形を整える処理を「掃き出し」と呼びます。各行で、基準となる最初の 1 を「ピボット」と呼びます。ピボットが下の行ほど右にあり、全て 0 の行が末尾に並ぶ形が「行階段形」です。さらに各ピボットの列で、他の行の値が全て 0 の形が「簡約行階段形」です。`rank`（階数）は、ピボットの個数、つまり他の行の XOR だけでは作れない行を最大何本選べるかを表します。

`Ax=0` を満たすベクトル全体を「核」と呼びます。その全てのベクトルを、いくつか選んで XOR することで表せる最小本数のベクトルの組を「基底」と呼びます。

行数を $$n$$、列数を $$m$$、積では左が $$n\times k$$、右が $$k\times m$$ とします。$$W(t)=\lceil t/64\rceil$$ は、長さ $$t$$ の 0・1 列を保持する 64 bit 整数の個数です。空間は返り値を含む追加空間です。

計算量の式は次の時間上界を使います。$$t$$ は作業する行の全列数で、係数以外に右辺や単位行列を付ける場合も含めます。

$$E(n,m,t)=n\min(n,m)W(t)+nm+nW(t)$$

$$P(n,k,m)=nkW(m)+nmW(k)+kW(m)+nW(m)$$

| 操作・型 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `MatrixMod2()` | 0 行 0 列で構築 | $$O(1)$$ | $$O(1)$$ |
| `MatrixMod2(n)` / `MatrixMod2(n,m,value = false)` | 正方行列／長方行列を同じ値で初期化 | $$O(nW(m))$$。正方行列では $$m=n$$ | $$O(nW(m))$$ |
| `MatrixMod2{ {...},{...} }` / `MatrixMod2(vector<vector<int>>)` | 0・1 の係数列から構築。各行の長さは同じにする | $$O(nm)$$ | $$O(nW(m))$$ |
| `MatrixMod2(vector<string>)` | 各文字が 0 または 1 の行から構築 | $$O(nm)$$ | $$O(nW(m))$$ |
| `MatrixMod2(vector<DynamicBitset>)` | 各 bitset を 1 行として構築。長さをそろえる | $$O(nW(m))$$ | $$O(nW(m))$$ |
| `MatrixMod2::from_words(n,m,words)` | 行ごとに $$W(m)$$ 個ずつ並んだ 64 bit 整数から構築。末尾の余分な bit は除く | $$O(nW(m))$$ | $$O(nW(m))$$ |
| `rows()` / `cols()` / `empty()` / `words_per_row()` | 行数／列数／行数または列数が 0 か／1 行の 64 bit 整数の個数 | $$O(1)$$ | $$O(1)$$ |
| `a(i,j)` / `a[i][j]` | 要素を読み書き。const 版は `bool`、非 const 版は代入できる `reference` を返す | $$O(1)$$ | $$O(1)$$ |
| `a[i]` / `a[i].size()` | 行を参照する `Row`・`ConstRow`／列数を `int` で返す | $$O(1)$$ | $$O(1)$$ |
| 要素参照の代入・`flip()`・`^= value`・`bool` への変換 | 指定した要素の更新・反転・XOR・読み取り | $$O(1)$$ | $$O(1)$$ |
| `set(i,j,value = true)` / `flip(i,j)` | 要素の代入／0 と 1 の反転。返り値は `void` | $$O(1)$$ | $$O(1)$$ |
| `fill(value)` | 全要素を同じ値で上書き。返り値は `void` | $$O(nW(m))$$ | $$O(1)$$ |
| `row_words(i)` | 第 `i` 行を `span<const uint64_t>` で参照 | $$O(1)$$ | $$O(1)$$ |
| `row(i)` | 第 `i` 行を `DynamicBitset` にコピー | $$O(W(m))$$ | $$O(W(m))$$ |
| `set_row(i,bits)` | 長さ `m` の `DynamicBitset` を第 `i` 行へコピー。返り値は `void` | $$O(W(m))$$ | $$O(1)$$ |
| `set_row(i,text)` | 長さ `m` の `string_view` を第 `i` 行へコピー。返り値は `void` | $$O(m)$$ | $$O(1)$$ |
| `row_string(i)` | 第 `i` 行を左から列順の `string` にコピー | $$O(m)$$ | $$O(m)$$ |
| `swap(b)` | 行列を交換。返り値は `void` | $$O(1)$$ | $$O(1)$$ |
| `MatrixMod2::identity(n)` | 単位行列 | $$O(nW(n)+n)$$ | $$O(nW(n))$$ |
| `transpose()` | 転置した行列を返す | $$O(nW(m)+mW(n))$$ | $$O(mW(n))$$ |
| `submatrix(row_l,row_r,col_l,col_r)` | 指定した半開区間の行・列をコピー | 切り出した形を $$h\times w$$ として $$O(hW(w))$$ | $$O(hW(w))$$ |
| `-a` / `a + b` / `a - b` | 符号反転／同じ形の行列の加減算。mod 2 では `-a == a`、加算と減算は同じ | $$O(nW(m))$$ | $$O(nW(m))$$ |
| `a += b` / `a -= b` | 加減算した値を自身へ代入 | $$O(nW(m))$$ | $$O(1)$$ |
| `a * b` / `a *= b` | 行列積。`a.cols() == b.rows()` | $$O(P(n,k,m))$$ | $$O(nW(m)+mW(k)+W(m))$$ |
| `a * value` / `value * a` / `a *= value` | `bool` の値を全要素に掛ける。0 倍は零行列、1 倍は元の値 | $$O(nW(m))$$ 以内 | 返す場合 $$O(nW(m))$$、その場では $$O(1)$$ |
| `a == b` / `a != b` | 形と全要素を比較 | $$O(nW(m))$$ | $$O(1)$$ |
| `a * bits` | 長さ `m` の `DynamicBitset` を列ベクトルとして掛ける。長さ `n` の `DynamicBitset` を返す | $$O(nW(m)+W(n))$$ | $$O(W(n))$$ |
| `pow(k)` | 正方行列の非負整数乗。`k` は `uint64_t` | $$O(P(n,n,n)\log k+nW(n))$$ | $$O(nW(n))$$ |
| `trace()` | 正方行列の対角成分の和を `bool` で返す | $$O(n)$$ | $$O(1)$$ |
| `rank()` | 階数を `int` で返す | $$O(E(n,m,m))$$ | $$O(nW(m)+n+\min(n,m))$$ |
| `det()` | 正方行列の行列式を `bool` で返す | $$O(E(n,n,n))$$ | $$O(nW(n)+n)$$ |
| `row_echelon()` / `rref()` | 行階段形／簡約行階段形と、ピボットの列番号の組を返す | $$O(E(n,m,m))$$ | $$O(nW(m)+n+\min(n,m))$$ |
| `inverse()` | 逆行列を `optional<MatrixMod2>` で返す。存在しなければ `nullopt` | $$O(E(n,n,2n))$$ | $$O(nW(2n)+n)$$ |
| `kernel()` | $$Ax=0$$ の全ての解を XOR で表すための基底を `vector<DynamicBitset>` で返す | $$O(E(n,m,m)+nm+mW(m))$$ | $$O(nW(m)+n+mW(m))$$ |
| `solve(bits)` | $$Ax=b$$ の解を `optional<LinearSolutionMod2>` で返す。解なしなら `nullopt` | $$O(E(n,m,m)+nm+mW(m))$$ | $$O(nW(m)+n+mW(m))$$ |
| `MatrixMod2::value_type` / `MatrixMod2::word_type` | それぞれ `bool`・`std::uint64_t` の別名 | —（型） | —（型） |
| `LinearSolutionMod2` / `MatrixMod2::Solution` | `DynamicBitset particular` と `vector<DynamicBitset> basis` を持つ同じ型 | —（型） | —（型） |

`rows()`・`cols()`・`words_per_row()` は `int`、`empty()` は `bool` を返します。行列の複合代入演算子は `MatrixMod2&`、要素参照の代入・`flip()`・`^=` は `reference&` を返します。`auto bit = a(i,j)` は要素を参照し続けるので、値だけを保存するときは `bool bit = a(i,j)` とします。

`row_echelon()`・`rref()` は `pair<MatrixMod2,vector<int>>` を返し、列番号は昇順です。行列式・階数・掃き出し・逆行列・方程式・核を求める操作は、元の行列を変更しません。

`solve` の `particular` は長さ `m` の解を 1 つ表します。`basis` は、$$Ax=0$$ を満たす全てのベクトルを、その中からいくつか選んで XOR することで表せる最小本数のベクトルの組です。これを基底と呼びます。`solve` の全ての解は、`particular` に `basis` の任意の部分集合を XOR して得られます。`kernel` はこの `basis` だけを返します。基底の本数は `m-rank()`、各ベクトルの長さは `m` です。

行数・列数は非負の `int` で指定します。行番号は `[0,n)`、列番号は `[0,m)` とし、`submatrix` は `0 <= row_l <= row_r <= n`・`0 <= col_l <= col_r <= m` を満たす半開区間です。初期値の数値は 0 または 1、文字列は `'0'` または `'1'` のみを使います。文字列に他の文字があれば `std::invalid_argument` を送出します。配列から構築するときは各行の長さをそろえ、行数・各行の長さを `INT_MAX` 以下にします。bitset の右辺は行数と同じ長さにします。

`row_words(i)` は読み取り専用の参照です。列 `j` は `row_words(i)[j/64]` の下位から `j%64` 番目の bit に対応します。`from_words` の入力はちょうど `n*words_per_row()` 個とし、行優先で並べます。各行の最終整数のうち、列数を超える部分は 0 にそろえます。要素の参照用の値や行の参照は、行列の破棄・代入・移動などで記憶領域が変わると使えなくなります。

`n × 0` と `0 × m` の形も保持します。`n × 0` と `0 × m` の積は `n × m` の零行列です。0 行 0 列の行列式は 1、逆行列は存在する 0 行 0 列の行列、0 乗は単位行列です。0 行 `m` 列の方程式は任意の長さ `m` のベクトルが解で、`solve` は零ベクトルと `m` 本の基底を返します。`n` 行 0 列では、右辺が全て 0 の場合だけ長さ 0 の解があります。

逆行列・行列式・対角和・累乗には正方行列を渡します。`inverse()` は `2*n <= INT_MAX`、`solve()` は `m < INT_MAX` が必要です。行列と作業領域がメモリに収まるサイズで使ってください。

保持する行列本体の領域は $$O(nW(m))$$ です。各演算の追加空間は表のとおりで、返す行列・bitset・基底も含みます。空の形の構築・読み取りなどにも定数時間かかります。列数 0 の `solve` は、右辺の確認を含めて $$O(n)$$ 時間・空間です。行の配列から構築する場合は、列数 0 でも各行の長さを確認するため $$O(n)$$ 時間です。`pow(0)` は $$O(nW(n)+n)$$ 時間、$$O(nW(n))$$ 空間で単位行列を返します。0 行 0 列の `pow(k)` は指数のビット数に比例した $$O(\log k)$$ 時間・$$O(1)$$ 空間で、`k=0` なら定数時間です。

コピー構築は $$O(nW(m))$$ 時間・空間で、コピー後の変更は独立です。コピー代入は、代入先の元の 64 bit 整数の個数を $$L$$ として $$O(L+nW(m))$$ 時間、$$O(nW(m))$$ 追加空間以内を使い、以前の確保容量が残る場合があります。確保済みの 64 bit 整数の個数を $$C$$ とすると、実際の保持領域は $$O(C)$$ です。移動構築・交換は $$O(1)$$ 時間・追加空間、移動代入には代入先の旧領域の解放時間が加わります。移動元は 0 行 0 列の空の行列になります。共有する前計算表や、演算をまたいで増えるキャッシュはありません。
