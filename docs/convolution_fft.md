---
title: "実数・複素数の畳み込み"
documentation_of: //convolution/convolution_fft.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/convolution_fft.hpp"

std::vector<double> a{1, 2, 3}, b{4, 5};
auto c = convolution_fft(a, b); // 約 {4,13,22,15}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `convolution_fft(a,b)` | 実数または複素数の通常の畳み込みを近似計算 | $$O(L\log L)$$ |

$$\displaystyle c_k = \sum_{i+j=k} a_i b_j$$。係数は浮動小数点型、またはその `std::complex` です。浮動小数点による近似計算です。

返り値は入力と同じ型の `vector` です。両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。整数の厳密な答えが必要な場合は、整数型の配列で `convolution_integer` を使えます。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、時間 $$O(L \log L)$$、追加空間 $$O(L)$$ です。
