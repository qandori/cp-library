---
title: "通常の畳み込み"
documentation_of: //convolution/convolution.hpp
---

```cpp
#include "lib/convolution/convolution.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5};
auto c = convolution(a, b);
```

`c[k] = Σ_{i+j=k} a[i]*b[j]` を返します。

| 係数型 | 実装・条件 |
| --- | --- |
| `mint`・mod 998244353 の modint | 既存 FPS の NTT。出力長 `<= 2^23` |
| その他の modint | 32 bit 以下の mod。`mod()`・`val()`・`raw()` が必要。出力長 `<= 2^24` |
| 64 bit 以下の整数 | CRT による厳密計算。結果が係数型に収まること。出力長 `<= 2^24` |
| 浮動小数点・`std::complex` | FFT による近似計算 |
| その他の環 | Karatsuba。`T(0)`・`+=`・`-=`・`*` が必要 |

[各種畳み込みの参考記事](https://noshi91.hatenablog.com/entry/2020/10/27/175112)。添字・集約・要素同士の演算は各ヘッダーの説明に記載しています。

`L = a.size()+b.size()` として、NTT・CRT・FFT は時間 `O(L log L)`、その他の環は `O(L^log₂3)`、追加空間はいずれも `O(L)` です。
