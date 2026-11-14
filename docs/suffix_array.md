---
title: Suffix Array Core
documentation_of: //components/suffix_array.hpp
library_category: components/string
---

[Suffix Tree](suffix_tree.md) で使う接尾辞配列の構築です。接尾辞配列は、各位置から末尾までの列を辞書順に並べ、その開始位置を格納した配列です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `suffix_array_detail::bytes(s)` | `string_view` の空でない接尾辞の開始位置を、符号なしバイトの辞書順で返す | $$O(n+\sigma)$$、バイトでは $$O(n)$$ |
| `suffix_array_detail::induced_sort(s,upper)` | `size()` と整数添字を持つ列を、値域 `0`～`upper` として構築する内部処理 | $$O(n+\text{upper})$$ |

`bytes` は空文字列なら空配列を返し、NUL と 128 以上を含む全バイトに対応します。`induced_sort` の要素は非負整数で、`upper` は要素の最大値以上、`INT_MAX-2` 以下にしてください。値域が大きい列は先に座標圧縮が必要です。

列長を $$n$$、文字の種類数を $$\sigma$$ とします。構築時間・作業空間は $$O(n+\text{upper})$$、バイト列では $$O(n)$$ です。
