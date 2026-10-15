---
title: "巡回・負巡回畳み込み"
documentation_of: //convolution/cyclic_convolution.hpp
---

```cpp
#include "lib/convolution/cyclic_convolution.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5};
auto c = cyclic_convolution(a, b, 3);
auto d = cyclic_convolution<true>(a, b, 3);
```

`cyclic_convolution(a,b,N)` は積を `x^N-1` で、`<true>` は `x^N+1` で割った余りを `N` 個の係数で返します。`N > 0` とし、係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

通常の長さ `N` の畳み込み時間を `M(N)` として、時間 `O(a.size()+b.size()+M(N))`、追加空間 `O(N)` です。
