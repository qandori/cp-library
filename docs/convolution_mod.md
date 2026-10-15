---
title: "任意 mod・整数の畳み込み"
documentation_of: //convolution/convolution_mod.hpp
---

```cpp
#include "lib/convolution/convolution_mod.hpp"

std::vector<long long> a{1, 2, 3}, b{4, 5};
auto c = convolution_mod(a, b, 1000000007);
auto exact = convolution_integer(a, b);
```

どちらも `Σ_{i+j=k} a[i]*b[j]` を計算します。入力は 64 bit 以下の整数型です。

`convolution_mod` は `1 <= mod <= 2^32-1` とし、`std::vector<std::uint32_t>` を返します。`convolution_integer` は入力と同じ係数型を返し、結果がその型に収まる必要があります。出力長は `2^24` 以下です。

`L = a.size()+b.size()` として、時間 `O(L log L)`、追加空間 `O(L)` です。
