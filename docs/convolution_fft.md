---
title: "実数・複素数の畳み込み"
documentation_of: //convolution/convolution_fft.hpp
---

```cpp
#include "lib/convolution/convolution_fft.hpp"

std::vector<double> a{1, 2, 3}, b{4, 5};
auto c = convolution_fft(a, b);
```

`c[k] = Σ_{i+j=k} a[i]*b[j]`。係数は浮動小数点型、またはその `std::complex` です。浮動小数点による近似計算です。

`L = a.size()+b.size()` として、時間 `O(L log L)`、追加空間 `O(L)` です。
