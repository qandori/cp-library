---
title: "Slope Trick"
documentation_of: //data_structure/slope_trick.hpp
---

```cpp
#include "lib/data_structure/slope_trick.hpp"

SlopeTrick<> f;
f.add_abs(3);
f.add_x_minus_a(1);
f.shift(-1, 2);
auto value = f.min();
auto [left, right] = f.argmin();
auto y = f.eval(5);
```

初期状態は $$f(x)=0$$ です。座標型・値の型は `SlopeTrick<T, Sum>` で指定し、既定はともに `std::int64_t` です。座標と遅延値の計算は `T`、関数値の計算は `Sum` に収めてください。

| 操作 | 変更後の関数・返り値 |
| --- | --- |
| `add_all(c)` | $$f(x)+c$$ |
| `add_a_minus_x(a)` | $$f(x)+\max(a-x,0)$$ |
| `add_x_minus_a(a)` | $$f(x)+\max(x-a,0)$$ |
| `add_abs(a)` | $$f(x)+\lvert x-a\rvert$$ |
| `shift(a)` | $$f(x-a)$$ |
| `shift(a,b)` | $$\min_{x-b\le y\le x-a}f(y)$$、$$a\le b$$ |
| `clear_right()` | $$\min_{y\le x}f(y)$$ |
| `clear_left()` | $$\min_{y\ge x}f(y)$$ |
| `min()` | 最小値 |
| `argmin()` | 最小値を取る閉区間の両端。無限側は `std::nullopt` |
| `eval(x)` | $$f(x)$$。関数は変更しない |
| `merge(g)` | $$f(x)+g(x)$$。`g` は初期状態に戻る |

`size()` は傾きの変化点の総数です。`reserve(n)` は左右それぞれに $$n$$ 点分を予約し、`clear()` は確保領域を残して初期状態に戻します。

変化点数を $$n$$ として、折れ線の追加は償却 $$O(\log n)$$、`eval` は $$O(n)$$、その他の操作は $$O(1)$$ です。変化点数 $$m$$ の関数との `merge` は $$O(\min(n,m)\log(n+m))$$、空間は $$O(n)$$ です。コピー・予約は $$O(n)$$、演算と要素のコピーは $$O(1)$$ とします。
