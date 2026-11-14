---
title: XOR の最小値・最大値を求める整数集合
documentation_of: //data_structure/xor_set.hpp
library_category: data_structure/associative
---

整数の追加・削除と、指定値との XOR が最小／最大になる要素の取得に対応する集合です。同じ値を追加しても重複しません。

```cpp
#include "lib/data_structure/xor_set.hpp"

XorSet<30> values;
values.insert(3);
values.insert(10);
auto a = values.argmin_xor(8); // 10
auto b = values.min_xor(8);    // 2
auto c = values.argmax_xor(8); // 3
auto d = values.max_xor(8);    // 11
values.erase(3);
```

`XorSet<Bits>` の `Bits` は既定で 30、範囲は 1〜31 です。格納できる値は `uint32_t` の `[0,2^Bits)`、XOR の問い合わせには任意の `uint32_t` を渡せます。

| 操作 | 内容・返り値 |
| --- | --- |
| `XorSet<Bits>()` | 空集合を作成 |
| `size()` / `empty()` | 要素数を `int64_t` で返す／空判定 |
| `contains(x)` | `x` が含まれるか |
| `insert(x)` | `x` を追加し、追加できた場合に `true` |
| `erase(x)` | `x` を削除し、含まれていた場合に `true` |
| `argmin_xor(x)` / `argmax_xor(x)` | `x` との XOR が最小／最大になる集合の要素を `uint32_t` で返す |
| `min_xor(x)` / `max_xor(x)` | `x` と集合の要素との XOR の最小値／最大値を `uint32_t` で返す |
| `clear()` | 容量を保って空集合にする |
| `swap(other)` | 内容を交換 |
| `memory_usage()` | 確保済みの動的領域のバイト数 |

`argmin_xor`・`argmax_xor`・`min_xor`・`max_xor` は空でない集合に対して呼んでください。コピーは独立した集合を作り、移動元は再利用できる空集合になります。

$$U=2^{Bits}$$、これまでに確保した葉の最大数を $$L$$ とします。最初の挿入は管理領域の確保に $$O(U/512)$$ 時間、その後の挿入は償却 $$O(\log_{64}U)$$ 時間、削除と XOR の問い合わせは $$O(\log_{64}U)$$ 時間です。`contains`・`size`・`empty`・交換は $$O(1)$$、コピーと `clear` は $$O(U/512+L)$$ 時間、使用空間は $$O(U/512+L)$$ 個の固定長整数です。`Bits` が 9 以下なら管理領域は定数サイズです。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
