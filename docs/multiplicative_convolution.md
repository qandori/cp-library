---
title: "素数 mod 上の乗法畳み込み"
documentation_of: //convolution/multiplicative_convolution.hpp
---

```cpp
#include "lib/convolution/multiplicative_convolution.hpp"

std::vector<mint> a{1, 2, 3, 4, 5}, b{6, 7, 8, 9, 10};
auto c = multiplicative_convolution(a, b);
```

入力は同じ長さの素数 `P` 個とし、`c[k] = Σ_{i*j mod P=k} a[i]*b[j]` を返します。係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

原始根探索後、通常の長さ `P` の畳み込み時間を `M(P)` として、時間 `O(M(P)+P)`、追加空間 `O(P)` です。原始根探索では `P-1` の素因数分解と候補ごとの `O(log² P)` の判定を行います。
