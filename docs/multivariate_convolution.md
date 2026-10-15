---
title: "多変数の切り捨て畳み込み"
documentation_of: //convolution/multivariate_convolution.hpp
---

```cpp
#include "lib/convolution/multivariate_convolution.hpp"

std::vector<mint> a{1, 2, 3, 4, 5, 6}, b{7, 8, 9, 10, 11, 12};
auto c = multivariate_convolution(a, b, {2, 3});
```

多項式の積を `(x₀^d₀, ..., xₛ^dₛ)` で切り捨てます。各 `d > 0`、入力長は `N = Πd` とし、添字は `i₀ + d₀*i₁ + d₀*d₁*i₂ + ...` の順です。

入力の次元数を `K`、1 より大きい次元数を `D`、通常の長さ `L` の畳み込み時間を `M(L)` とします。mod 998244353 では `N <= 2^22`、時間 `O(K + DN log N + D²N)`、その他は時間 `O(K + M(ND))` です。係数型・変換長の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じで、追加空間はいずれも `O(ND)` です。`N=1` は時間 `O(K+1)`、追加空間 `O(1)` です。
