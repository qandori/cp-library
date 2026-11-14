---
title: "素数 mod 上の乗法畳み込み（軽量実装）"
documentation_of: //convolution/multiplicative_convolution_light.hpp
library_category: convolution
---

通常版と同じ関数名で呼び出します。同じ機能は通常版か軽量版のいずれか一方を include してください。法 998244353 の計算には軽量 FPS を使います。

```cpp
#include "lib/convolution/multiplicative_convolution_light.hpp"

std::vector<mint> a{1, 2, 3}, b{4, 5, 6};
auto c = multiplicative_convolution(a, b); // {35,28,27}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `multiplicative_convolution(a,b)` | 同じ素数長 `P` の列から $$\displaystyle c_k=\sum_{ij\bmod P=k}a_ib_j$$ を返す | $$O(M(P)+P)$$＋因数分解・原始根探索 |

添字 `i,j` の積を `P` で割った余りを出力先にします。係数の計算は入力型の加算・乗算です。例は長さ `P=3` なので、`a[2]*b[2]` は `2*2 % 3 = 1` より `c[1]` に加えます。`c` は入力と同じ型の長さ `P` の配列です。

入力は同じ長さの素数 $$P$$ 個とし、$$\displaystyle c_k = \sum_{ij\bmod P=k} a_i b_j$$ を返します。どちらかの入力が空なら空配列を返します。入力は変更せず、同じ配列を両方に渡すこともできます。係数型と畳み込み長の条件は [軽量畳み込み](convolution_light.md) と同じです。

原始根（$$0$$ 乗から $$P-2$$ 乗までが、法 $$P$$ の 1 から $$P-1$$ までの余りを全て一度ずつ取る数）の探索後、通常の長さ $$P$$ の畳み込み時間を $$M(P)$$ として、時間 $$O(M(P)+P)$$、追加空間 $$O(P)$$ です。原始根探索では $$P-1$$ の素因数分解と候補ごとの $$O(\log^{2} P)$$ の判定を行います。

入力長は `uint32_t` に収めます。原始根の探索に使う $$P-1$$ の相異なる素因数は試し割りで求めます。判定する候補数を $$C$$ とすると、準備に $$O(\sqrt P+C\log^2 P)$$ 時間・$$O(\log P)$$ 空間を使います。畳み込みを呼ぶたびにこの準備も行います。
