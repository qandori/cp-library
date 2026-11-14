---
title: "通常の畳み込み（軽量実装）"
documentation_of: //convolution/convolution_light.hpp
library_category: convolution
---

2 つの係数列から、多項式の積の係数を求めます。NTT は素数を法とした変換、FFT は複素数などを使う変換で、係数列を変換してから要素ごとに掛け、逆変換して多項式の積を求めます。CRT は、複数の法での余りを同時に満たす値を求める中国剰余定理です。

通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法 998244353 の計算には軽量 FPS を使います。`convolution_mod`・`convolution_integer` も同じ引数・返り値で使えます。両版を include した場合、公開関数は先に include した版を使います。

```cpp
#include "lib/convolution/convolution_light.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5};
auto c = convolution(a, b); // {4,13,22,15}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convolution(a,b)` | 通常の畳み込み $$\displaystyle c_k=\sum_{i+j=k}a_ib_j$$ を返す。係数型から実装を選ぶ | NTT・CRT・FFT: $$O(L\log L)$$、一般の環: $$O(L^{\log_2 3})$$ |

$$\displaystyle c_k = \sum_{i+j=k} a_i b_j$$ を返します。

配列は定数項から順に多項式の係数を並べます。例は $$(1+2x+3x^2)(4+5x)$$ の係数です。返り値は入力と同じ係数型の `vector` で、両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。`mint` の結果を整数に直すには `c[i].val()` を使います。

| 係数型 | 実装・条件 |
| --- | --- |
| `mint`・mod 998244353 の modint | 軽量 FPS の NTT。出力長 $$\le 2^{23}$$ |
| その他の modint | 32 bit 以下の mod。`mod()`・`val()`・`raw()` が必要。出力長 $$\le 2^{24}$$ |
| 64 bit 以下の整数 | CRT による厳密計算。結果が係数型に収まること。出力長 $$\le 2^{24}$$ |
| 浮動小数点・`std::complex` | FFT による近似計算 |
| その他の環 | 積を小さい積に分割する Karatsuba 法。`T(0)`・`+=`・`-=`・`*` が必要 |

表の「その他の環」は、整数・modint と同じような加減算・乗算ができる型です。加算と乗算は括弧の位置を変えても結果が変わらず、加算は順序を入れ替えられ、乗算は和に分配できる必要があります。このような演算を持つ型を環と呼び、除算は要求しません。

[各種畳み込みの参考記事](https://noshi91.hatenablog.com/entry/2020/10/27/175112)。添字・集約・要素同士の演算は各ヘッダーの説明に記載しています。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、NTT・CRT・FFT は時間 $$O(L \log L)$$、その他の環は $$O(L^{\log_2 3})$$、追加空間はいずれも $$O(L)$$ です。
