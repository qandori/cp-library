---
title: "Fenwick Sum"
documentation_of: //components/fenwick_sum.hpp
library_category: components/data_structure
---

```cpp
#include "lib/components/fenwick_sum.hpp"

FenwickSum<long long> bit(std::vector<long long>{3, 1, 4});
bit.add(1, 5);
auto sum = bit.prod(0, 3); // 13
```

区間は $$[l,r)$$、添字は 0 始まりです。`T{}` は加算の 0 を表す値にします。`+=`・`-=` は通常の加算・減算として使え、加算の順序や括弧の位置を変えても結果が変わらず、加えた値を引くと元の値に戻る型を使います。整数や modint などが使えます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `FenwickSum()` / `FenwickSum(n)` | 空／`T{}` が `n` 個の列を構築 | 空: $$O(1)$$、長さ指定: $$O(n)$$ |
| `FenwickSum(a)` / `build(a)` | 配列から構築／再構築 | $$O(n)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `add(i,x)` | 要素に `x` を加算 | $$O(\log n)$$ |
| `add_pair(i,x,j,y)` | 2 点にそれぞれ `x`・`y` を加算 | $$O(\log n)$$ |
| `prefix(r)` | $$[0,r)$$ の和 | $$O(\log n)$$ |
| `prod(l,r)` | 区間和 | $$O(\log n)$$ |
| `prod_as<Accum>(l,r)` | 加減算を `Accum` 型で行い、区間和を返す | $$O(\log n)$$ |

`prod_as` は保持する値の型を変えず、取得時の加減算だけを指定した型で行います。木の各部分和は `T`、取得中の値と結果は `Accum` に収まる必要があります。

要素数を $$n$$ として、構築・空間は $$O(n)$$、更新と和は $$O(\log n)$$、`size` は $$O(1)$$ です。`T` の演算は $$O(1)$$ とします。
