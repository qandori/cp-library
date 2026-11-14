---
title: "形式的冪級数"
documentation_of: //fps/fps.hpp
library_category: fps
---

形式的冪級数（FPS）は、$$f_0+f_1x+f_2x^2+\cdots$$ を係数列として扱うものです。このライブラリでは必要な個数まで係数を計算し、実数の無限和が収束するかどうかは考えません。

```cpp
#include "lib/fps/fps.hpp"
#include <cassert>

int main() {
	fps f{1, 2, 3}, g{4, 5};
	fps product = f * g;
	fps inverse = f.inv(10);
	assert((product == fps{4, 13, 22, 15}));
	assert(inverse.size() == 10 && inverse[0] == mint(1));
}
```

mod は 998244353、係数は定数項から順に格納します。`fps` は `std::vector<mint>` と同様に扱えます。`deg` は返す係数の個数で、省略時は元のサイズです。

`FormalPowerSeries<T>` の `T` は `mod()` が定数式となる法 998244353 の modint 型です。整数からの構築、`mod()`・`raw()`・`val()`・`inv()`・`pow()`、四則演算と等値比較が必要です。例えば `<atcoder/modint>` を別途 include すると、`FormalPowerSeries<atcoder::modint998244353>` を使えます。

通常の `oj-bundle` で提出コードを生成できます。FPS 本体は `fps/fps.hpp`、合成・補間・GCD などの追加演算はそれぞれの公開ヘッダーを include します。必要な依存だけを展開し、共通部品は一度だけ出力します。

```sh
oj-bundle -I . a.cpp > submit.cpp
wc -c submit.cpp
```

通常版は各 FPS 公開ヘッダーの API 使用例と基本の `FastInput`・`FastOutput` を合わせて、通常展開のまま 256 KiB 未満になることを検査しています。軽量版は派生演算も含めて軽量 FPS に統一し、全 FPS 演算と同じ入出力を 64 KiB 未満で使えます。複数の通常版ライブラリを併用する場合や解答本体を追加した場合は、最終ソースのサイズを確認してください。[提出コードの生成](https://github.com/qandori/cp-library/blob/main/docs/bundle.md)を参照してください。

`fps{1,2,3}` は $$1+2x+3x^2$$ を表します。係数 `f[i]` は `mint` なので、整数として取り出すには `f[i].val()` を使います。`f.inv(10)` などの 10 は最高次数ではなく返す長さで、$$x^0$$ から $$x^9$$ までを計算します。

`f.pow(k)`・微分・積分・シフトは元の長さで打ち切ります。例えば `fps{1,1}.pow(3)` は `{1,3}` です。4 係数が必要なら `f.resize(4)` としてから `f.pow(3)` を呼びます。一方、通常の積 `f*g` は両入力が非空なら長さ `f.size()+g.size()-1` を返します。

`f/g` は指定長までの級数の除算です。多項式の割り算の商と余りを求める場合は `auto [q,r] = f.divmod(g)` とします。これらの戻り値は新しい配列で、元の `f` を更新する演算には `*=` などの複合代入を使います。

`pre(n)` は高次の係数を切り落とすだけで、零を補って長くする操作ではありません。長さを伸ばすときは `resize(n)` を使います。加減算は長い方の入力長、定数との加減算は空列なら長さ 1 になり、それ以外は元の長さを保ちます。`divmod` の商・余りは末尾の零を除き、零多項式は空配列で返します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps()` / `fps(n)` / `fps{a0,a1,...}` / `fps(vector<mint>)` | 空列／長さ指定／係数列から構築 | 空: $$O(1)$$、長さ指定: $$O(n)$$ |
| `f = a` / `f = {a0,a1,...}` | 配列／初期値列を代入 | $$O(n)$$ |
| `f[i]` / `f.size()` / `f.resize(n)` / `f.empty()` | `std::vector<mint>` と同じ要素取得・サイズ操作。イテレータや `push_back` なども利用可 | 参照: $$O(1)$$、resize: $$O(n)$$ |
| `fps::multiply(f,g)` / `f * g` | 多項式の積 | $$O(n\log n)$$ |
| `-f` / `f + g` / `f - g` | 符号反転／加算／減算 | $$O(n)$$ |
| `f + c` / `f - c` / `f * c` / `f / c` | 定数項への加減算／全係数の定数倍・除算。除数は非零 | $$O(n)$$ |
| `f += g` / `f -= g` / `f *= g` / `f /= g` / `f %= g` | 演算結果を代入。定数との複合代入も利用可 | 加減算: $$O(n)$$、乗除算・剰余: $$O(n\log n)$$、定数項の加減算: $$O(1)$$ |
| `f << d` / `f >> d` / `f <<= d` / `f >>= d` | 係数列をずらし、元のサイズを保つ。`d >= 0` | $$O(n)$$ |
| `f.pre(n)` | 先頭から高々 `n` 個の係数を返す。`n >= 0` | $$O(n)$$ |
| `f / g` / `f.div_series(g,deg = -1)` | 級数の除算。$$g_0 \ne 0$$ | $$O(n\log n)$$ |
| `f.divmod(g)` / `f % g` | 多項式の商と余り／余り。$$g \ne 0$$ | $$O(n\log n)$$ |
| `f.inv(deg = -1)` | 逆数。$$f_0 \ne 0$$ | $$O(n\log n)$$ |
| `f.log(deg = -1)` / `f.exp(deg = -1)` | 対数／指数。定数項はそれぞれ 1／0 | $$O(n\log n)$$ |
| `f.sqrt(deg = -1)` | 平方根。存在しなければ空配列 | $$O(n\log n)$$ |
| `f.pow(k)` | `long long` の非負整数乗。元のサイズで打ち切る | $$O(n\log n+\log k)$$ |
| `f.diff()` / `f.integral()` | 微分／定数項 0 の積分。元のサイズを保つ | $$O(n)$$ |
| `f.eval(x)` | 一点評価 | $$O(n)$$ |
| `f.onemul(d,c)` / `f.onediv(d,c)` | 自身に $$1+cx^d$$ を掛ける／割る。返り値は `void`。`d >= 0`、自身のサイズを保つ。除算で `d = 0` なら `c != -1` | $$O(n)$$ |

指定長 0 の `inv`・`log`・`exp`・`sqrt`・`div_series` は空配列を返します。正の長さの `inv`・`log` には定数項を持つ非空の入力が必要です。空列の `exp(n)` は長さ `n` の `{1,0,...}`、零級数の `sqrt(n)` は長さ `n` の零列です。`pow(0)` も元の長さの `{1,0,...}` を返し、元が空列なら空列です。

`sqrt(deg)` は入力を `deg` 係数で打ち切って扱います。その範囲の先頭の非零係数の次数が奇数、またはその係数を 2 乗で表す値が法 998244353 上に存在しなければ空配列です。平方根が存在する場合は一つを返します。零級数は失敗ではなく指定長の零列なので、`deg > 0` なら空配列かどうかで区別できます。

係数数と長さ引数は `int` の範囲とし、積の計算に使う、素数を法とした変換（NTT）の長さは $$2^{23}$$ 以下にしてください。全ての入力長と指定する出力長を $$2^{22}$$ 以下にすると、基本演算はこの変換長に収まります。積分・対数・指数・冪で使う整数の逆元のため、次数は法未満とします。

[Bostan–Mori](https://qandori.github.io/cp-library/fps/bostan_mori.hpp.html)、[線形漸化式](https://qandori.github.io/cp-library/fps/linear_recurrence.hpp.html)、[合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)、[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html)、[多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html)、[補間](https://qandori.github.io/cp-library/fps/interpolate.hpp.html)、[Taylor shift](https://qandori.github.io/cp-library/fps/taylor_shift.hpp.html) の使い方は、各ページを参照してください。

入力・出力の最大係数数を $$n$$ とします。

| 操作 | 時間計算量 |
| --- | --- |
| 加減算・定数との演算・`diff`・`integral`・`onemul`・`onediv`・一点評価 | $$O(n)$$ |
| 乗算・級数除算・多項式除算・`inv`・`log`・`exp`・`sqrt` | $$O(n \log n)$$ |
| `pow(k)` | $$O(n \log n + \log k)$$ |

追加空間は $$O(n)$$ です。共有する NTT と整数逆元の表は、それまでに使った最大の長さに応じて領域を保持します。
