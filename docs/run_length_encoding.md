---
title: 連長圧縮（RLE）
documentation_of: //sequence/run_length_encoding.hpp
library_category: sequence
---

連続する同じ値をまとめ、`(値, 連続する個数)` の順に返します。文字列と数列で共通して使え、入力は変更しません。

```cpp
#include "lib/sequence/run_length_encoding.hpp"
#include <string>

auto text = run_length_encode(std::string("aaabbcaaa"));
// ('a',3), ('b',2), ('c',1), ('a',3)

std::vector<int> a{2, 2, 5, 5, 5, 2};
auto runs = run_length_encode(a);
// (2,2), (5,3), (2,1)
```

| 操作 | 内容・返り値 |
| --- | --- |
| `run_length_encode(a,equal = std::equal_to<>{})` | `std::vector<std::pair<T,std::int64_t>>`。`T` は入力の要素型 |

空列は空配列を返します。離れた位置にある同じ値はまとめません。個数は常に正で、合計は入力の長さです。個数は符号付きの `std::int64_t` で返し、各区間の長さは `INT64_MAX` 以下とします。

入力は読み取り専用で先頭から順に走査できる列とし、要素は結果へコピーできるものにします。`std::string`・`std::string_view`・`vector`・`array`・`span`・`deque`・`list`・C 配列・range view などを使え、`vector<bool>` にも対応します。文字列リテラルは `std::string_view("aaabb")` のように渡してください。C 配列を直接渡すと、末尾のヌル文字も含む全要素が対象です。

`equal(x,y)` は2要素を同じ値として扱うときに `true` を返す関数です。`equal(x,x)` は常に `true`、`equal(x,y)` と `equal(y,x)` は同じ結果とし、`x` と `y`、`y` と `z` を同じとするなら `x` と `z` も同じとする必要があります。各区間では最初の値を代表として返します。例えばキーの一部だけを比較する場合、キーが同じでも他のメンバーが異なる値は先頭の値で代表されます。

入力長を $$n$$、圧縮後の区間数を $$r$$ として、時間は $$O(n)$$、返り値を含む追加空間は $$O(r)$$ です。要素のコピー・移動・比較・イテレータの操作を $$O(1)$$ とします。
