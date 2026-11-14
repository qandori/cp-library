---
title: "Small Vector"
documentation_of: //components/small_vector.hpp
library_category: components/data_structure
---

```cpp
#include "lib/components/small_vector.hpp"
SmallVector<unsigned long long, 2> a;
a.push_back(123);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SmallVector()` / `SmallVector(n)` / `SmallVector{...}` / `SmallVector(first,last)` | 空／長さ指定／初期値列／イテレータ範囲から構築 | 空: $$O(1)$$、要素指定: $$O(n)$$ |
| `size()` / `capacity()` / `empty()` | 要素数／確保容量／空判定 | $$O(1)$$ |
| `data()` / `begin()` / `end()` | 先頭ポインタ／先頭・末尾イテレータ | $$O(1)$$ |
| `operator[](i)` / `back()` | 要素／末尾の参照。const 版も利用可 | $$O(1)$$ |
| `reserve(n)` | 容量を確保 | $$O(n)$$ |
| `resize(n)` / `resize_for_overwrite(n)` | サイズ変更。前者は追加要素を 0 初期化、後者は初期化を省略 | $$O(n)$$ |
| `clear()` | 容量を残して空にする | $$O(1)$$ |
| `push_back(x)` / `pop_back()` | 末尾への追加／末尾の削除 | push_back: 償却 $$O(1)$$、pop_back: $$O(1)$$ |
| `assign(n,x)` / `assign(first,last)` | 同じ値を `n` 個／イテレータ範囲で置換 | $$O(n)$$ |
| `insert(pos,count,x)` / `insert(pos,first,last)` | `pos` の直前に複数要素を挿入し、挿入位置を返す | 移動・追加する要素数に比例 |
| `erase(first,last)` | ポインタ範囲を削除し、直後の要素の位置を返す | 移動する要素数に比例 |
| `a == b` / `a != b` | 要素列を比較 | $$O(n)$$ |

`SmallVector<T, N>` は先頭 $$N$$ 要素分を内部に持つ可変長配列です。`T` は `std::is_trivially_copyable_v<T>` と `std::is_trivially_destructible_v<T>` がともに `true` の型、$$N>0$$ が必要です。これは `int` など、コピーや破棄に特別な処理を必要としない型を表します。時間計算量では $$N$$ と `sizeof(T)` を定数とします。`size`・`capacity`・`empty`・`data`・`begin`・`end`・`[]`・`back`・`reserve`・`resize`・`clear`・`push_back`・`pop_back`・`assign`・`insert`・`erase` に対応します。範囲の `assign`・`insert` に自身の要素を渡さないでください。

`resize_for_overwrite(n)` は要素の初期化を省略してサイズを変更します。追加した要素は代入してから参照してください。

要素数を $$n$$ とすると、参照・末尾削除は $$O(1)$$、末尾追加は償却 $$O(1)$$、再確保・コピー・中間の挿入削除は $$O(n)$$ です。`capacity()` の値を $$C$$ とすると保持空間は $$O(C)$$ です。$$C\ge N$$ で、削除や `clear()` では容量を解放せず、`reserve(n)` で要素数より大きい容量を確保することもできます。再確保中は新旧の領域を同時に持ち、確保後の容量に比例する空間を使います。

償却計算量は、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。
