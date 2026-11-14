---
title: "Run 列挙"
documentation_of: //string/run_enumerate.hpp
library_category: string/substring
---

周期的な並びが2周期分以上続き、その周期のまま左右へ延ばせない区間（run）を列挙します。周期 `p` とは、区間内で `p` だけ離れた要素が全て等しくなる正の整数です。

```cpp
#include "lib/string/run_enumerate.hpp"

auto runs = run_enumerate("ababa");
for (auto [period, l, r] : runs) {
	// {2, 0, 5}
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `run_enumerate(s)` | run の一覧を `vector<Run>` で返す。文字列・vector・span に対応 | $$O(n\log n)$$ |
| `run.period` / `run.l` / `run.r` | 最小周期／半開区間の左端／右端 | $$O(1)$$ |
| `a == b` / `a != b` / `a < b` / `a <= b` / `a > b` / `a >= b` | `Run` を `(period,l,r)` の辞書順で比較 | $$O(1)$$ |

`run_enumerate(s)` は `std::vector<Run>` を返します。各要素は最小周期 `period` と半開区間 `[l, r)` です。区間長は周期の 2 倍以上で、左右どちらにも同じ周期のまま延長できません。結果は重複がなく、`(period, l, r)` の辞書順です。空列・長さ 1 の列では空になります。

`std::string_view`・`std::vector<T>`（`bool` を除く）・`std::span<const T>` を渡せます。文字列はバイト単位で扱い、NUL を含めて全バイトを使用できます。一般の要素には、一致判定に加えて矛盾のない大小比較が必要です。等しくない2要素は必ず一方が小さくなり、`x < y` かつ `y < z` なら `x < z` も成り立つようにします。長さは `INT_MAX` 未満としてください。

ハッシュを使わず、結果は決定的です。

長さを $$n$$ として、時間は最悪 $$O(n\log n)$$、空間は出力を含めて $$O(n)$$ です。
