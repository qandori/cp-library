---
title: 転倒数
documentation_of: //sequence/inversion_count.hpp
library_category: sequence
---

数列の転倒数、すなわち $$i<j$$ かつ $$a_i>a_j$$ を満たす添字の組の個数を返します。同じ値の組は数えません。入力は変更しません。

```cpp
#include "lib/sequence/inversion_count.hpp"

std::vector<int> a{3, 1, 2, 1};
auto count = inversion_count(a);                     // 4
auto increasing = inversion_count(a, std::greater<>{}); // 1
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `inversion_count(a,compare = std::less<>{})` | `i < j` かつ `compare(a[j],a[i])` の組数を `std::int64_t` で返す | $$O(n\log n)$$ |

空列・長さ 1 の列は 0 です。比較関数は `compare(x,y)` が「指定した順序で `x` は `y` より前に来る」を返すものにします。`compare(x,x)` は `false` とし、`compare(x,y)` と `compare(y,z)` が真なら `compare(x,z)` も真にしてください。また、どちら向きの比較も `false` となる2要素を同値としたとき、`x` と `y`、`y` と `z` が同値なら `x` と `z` も同値となる必要があります。`std::greater<>{}` を指定すると、左の値が右の値より小さい組を数えます。比較関数で同値となる要素同士は、どちらの向きでも数えません。

入力は読み取り専用で、要素数の取得と添字による要素の取得ができる列とし、`vector`・`array`・`span`・`deque`・C 配列などを使えます。要素型に応じた比較関数を指定すれば、コピー不能な要素にも対応します。`vector<bool>` も扱えます。入力の長さは `INT_MAX` 以下とします。この範囲では最大の転倒数も符号付き 64 bit に収まります。

長さを $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。単調非減少・単調非増加の列は $$O(n)$$ 時間・$$O(1)$$ 追加空間、空列は $$O(1)$$ 時間・空間です。比較と添字による要素取得を $$O(1)$$ とします。
