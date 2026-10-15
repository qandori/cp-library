---
title: "FFT"
documentation_of: //convolution/fft.hpp
---

```cpp
#include "lib/convolution/fft.hpp"

fft(a);
fft(a, true);
```

`a` は `std::vector<std::complex<Real>>`、長さは 2 の冪です。順変換は正の偏角、逆変換は負の偏角を使い、逆変換には長さでの除算を含みます。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。
