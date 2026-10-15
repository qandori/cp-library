---
title: "i < j 制約付き畳み込み"
documentation_of: //convolution/ordered_convolution.hpp
---

```cpp
#include "lib/convolution/ordered_convolution.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
auto c = ordered_convolution(a, b);
```

$$c_k = \sum_{\substack{i+j=k \\ i<j}} a_i b_j$$。係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

$$L = \lvert a\rvert + \lvert b\rvert$$ として、NTT・CRT・FFT では時間 $$O(L \log^{2} L)$$、一般の環では $$O(L^{\log_2 3})$$、追加空間はいずれも $$O(L)$$ です。
