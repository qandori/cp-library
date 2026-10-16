---
title: "Walsh–Hadamard Transform"
documentation_of: //components/walsh_hadamard.hpp
redirect_from: /convolution/walsh_hadamard.hpp.html
---

```cpp
#include "lib/components/walsh_hadamard.hpp"

walsh_hadamard(a);
```

長さは 2 の冪とします。同じ変換をもう一度適用し、各要素を長さで割ると逆変換になります。

長さを $$n$$ として時間は $$O(n\log n)$$、追加空間は $$O(1)$$ です。要素の演算は $$O(1)$$ とします。
