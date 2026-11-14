---
title: "軽量な形式的冪級数"
documentation_of: //fps/fps_light.hpp
library_category: fps
---

形式的冪級数（FPS）は、$$f_0+f_1x+f_2x^2+\cdots$$ を係数列として扱うものです。必要な個数まで係数を計算し、実数の無限和の収束は考えません。

FPS の本体と追加演算は、同じ版のヘッダーを選んでください。

```cpp
#include "lib/fps/fps_light.hpp"
#include <cassert>

int main() {
	fps f{1, 2, 3}, g{4, 5};
	auto product = f * g;
	auto inverse = f.inv(10);
	auto logarithm = f.log(10);
	assert((product == fps{4, 13, 22, 15}));
	assert(inverse.size() == 10 && inverse[0] == mint(1));
	assert(logarithm.size() == 10 && logarithm[0] == mint(0));
}
```

Codeforces などの小さいソースサイズ上限向けの FPS です。法は 998244353、係数は定数項から順に格納します。`fps` は `FormalPowerSeries<mint>` の別名で、`std::vector<mint>` と同様の要素アクセス・イテレータ・サイズ変更を使えます。

公開名と操作は [通常版 FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) と同じです。同じ機能は通常版か軽量版のいずれか一方を include してください。`light_fps`・`LightFPS`・`LightFormalPowerSeries<T>` でも軽量版を明示できます。提出するソース全体のサイズを確認してください。

係数をまとめて出力するときは [FPS の高速出力](fps_write.md) を追加できます。

`FormalPowerSeries<T>` は `mod()` が定数式となる法 998244353 の modint 型を指定できます。`T` は整数からの構築、`mod()`・`raw()`・`val()`・`inv()`・`pow()`、四則演算と等値比較に対応する必要があります。

`deg` は返す係数数で、省略時は元の長さです。逆数の例は $$x^0$$ から $$x^9$$ までを返します。通常の積は入力が非空なら長さ `f.size()+g.size()-1`、`diff()`・`integral()`・シフト・`pow(k)` は元の長さを保ちます。例えば `fps{1,1}.pow(3)` は `{1,3}` なので、4 係数必要なら先に `resize(4)` してください。

`pre(n)` は高次の係数を切り落とすだけで、長さを伸ばすときは `resize(n)` を使います。加減算は長い方の入力長、定数との加減算は空列なら長さ 1 になり、それ以外は元の長さを保ちます。`divmod` は `{商,余り}` の `pair` を返し、どちらも末尾の零を除き、零多項式は空配列です。`f/g` は多項式の商ではなく級数の除算です。

表の $$n$$ は、操作に渡す係数列と返す係数列の最大長です。`pre(n)`・`resize(n)` の引数も、この最大長に含めます。係数の四則演算とコピーを定数時間とします。

| 操作 | 内容・条件 | 時間計算量 |
| --- | --- | --- |
| `fps()` / `fps(n)` / `fps{a0,a1,...}` / `fps(vector<mint>)` | 空列／零列／係数列から構築 | 空列・配列の移動: $$O(1)$$、長さ指定・係数のコピー: $$O(n)$$ |
| `f = a` / `f = {a0,a1,...}` | 配列／初期値列の代入 | $$O(n)$$ |
| `f[i]` / `f.size()` / `f.empty()` / `f.resize(n)` | `std::vector<mint>` と同じ要素取得・サイズ操作。`0 <= i < f.size()`、`n >= 0` | 参照・サイズ・空判定: $$O(1)$$、サイズ変更: $$O(n)$$ |
| `FormalPowerSeries<T>::multiply(f,g)` / `f * g` | 多項式の積 | $$O(n\log n)$$ |
| `-f` / `f + g` / `f - g` | 符号反転／加減算 | $$O(n)$$ |
| `f + c` / `f - c` / `f * c` / `f / c` | 定数項への加減算／全係数の定数倍・除算。除数は非零 | $$O(n)$$ |
| `f += g` / `f -= g` | 係数列との加減算を自身へ代入 | $$O(n)$$ |
| `f *= g` / `f /= g` / `f %= g` | 乗算・級数除算・剰余を自身へ代入 | $$O(n\log n)$$ |
| `f += c` / `f -= c` / `f *= c` / `f /= c` | 定数との演算を自身へ代入。除数は非零 | 定数項の加減算: $$O(1)$$、全係数の定数倍・除算: $$O(n)$$ |
| `f << d` / `f >> d` / `f <<= d` / `f >>= d` | 係数をずらして元の長さで打ち切る。`d >= 0` | $$O(n)$$ |
| `f.pre(n)` | 先頭から高々 `n` 係数を返す。`n >= 0` | $$O(n)$$ |
| `f.shrink()` / `f.trim()` | 末尾の零を除く。零多項式は空配列になる | $$O(n)$$ |
| `f / g` / `f.div_series(g,deg = -1)` | 級数除算。$$g_0\ne0$$ | $$O(n\log n)$$ |
| `f.divmod(g)` / `f % g` | 多項式の商と余り。除数は零多項式でない | $$O(n\log n)$$ |
| `f.inv(deg = -1)` | 逆数。$$f_0\ne0$$ | $$O(n\log n)$$ |
| `f.log(deg = -1)` | 対数。$$f_0=1$$ | $$O(n\log n)$$ |
| `f.exp(deg = -1)` | 指数。$$f_0=0$$ | $$O(n\log n)$$ |
| `f.sqrt(deg = -1)` | 平方根。存在しなければ空配列 | $$O(n\log n)$$ |
| `f.pow(k)` | `k >= 0` の整数乗。`k` は `long long` | $$O(n\log n+\log k)$$ |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分 | $$O(n)$$ |
| `f.eval(x)` | 一点評価 | $$O(n)$$ |
| `f.onemul(d,c)` / `f.onediv(d,c)` | 自身に $$1+cx^d$$ を掛ける／割る。返り値は `void`。`d >= 0`。除算で `d = 0` なら `c != -1` | $$O(n)$$ |

指定長 0 の `inv`・`log`・`exp`・`sqrt`・`div_series` は空配列です。正の長さの `inv`・`log` には非空の入力が必要です。空列の `exp(n)` は長さ `n` の `{1,0,...}` です。`pow(0)` は元の長さの `{1,0,...}` を $$O(n)$$ 時間で返し、元が空列なら空列です。

平方根は入力を `deg` 係数で打ち切って扱い、先頭の非零係数の平方根のうち通常の整数値が小さい方を選びます。先頭の非零係数の次数が奇数、またはその係数を 2 乗で表す値が法 998244353 上に存在しなければ空配列を返します。零級数の平方根は指定長の零列です。

`inv`・`log`・`exp`・`sqrt`・`pow`・`div_series` の出力長は $$2^{22}$$ 以下です。`divmod` は末尾の零を除いた被除数の係数数を $$2^{22}$$ 以下にします。変換長は最大 $$2^{23}$$ です。各操作の途中の積も含め、素数を法とした変換（NTT）に必要な長さがこの上限以内であることが必要です。法 998244353 の整数の逆元を用いるため、積分・対数・指数などの次数は法未満にします。

Taylor shift・Bostan–Mori・線形漸化式・多点評価・補間は [軽量 FPS の追加演算](https://qandori.github.io/cp-library/fps/fps_light_algorithms.hpp.html) にあります。合成・合成逆元・power projection・多項式 GCD も、それぞれ `fps/compose_light.hpp`・`fps/compositional_inverse_light.hpp`・`fps/power_projection_light.hpp`・`fps/polynomial_gcd_light.hpp` にあります。いずれも通常版と同じ関数名で呼び出せます。`light_fps_algorithms` 名前空間でも利用できます。

返す係数列を含む各操作の追加空間は $$O(n)$$ 以内です。`eval`・`onemul`・`onediv`・`shrink` と、その場で行う定数倍・シフトは $$O(1)$$ の追加空間で処理します。係数列自体は `capacity()` に比例した領域を持ち、`shrink` は確保済みの容量を減らしません。

共有する NTT の根の表は、これまでに使用した最大変換長を $$L$$ として $$O(L)$$ 空間を保持します。整数逆元の表は、これまでに要求した最大長を $$I$$ として $$O(I)$$ 空間を保持し、拡張時は新しい長さに対して $$O(I)$$ 時間以内を使います。各表は操作をまたいで再利用されます。
