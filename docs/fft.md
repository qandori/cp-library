---
title: "FFT"
documentation_of: //components/fft.hpp
redirect_from: /convolution/fft.hpp.html
library_category: components
---

複素数列の高速フーリエ変換（FFT）を行います。多項式の畳み込みなどに使う変換です。

```cpp
#include "lib/components/fft.hpp"

std::vector<std::complex<double>> a{1, 2, 3, 4};
fft(a);
fft(a, true);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fft(a,inverse = false)` | 複素数配列をその場で FFT。`true` は長さによる除算を含む逆変換 | $$O(n\log n)$$ |

`a` は `std::vector<std::complex<Real>>`、`Real` は浮動小数点型、長さは 2 の冪です。空配列では何もしません。入力を変換結果で上書きし、返り値は `void` です。順変換は正の偏角、逆変換は負の偏角を使い、逆変換には長さでの除算を含みます。浮動小数点の丸め誤差が生じます。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。
