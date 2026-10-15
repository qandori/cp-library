---
title: "i < j 制約付き畳み込み"
documentation_of: //convolution/ordered_convolution.hpp
---

```cpp
#include "lib/convolution/ordered_convolution.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
auto c = ordered_convolution(a, b);
```

`c[k] = Σ_{i+j=k, i<j} a[i]*b[j]`。係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

`L = a.size()+b.size()` として、NTT・CRT・FFT では時間 `O(L log² L)`、一般の環では `O(L^log₂3)`、追加空間はいずれも `O(L)` です。
