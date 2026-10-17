---
title: "多項式 GCD"
documentation_of: //fps/polynomial_gcd.hpp
---

```cpp
#include "lib/fps/polynomial_gcd.hpp"

fps a{mint(-1), 0, 1};
fps b{mint(-1), 1};
auto g = polynomial_gcd(a, b);              // x-1
auto [d, u, v] = polynomial_extended_gcd(a, b);
auto inverse = polynomial_inv_mod(fps{1, 1}, fps{1, 0, 1});
if (inverse) {
	// (1+x) * *inverse ≡ 1 mod (1+x^2)
}
```

| 関数・型 | 内容・返り値 |
| --- | --- |
| `polynomial_gcd(a,b)` | 最高次係数を 1 にした最大公約多項式 |
| `polynomial_extended_gcd(a,b)` | `PolynomialExtendedGCD<T>{gcd,x,y}`。`a*x+b*y=gcd` を満たす |
| `PolynomialExtendedGCD<T>` | `gcd`・`x`・`y` はいずれも `FormalPowerSeries<T>`。構造化束縛が可能 |
| `polynomial_inv_mod(a,modulus)` | `a*h ≡ 1 mod modulus` を満たす次数最小の代表元 `h` を `optional<FormalPowerSeries<T>>` で返す。存在しなければ `nullopt` |

引数は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `FormalPowerSeries<T>` で、係数は定数項から並べます。法は 998244353 です。既定の `fps`・`mint` に加え、同じ法で `val()`・`raw()` など FPS の要件を満たす別の modint 型も使えます。整数係数・浮動小数点係数の GCD には対応しません。

入力の末尾の 0 は除き、返り値にも不要な末尾の 0 は含みません。零多項式は空配列です。入力は変更しません。

`gcd(0,0)` は零多項式とし、拡張 GCD も三つの空配列を返します。片方だけが 0 の場合は、もう一方を最高次係数 1 に正規化します。非零の定数同士の GCD は `fps{1}` です。

`polynomial_inv_mod` の法多項式は非零にします。法が非零の定数の場合、代表元は零多項式で、空配列を持つ `optional` を返します。それ以外では GCD が 1 のときに限り逆元が存在し、返り値の次数は法多項式の次数未満です。`optional` が値を持つかどうかで存在を判定してください。FPS の `f.inv()` が求める $$x^n$$ を法とする逆数とは別の操作です。

大きい入力では Half-GCD により互除法の操作をまとめ、小さい入力では通常の互除法を使います。変換行列の積では NTT を共有し、行列の作用には次数の低下を利用した巡回畳み込みを使います。必要な NTT 長は $$2^{23}$$ 以下に収めます。最大係数数が $$2^{22}$$ 以下ならこの条件を満たします。

入力の最大係数数を $$n$$ として、各関数は $$O(n\log^2 n)$$ 時間、追加空間は $$O(n)$$ です。
