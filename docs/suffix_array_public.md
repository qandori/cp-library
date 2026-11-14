---
title: Suffix Array
documentation_of: //string/suffix_array.hpp
library_category: string/substring
---

```cpp
#include "lib/string/suffix_array.hpp"
#include <cassert>

int main() {
	auto sa = suffix_array("banana");
	assert((sa == std::vector<int>{5, 3, 1, 0, 4, 2}));
}
```

| 関数 | 内容 |
| --- | --- |
| `suffix_array(s)` | `std::string_view`、または `std::vector<T>` の空でない接尾辞を辞書順に並べ、その開始位置を `std::vector<int>` で返す |
| `suffix_array(s, upper)` | `std::vector<int>` を、各要素が `0 <= x <= upper` の整数列として処理する。`upper >= 0` |

長さは `INT_MAX` 以下です。空列は空配列、長さ 1 なら `[0]` を返します。文字列は符号なしバイト順です。一般の `T` は `<` による大小比較が必要です。`x < x` は偽とし、`x < y` かつ `y < z` なら `x < z` も成り立つようにします。また、どちら向きの `<` も偽となる要素を同値としたとき、`x` と `y`、`y` と `z` が同値なら `x` と `z` も同値となる必要があります。文字列と `vector<char>` は、高位ビットのある文字で順序が異なる場合があります。後者は `char` の大小関係に従います。

長さを $$n$$ として、文字列版は $$O(n)$$ 時間・空間、整数の値域を渡す版は $$O(n+\text{upper})$$ 時間・空間、一般の列は $$O(n\log n)$$ 時間、$$O(n)$$ 空間です。`upper > INT_MAX - 2` の場合は座標圧縮する版を使います。空列は定数時間・空間です。
