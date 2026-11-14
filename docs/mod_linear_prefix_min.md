---
title: Mod of Linear の Prefix Minimum の内部走査
documentation_of: //components/mod_linear_prefix_min.hpp
library_category: components/math
---

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `mod_linear_detail::prefix_min<U>(n,m,a,b,emit)` | 更新点を圧縮し、各区間で `emit(x,y,dx,dy,count)` を呼ぶ | $$O(\log m)$$ |

`math/prefix_min_of_mod_of_linear.hpp` が使う、整数幅に応じた内部処理です。通常は公開側を使用します。

`mod_linear_detail::prefix_min<U>(n, m, a, b, emit)` は、`U` を `std::uint32_t` または `std::uint64_t` とし、$$1\le n\le m$$、$$m>0$$、$$0\le a,b<m$$ を前提にします。
`emit(x, y, dx, dy, count)` を更新点の等差数列ごとに呼びます。各引数は `U` で、区間の意味は公開側と同じです。

時間 $$O(\log m)$$、追加空間 $$O(1)$$ です。コールバック1回を $$O(1)$$ とします。
