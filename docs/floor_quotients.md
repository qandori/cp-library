---
title: 整数商の列挙・添字
documentation_of: //components/floor_quotients.hpp
library_category: components/math
---

```cpp
#include "lib/components/floor_quotients.hpp"

FloorQuotients table(10);
auto values = table.values(); // {10,5,3,2,1}
auto i = table.index(5);      // 1
auto j = table.divided_index(0, 3); // values[j] == 10/3 == 3
```

`FloorQuotients` は、正の整数 $$i$$ に対する $$\displaystyle \left\lfloor\frac{N}{i}\right\rfloor$$ の異なる正の値を、降順で保持します。`N = 0` では空です。整数平方根を積で補正し、添字の判定は整数演算で行います。

| 操作 | 内容・返り値 |
| --- | --- |
| `FloorQuotients(n = 0)` | 上限 `n` の表を作る |
| `max_n()` / `root()` | 上限／その整数平方根 |
| `size()` / `values()` | 要素数／降順の `const vector<uint64_t>&` |
| `large_size()` | 整数平方根より大きい要素の個数 |
| `contains(x)` | `x` が表に含まれるか。0 は含まない |
| `index(x)` | 表に含まれる正の `x` の添字 |
| `divided_index(i,d)` | `values()[i] / d` の添字 |
| `swap(other)` | 二つの表を交換する |

`n` は `std::uint64_t` です。`index(x)` の引数は表に含まれる値に限ります。任意の入力を調べる場合は先に `contains(x)` を呼んでください。`divided_index(i,d)` は `i < size()`、`1 <= d <= values()[i]` が必要です。この商も必ず表に含まれます。

コピーは独立し、ムーブ後の元の表は上限 0 の空表になります。巨大な `n` では表自身が大きくなるため、確保できるメモリの範囲で使ってください。

`FloorQuotientRange(n = 0)` は同じ商を昇順に列挙する、配列を保持しない範囲です。

```cpp
#include "lib/components/floor_quotients.hpp"
#include <iostream>

FloorQuotientRange range(10);
for (auto x : range) std::cout << x << ' '; // 1 2 3 5 10
```

| 操作 | 内容・返り値 |
| --- | --- |
| `FloorQuotientRange(n = 0)` | `uint64_t` の上限 `n` に対する範囲を作る |
| `max_n()` / `root()` / `large_size()` | 上限／整数平方根／整数平方根より大きい商の個数 |
| `size()` / `empty()` | 要素数を `uint64_t`、空かどうかを `bool` で返す |
| `range[i]` | `0 <= i < size()` に対する昇順の第 `i` 要素を `uint64_t` で返す |
| `begin()` / `end()` | 前方走査用のイテレータ。間接参照の返り値は `uint64_t` の値 |

`n = 0` では空です。`uint64_t` 全域に対応し、`int64_t` を超える商もそのまま符号なし整数で返します。イテレータの走査中は元の範囲を保持してください。

`FloorQuotients` の構築・コピーは $$\displaystyle O(\sqrt n)$$ 時間・空間、参照・添字取得・交換・ムーブ構築は $$O(1)$$ 時間です。ムーブ代入では代入先の旧領域を解放します。

`FloorQuotientRange` の構築・コピー・各参照・イテレータの取得・増分は $$O(1)$$ 時間・空間、全要素の列挙は $$\displaystyle O(\sqrt n)$$ 時間です。
