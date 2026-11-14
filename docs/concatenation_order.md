---
title: 文字列結合の辞書順比較・並べ替え
documentation_of: //string/concatenation_order.hpp
library_category: string
---

文字列を並べ替えて結合したときに、辞書順最小・最大となる順序や文字列を求めます。

```cpp
#include "lib/string/concatenation_order.hpp"

std::vector<std::string> s{"b", "ba", "baa"};
int c = concatenation_compare("b", "ba"); // 1
auto p = concatenation_order(s);          // {2,1,0}
auto q = concatenation_order(s, true);    // {0,1,2}
auto small = min_concatenation(s);        // "baabab"
auto large = max_concatenation(s);        // "bbabaa"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `concatenation_compare(a,b)` | `a+b` と `b+a` の辞書順比較。小さい・等しい・大きいとき、それぞれ -1・0・1 | $$O(\lvert a\rvert+\lvert b\rvert)$$ |
| `concatenation_order(s,descending = false)` | 結合結果が辞書順最小となる元の添字の順列。`true` なら最大 | $$O(N+L\log N)$$ |
| `min_concatenation(s)` | 辞書順最小の結合結果 | $$O(N+L\log N)$$ |
| `max_concatenation(s)` | 辞書順最大の結合結果 | $$O(N+L\log N)$$ |

`s` には `vector<string>`・`vector<string_view>`・配列・`span` などを渡せます。入力は変更しません。空文字列は順列の先頭に元の順序でまとめます。空でない文字列のうち `a+b == b+a` となるものも、元の順序を保ちます。空の入力に対しては空の順列・文字列を返します。

`concatenation_compare` は `string_view` を受け取ります。一方が空なら 0 を返すため、空文字列を含む列のソートには `concatenation_order` を使ってください。

NUL を含む全 256 種類のバイトに対応し、バイトを符号なしとして比較します。文字列数と各文字列の長さはそれぞれ `INT_MAX` 以下としてください。

文字列数を $$N$$、長さの合計を $$L$$ とすると、並べ替え・結合は時間 $$O(N+L\log N)$$、追加空間 $$O(N+L)$$ です。$$N\le1$$ の結合は時間 $$O(N+L)$$ です。2 文字列の比較だけなら追加空間は $$O(1)$$ です。

参考：[AtCoder Algorithm Lectures：文字列の並べ替えと辞書順最小化](https://info.atcoder.jp/entry/algorithm_lectures/lex_min_string_concatenation)
