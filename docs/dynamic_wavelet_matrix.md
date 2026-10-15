---
title: "Dynamic Wavelet Matrix"
documentation_of: //data_structure/dynamic_wavelet_matrix.hpp
---

```cpp
#include "lib/data_structure/dynamic_wavelet_matrix.hpp"

DynamicWaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
wm.insert(2, -7);
wm.erase(0);
wm.set(1, 6);
auto value = wm.kth_smallest(0, wm.size(), 2);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の順位は 0 始まりです。

| 操作 | 内容 |
| --- | --- |
| `insert(i,x)` / `erase(i)` / `set(i,x)` | 挿入／削除／代入。`erase` は削除した値を返す |
| `push_back(x)` / `pop_back()` | 末尾への挿入／削除 |
| `access(i)` / `operator[]` | 要素の取得 |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ にある $$x$$ の個数 |
| `select(x,k)` | $$k$$ 番目の $$x$$ の位置。なければ `-1` |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の $$k$$ 番目 |
| `range_freq(l,r,upper)` | $$x < \mathrm{upper}$$ の個数 |
| `range_freq(l,r,lower,upper)` | $$\mathrm{lower} \le x < \mathrm{upper}$$ の個数 |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | $$x < \mathrm{upper}$$ の最大値／$$\mathrm{lower} \le x$$ の最小値。なければ `nullopt` |

型 `T` は 64 bit 以下の整数型です。`DynamicWaveletMatrix<T,B>` で値の bit 数を指定でき、既定は `T` の bit 数です。符号付き型の値域は $$[-2^{B-1},2^{B-1})$$、符号なし型は $$[0,2^B)$$ です。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。

要素数を $$n$$、bit 数を $$B$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・問い合わせは $$O(B\log n)$$ です。サイズ取得は $$O(1)$$、`clear` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
