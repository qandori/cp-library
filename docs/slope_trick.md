---
title: "Slope Trick"
documentation_of: //data_structure/slope_trick.hpp
library_category: data_structure
---

傾きが左から右へ減少しない折れ線の関数を保持します。絶対値関数の加算や横方向への移動を行い、関数の最小値とその位置を求められます。

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

初期状態は $$f(x)=0$$ です。座標型・値の型は `SlopeTrick<T, Sum>` で指定し、既定はともに `std::int64_t` です。座標と横方向の移動量の計算は `T`、関数値の計算は `Sum` に収めてください。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SlopeTrick()` | $$f(x)=0$$ で初期化 | $$O(1)$$ |
| `add_all(c)` | $$f(x)+c$$ | $$O(1)$$ |
| `add_a_minus_x(a)` | $$f(x)+\max(a-x,0)$$ | 償却 $$O(\log n)$$ |
| `add_x_minus_a(a)` | $$f(x)+\max(x-a,0)$$ | 償却 $$O(\log n)$$ |
| `add_abs(a)` | $$f(x)+\lvert x-a\rvert$$ | 償却 $$O(\log n)$$ |
| `shift(a)` | $$f(x-a)$$ | $$O(1)$$ |
| `shift(a,b)` | $$\displaystyle \min_{x-b\le y\le x-a}f(y)$$、$$a\le b$$ | $$O(1)$$ |
| `clear_right()` | $$\displaystyle \min_{y\le x}f(y)$$ | $$O(1)$$ |
| `clear_left()` | $$\displaystyle \min_{y\ge x}f(y)$$ | $$O(1)$$ |
| `min()` | 最小値 | $$O(1)$$ |
| `argmin()` | 最小値を取る閉区間の両端。無限側は `nullopt` | $$O(1)$$ |
| `eval(x)` | $$f(x)$$。関数は変更しない | $$O(n)$$ |
| `merge(g)` / `merge(std::move(g))` | $$f(x)+g(x)$$。`g` は初期状態に戻る | 償却 $$O(\min(n,m)\log(n+m))$$ |
| `size()` / `empty()` | 傾きの変化を記録した点数（重複込み）／記録した点がないか | $$O(1)$$ |
| `reserve(count)` / `clear()` | 左右にそれぞれ `count` 点分を予約／容量を残して初期状態に戻す | reserve: 再確保時 $$O(n)$$、容量が十分なら $$O(1)$$。clear: $$O(1)$$ |
| `swap(g)` | 関数と確保領域を交換 | $$O(1)$$ |

変化点数を $$n$$、併合する関数の変化点数を $$m$$ とします。`merge` の時間は領域の再確保を含む償却計算量です。`f.merge(f)` では先に自分自身をコピーするため、$$O(n)$$ 時間・追加空間も使います。この場合、`f` は初期状態には戻らず、元の関数を 2 倍した関数になります。$$n\le1$$ での単独の操作と、どちらかが空の関数との併合は定数時間です。

左右の領域の確保容量の合計を $$C$$ として、保持空間は $$O(C)$$ です。`reserve` や `clear_left`・`clear_right`・`clear` の後も容量を保持します。コピーは $$O(n)$$ 時間・追加空間、`eval` の追加空間は $$O(1)$$ です。演算と要素のコピーは $$O(1)$$ とします。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
