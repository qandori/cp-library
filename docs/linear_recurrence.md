---
title: "線形漸化式の第 k 項"
documentation_of: //fps/linear_recurrence.hpp
library_category: fps
---

```cpp
#include "lib/fps/linear_recurrence.hpp"

std::vector<mint> initial{0, 1}, coefficients{1, 1};
auto value = linear_recurrence(initial, coefficients, 10); // 55
```

初期値と係数は `vector<T>` で渡します。法は 998244353 です。

係数が未知で先頭の数項が分かっている場合は、[線形漸化式の復元](berlekamp_massey.md)で係数を求められます。

連続する複数の項は [有理 FPS の区間係数取得](rational_fps_coefficients.md) でまとめて計算できます。

`initial` は `{a[0],a[1],...,a[d-1]}` の順、`coefficients[0]` は一つ前の項に掛ける係数です。例は `a[0]=0, a[1]=1, a[n]=a[n-1]+a[n-2]` の Fibonacci 数列です。`k` は `long long` の 0 始まりの番号なので、10 を渡すと `a[10]=55` を返します。戻り値は入力と同じ係数型 `T` です。係数型と途中で使う畳み込みの条件は [FPS](fps.md) と同じです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `linear_recurrence(initial,c,k)` | $$a_n=c_0a_{n-1}+\cdots+c_{d-1}a_{n-d}$$ の $$a_k$$。初期値・係数は各 $$d$$ 個、$$k\ge0$$。$$d=0$$ は `0` | $$O(d\log d\log k)$$ |

係数末尾の零は内部で除き、その次数に対応する末尾の初期値を使います。指定済みの初期値と、漸化式を適用し始める位置は維持します。

漸化式の次数を $$d$$ として、時間 $$O(d\log d\log k)$$、追加空間 $$O(d)$$ です。次数 1 は $$O(\log k)$$ 時間・$$O(1)$$ 追加空間です。$$k<d$$ なら指定済みの初期値を $$O(1)$$ 時間・追加空間で返します。係数が全て 0 で、初期値以降を求める場合は $$O(d)$$ 時間・$$O(1)$$ 追加空間です。
