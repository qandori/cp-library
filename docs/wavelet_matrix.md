---
title: "Wavelet Matrix"
documentation_of: //data_structure/wavelet_matrix.hpp
---

```cpp
#include "lib/data_structure/wavelet_matrix.hpp"

WaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
auto value = wm.kth_smallest(0, 5, 2);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の $$k$$ は 0 始まりです。

| 操作 | 内容 |
| --- | --- |
| `access(i)` | 元の配列の $$i$$ 番目 |
| `count(l, r, x)` / `rank(x, r)` | 区間／$$[0,r)$$ にある $$x$$ の個数 |
| `select(x, k)` | $$k$$ 番目の $$x$$ の位置。存在しなければ `-1` |
| `kth_smallest(l, r, k)` / `kth_largest(l, r, k)` | 小さい順／大きい順の $$k$$ 番目 |
| `range_freq(l, r, upper)` | $$x < \mathrm{upper}$$ の個数 |
| `range_freq(l, r, lower, upper)` | $$\mathrm{lower} \le x < \mathrm{upper}$$ の個数 |
| `prev_value(l, r, upper)` / `next_value(l, r, lower)` | $$x < \mathrm{upper}$$ の最大値／$$\mathrm{lower} \le x$$ の最小値。なければ `nullopt` |
| `range_sum(l, r)` | 区間和 |
| `sum_smallest(l, r, k)` / `sum_largest(l, r, k)` | 下位／上位 $$k$$ 個の和 |

和の前処理は必要時に行います。構築時に済ませる場合は `WaveletMatrix<T> wm(a, true)` とします。整数の和は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。`WaveletMatrix<T, Sum>` で和の型を指定できます。

要素数を $$n$$、異なる値の数を $$\sigma$$ として、整数型の構築は $$O(n \log(\sigma))$$、その他の型は $$O(n \log n)$$ 時間です。`access` は $$O(1)$$、`select` は $$O(\log(\sigma))$$、その他の問い合わせは $$O(\log(\sigma))$$ です。

和の前処理は時間・追加空間ともに $$O(n \log(\sigma))$$ で、前処理後の `range_sum` は $$O(1)$$、上下位の和は $$O(\log(\sigma))$$ です。和を使う最初の問い合わせには前処理時間が加わります。全体の空間は和を含めて $$O(n \log(\sigma))$$ です。比較・算術演算は $$O(1)$$ とします。
