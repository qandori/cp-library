---
title: "軽量 FPS の追加演算"
documentation_of: //fps/fps_light_algorithms.hpp
library_category: fps
---

表の $$[x^k]h(x)$$ は、$$h(x)$$ の $$x^k$$ の係数を表します。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

FPS の本体と追加演算は、同じ版のヘッダーを選んでください。

```cpp
#include "lib/fps/fps_light_algorithms.hpp"

fps f{1, 2, 3};
auto shifted = taylor_shift(f, mint(2)); // {17,14,3}
std::vector<mint> xs{0, 1, 2};
auto ys = multipoint_eval(f, xs); // {1,6,17}
auto restored = interpolate(xs, ys); // {1,2,3}

std::vector<mint> initial{0, 1}, coefficients{1, 1};
auto value = linear_recurrence(initial, coefficients, 10); // 55
```

[軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) の追加演算です。通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法は 998244353 で、係数型は `FormalPowerSeries<T>` と同じ条件です。

係数数を $$n$$、点数を $$m$$、Bostan–Mori の分子・分母の最大係数数または漸化式の次数を $$d$$ とします。

| 関数 | 内容・条件 | 時間計算量 |
| --- | --- | --- |
| `taylor_shift(f,c)` | $$f(x+c)$$ の係数列を `FormalPowerSeries<T>` で返す | $$O(n\log n)$$ |
| `bostan_mori(p,q,k)` | $$[x^k]p(x)/q(x)$$ を返す。$$q_0\ne0$$、`k >= 0` | $$O(d\log d\log k)$$ |
| `linear_recurrence(initial,c,k)` | $$a_n=c_0a_{n-1}+\cdots+c_{d-1}a_{n-d}$$ の $$a_k$$ を返す。初期値・係数は各 `d` 個、`k >= 0` | $$O(d\log d\log k)$$ |
| `multipoint_eval(f,xs)` | `xs` の順に値を `std::vector<T>` で返す。点の重複を許す | $$O((n+m)\log^2(n+m))$$ |
| `interpolate(xs,ys)` | $$f(x_i)=y_i$$ を満たす次数が点数未満の係数列を `FormalPowerSeries<T>` で返す | $$O(m\log^2 m)$$ |

係数列は定数項から順です。線形漸化式の `c[0]` は一つ前の項に掛ける係数、`k` は 0 始まりの項番号です。`initial` と `c` は `std::vector<T>` で渡します。次数 0 の漸化式は 0 を返します。

多点評価は入力順を保ち、空の点列では空配列です。補間は `xs` と `ys` の長さが等しく、`xs` が法上で相異なることが必要です。空の補間は空配列、補間結果の長さは点数です。入力の点をソートする必要はありません。

演算中に必要となる、素数を法とした変換（NTT）の長さは $$2^{23}$$ 以下である必要があります。

合成・合成逆元・射影・多項式 GCD は `fps/compose_light.hpp`・`fps/compositional_inverse_light.hpp`・`fps/power_projection_light.hpp`・`fps/polynomial_gcd_light.hpp` に分けています。同じ軽量 FPS を使い、通常版と同じ関数名で必要な演算だけを追加できます。

Taylor shift は $$O(n)$$、Bostan–Mori と線形漸化式は $$O(d)$$、多点評価は $$O((n+m)\log(n+m))$$、補間は $$O(m\log m)$$ の追加空間です。Bostan–Mori の `k == 0` は入力のコピー・末尾の零の除去に $$O(d)$$ 時間を使います。線形漸化式で既知の初期項を返す場合は $$O(1)$$ 時間・追加空間、次数 1 は $$O(\log k)$$ 時間・$$O(1)$$ 追加空間です。係数の四則演算を定数時間とします。軽量 FPS と共有する NTT の根・整数逆元の表は、使用した最大長に比例する空間を保持します。
