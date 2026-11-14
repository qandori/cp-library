---
title: "巡回・負巡回畳み込み（軽量実装）"
documentation_of: //convolution/cyclic_convolution_light.hpp
library_category: convolution
---

通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法 998244353 の計算には軽量 FPS を使います。

```cpp
#include "lib/convolution/cyclic_convolution_light.hpp"

std::vector<long long> a{1, 2, 3}, b{4, 5};
auto c = cyclic_convolution(a, b, 3); // {19,13,22}
auto d = cyclic_convolution<true>(a, b, 3); // {-11,13,22}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `cyclic_convolution(a,b,N)` | 積を $$x^N-1$$ で割った余りの `N` 個の係数 | $$O(\lvert a\rvert+\lvert b\rvert+M(N))$$ |
| `cyclic_convolution<true>(a,b,N)` | 積を $$x^N+1$$ で割った余りの `N` 個の係数 | $$O(\lvert a\rvert+\lvert b\rvert+M(N))$$ |

普通の積の係数 `{4,13,22,15}` にある $$15x^3$$ は、`N=3` の巡回版では定数項へ足し、負巡回版では定数項から引きます。出力は常に長さ `N`、入力は定数項から順に並べます。入力が `N` より長い場合も同じ規則で折り返します。

`cyclic_convolution(a,b,N)` は積を $$x^N-1$$ で、`<true>` は $$x^N+1$$ で割った余りを $$N$$ 個の係数で返します。$$N > 0$$ とし、係数型の条件は [通常の畳み込み](https://qandori.github.io/cp-library/convolution/convolution.hpp.html) と同じです。

通常の長さ $$N$$ の畳み込み時間を $$M(N)$$ として、時間 $$O(\lvert a\rvert+\lvert b\rvert+M(N))$$、追加空間 $$O(N)$$ です。
