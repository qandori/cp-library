---
title: "Monotone Minima"
documentation_of: //components/monotone_minima.hpp
library_category: components/data_structure
---

```cpp
#include "lib/components/monotone_minima.hpp"

int n = 100, m = 100;
auto value = [](int row, int col) { return 1LL * (row - col) * (row - col); };
auto best = monotone_minima(n, m, [&](int row, int old_col, int new_col) {
    return value(row, new_col) < value(row, old_col);
});
```

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `monotone_minima(rows,columns,better)` | 行数・列数を指定し、各行の最良の列番号を `vector<int>` で返す | $$O(n+m\log n)$$、1 行: $$O(m)$$ |
| `monotone_minima(rows,column_ids,better)` | `vector<int>` に並べた列番号を対象にし、各行の最良の列番号を返す | $$O(n+m\log n)$$、1 行: $$O(m)$$ |

`better(row,old_col,new_col)` は、その行で新しい列を選ぶときに `true` を返します。最小値なら `<`、最大値なら `>` で比較します。同値で `false` を返すと、同じ値を取る列のうち先の列を選びます。行列を保存せず、比較時に値を計算しても構いません。

行番号が増えたとき、選ばれる最良の列の位置が左へ戻らないことが条件です。行と列を一部だけ取り出した全ての行列でも同じ性質が成り立つ場合を、完全単調と呼びます。この関数は元の行列でだけ条件を満たせば使えます。完全単調行列には、比較回数が線形の [SMAWK](https://qandori.github.io/cp-library/components/smawk.hpp.html) も使えます。条件の検査は行いません。

`column_ids` 版では、配列に並べた順を列の順として扱い、返り値は配列内の位置ではなく列番号です。列を絞った後の行列でも、選ばれる位置の単調性が必要です。

行数・列数は非負の `int` の範囲です。行があれば少なくとも 1 列が必要です。0 行は空配列を返します。0 行の場合や、1 列の場合は比較関数を呼びません。行列全体を配列に保存する必要はありません。

行数を $$n$$、列数を $$m$$ とし、比較を $$O(1)$$ とします。$$n\ge2,m\ge1$$ では $$O(n+m\log n)$$ 時間、返り値を含め $$O(n)$$ 空間です。返り値以外の作業空間は $$O(1)$$ です。1 行は $$O(m)$$ 時間、0 行は $$O(1)$$ 時間・空間です。
