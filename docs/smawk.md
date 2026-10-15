---
title: "SMAWK"
documentation_of: //math/smawk.hpp
---

```cpp
#include "lib/math/smawk.hpp"

auto best = smawk(rows, columns, [&](int row, int old_col, int new_col) {
    return value(row, new_col) < value(row, old_col);
});
```

完全単調行列の各行の最小値を取る列番号を返します。比較関数は新しい列がより良いときに `true` を返し、同値なら先の列を残します。

`columns` は列数、または調べる列番号を順に並べた `std::vector<int>` です。後者ではその順序で完全単調とします。行がある場合は少なくとも 1 列必要です。

行数を $$n$$、列数を $$m$$ として時間・空間は $$O(n+m)$$ です。比較は $$O(1)$$ とします。
