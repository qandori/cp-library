---
title: "多項式 GCD（軽量版）"
documentation_of: //fps/polynomial_gcd_light.hpp
library_category: fps
---

```cpp
#include "lib/fps/polynomial_gcd_light.hpp"

fps a{mint(-1), 0, 1};
fps b{mint(-1), 1};
auto g = polynomial_gcd(a, b);
auto [d, x, y] = polynomial_extended_gcd(a, b);
auto inverse = polynomial_inv_mod(fps{1, 1}, fps{1, 0, 1});
```

[軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) の `FormalPowerSeries<T>` を使う多項式 GCD です。係数は定数項から並べ、法は 998244353 とします。既定の `fps`・`mint` に加え、軽量 FPS の要件を満たす同じ法の modint 型も使えます。

表の $$n$$ は入力の最大係数数です。

| 関数・型 | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `polynomial_gcd(a,b)` | 最高次係数を 1 にした最大公約多項式 | $$O(n\log^2 n)$$ |
| `polynomial_extended_gcd(a,b)` | `a*x+b*y=gcd` を満たす `PolynomialExtendedGCD<T>{gcd,x,y}` | $$O(n\log^2 n)$$ |
| `polynomial_inv_mod(a,modulus)` | `a*h ≡ 1 mod modulus` を満たす `optional<FormalPowerSeries<T>>`。逆元がなければ `nullopt` | $$O(n\log^2 n)$$ |

`PolynomialExtendedGCD<T>` の `gcd`・`x`・`y` は `FormalPowerSeries<T>` で、構造化束縛が可能です。結果の保持には $$O(n)$$ 空間を使います。

入力は変更しません。入力・出力とも不要な末尾の 0 を除き、零多項式は空配列とします。`gcd(0,0)` は零多項式、拡張 GCD は三つの空配列を返します。片方だけが 0 なら、もう片方の全係数を最高次係数で割り、最高次係数を 1 にそろえます。

`polynomial_inv_mod` の法多項式は非零にします。法が非零の定数なら、空配列を持つ `optional` を返します。それ以外では GCD が 1 のときに限り逆元が存在し、返り値の次数は法多項式の次数未満です。FPS の `f.inv()` が求める $$x^n$$ を法とする逆数とは別の操作です。

[通常版はこちら](https://qandori.github.io/cp-library/fps/polynomial_gcd.hpp.html)。

入力の最大係数数は $$2^{22}$$ 以下とします。最大係数数を $$n$$、畳み込みの計算量を $$M(n)$$ として、各関数は $$O(M(n)\log n)=O(n\log^2 n)$$ 時間、追加空間は $$O(n)$$ です。
