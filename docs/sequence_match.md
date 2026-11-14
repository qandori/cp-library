---
title: "列の一致比較"
documentation_of: //components/sequence_match.hpp
library_category: components/string
---

```cpp
#include "lib/components/sequence_match.hpp"

std::vector<int> s{1, 2, 1, 2, 3};
sequence_match_detail::Matcher<int> match(s);
int prefix = match.forward(0, 2, 3);  // 2
int suffix = match.backward(2, 4, 2); // 2
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `sequence_match_detail::Matcher<T> match(s)` | 所有しない列の一致比較を準備 | $$O(1)$$ |
| `match.forward(i,j,limit)` | 2 位置からの前方一致長 | $$O(\mathrm{limit})$$ |
| `match.backward(i,j,limit)` | 2 位置の直前からの後方一致長 | $$O(\mathrm{limit})$$ |
| `match.index_runs(n)` | 長さ `n` の同一要素区間を前計算 | $$O(n)$$ |
| `match.forward_counted(i,j,limit)` / `match.backward_counted(i,j,limit)` | 前方／後方の `(一致長,比較量)`。同一要素区間を利用 | $$O(\mathrm{limit})$$ |
| `sequence_match_detail::z_table(n,z,match)` | `match(i,j,limit)` を用いて Z 配列を `z` に書き込む | $$O(n)$$ 回の要素比較 |
| `sequence_match_detail::prefix_matches(n,m,z,match,emit)` | 長さ `m` の接頭辞の Z 配列を再利用し、対象の各位置で `emit(i,一致長)` を呼ぶ | $$O(n+m)$$ 回の要素比較 |

列を所有しない内部部品です。元の列は比較中に有効である必要があります。`forward(i, j, limit)` は位置 `i, j` からの前方一致長、`backward(i, j, limit)` は位置 `i, j` の直前からの後方一致長です。比較する全要素が列の範囲内になる `limit >= 0` を指定します。

要素は `==` で比較します。`Matcher<T, false>` で AVX2 を無効にできます。

比較処理は [共通接頭辞・接尾辞の比較](https://qandori.github.io/cp-library/components/sequence_compare.hpp.html) と共有しています。異なる 2 配列の比較だけが必要な場合は、そちらの `prefix`・`suffix` を直接使えます。

`index_runs(n)` は同一要素区間の境界を前計算します。`forward_counted`・`backward_counted` は `(一致長, 比較量)` を返し、最初の同一要素区間をまとめて飛ばします。前計算後は元の列を変更しないでください。`z_table`・`prefix_matches` だけが必要な場合は [Z algorithm の補助部品](z_algorithm_core.md) を直接 include できます。

長さを $$n$$ として、通常の比較は最悪 $$O(\mathrm{limit})$$ 時間・$$O(1)$$ 追加空間、同一要素区間の前計算は $$O(n)$$ 時間・空間、Z algorithm は合計 $$O(n)$$ 要素比較です。`z_table`・`prefix_matches` は、渡した Z 配列と `match`・`emit` 自身が使う領域を除き、$$O(1)$$ 追加空間です。`emit` の処理時間は別途必要です。
