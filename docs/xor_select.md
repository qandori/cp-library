---
title: 64 bit 内の XOR 順位選択
documentation_of: //components/xor_select.hpp
library_category: components/data_structure
---

[Predecessor Set](predecessor_set.md)・[XOR の整数集合](xor_set.md)が共有する内部処理です。

`xor_select_detail::argmin(word,key)` は、1 bit の立つ位置のうち `key` との XOR が最小になる位置を返します。`word != 0`、`key < 64` とします。時間・追加空間は $$O(1)$$ です。
