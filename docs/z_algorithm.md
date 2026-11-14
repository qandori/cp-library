---
title: Z algorithm
documentation_of: //string/z_algorithm.hpp
library_category: string/pattern_matching
---

```cpp
#include "lib/string/z_algorithm.hpp"

auto z = z_algorithm("ababa"); // {5,0,3,0,1}
auto a = z_algorithm(std::vector<int>{1, 2, 1, 2}); // {4,0,2,0}
```

`z_algorithm(sequence)` は、各位置からの接尾辞と列全体との最長共通接頭辞の長さを `vector<int>` で返します。非空列では `z[0]=sequence.size()`、空列では空配列です。

`string_view`、`vector<T>`、`span<const T>` を渡せます。`T` は等値比較が可能とし、`vector<bool>` は対象外です。列長は `INT_MAX` 以下とします。

列長を $$n$$ として、時間・追加空間は $$O(n)$$ です。
