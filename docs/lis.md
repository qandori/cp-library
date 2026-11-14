---
title: "Longest Increasing Subsequence"
documentation_of: //sequence/lis.hpp
redirect_from:
- /string/lis.hpp.html
library_category: sequence
---

列から要素を順序通りに選び、値が増加する部分列の最大長と、そのような部分列の 1 つを求めます。元の列で連続している必要はありません。数値以外の要素や独自の比較にも対応します。

```cpp
#include "lib/sequence/lis.hpp"

std::vector<int> a{3, 1, 2, 2, 4};
int length = lis_length(a);       // 3
auto indices = lis(a);            // 1, 3, 4
auto lengths = lis_lengths(a);    // 1, 1, 2, 2, 3

std::vector<int> values;
for (int i : indices) values.push_back(a[i]); // 1, 2, 4

int weak_length = lis_length<false>(a); // 4
auto weak_indices = lis<false>(a);     // 1, 2, 3, 4
```

既定は狭義増加 $$a_{i_0}<a_{i_1}<\cdots$$ です。`<false>` を指定すると、等しい値を続けて選べる広義増加 $$a_{i_0}\le a_{i_1}\le\cdots$$ になります。上の例で、狭義増加では 2 を 1 個、広義増加では 2 個選べます。

`lis` が返すのは値そのものではなく、元の列の **0 始まりの添字列** です。添字は昇順で、`a[indices[i]]` として値を取り出します。答えが複数ある場合は、そのうち 1 つを返します。

`lis_lengths(a)[i]` は、**位置 `i` を必ず最後に選ぶ部分列** の最大長です。例えば `[1,2,3,0]` に対しては `[1,2,3,1]` を返します。位置 `i` までの列全体での最大長が必要なら、この配列の先頭から最大値を累積してください。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `lis_length(a,compare = std::less<>{})` | 狭義増加部分列の最大長を `int` で返す | $$O(n\log n)$$ |
| `lis(a,compare = std::less<>{})` | 狭義増加部分列の添字を `vector<int>` で返す | $$O(n\log n)$$ |
| `lis_lengths(a,compare = std::less<>{})` | 各位置を終点とする狭義増加部分列の長さを `vector<int>` で返す | $$O(n\log n)$$ |
| `lis_length<false>(a,compare)` / `lis<false>(a,compare)` / `lis_lengths<false>(a,compare)` | 広義増加で同じ計算を行う。比較関数は省略可 | $$O(n\log n)$$ |

`a` は const でサイズ取得と添字による要素の取得ができる列を渡します。`vector`・`array`・C 配列・`span`・`deque`・`string_view` などに対応します。長さは `INT_MAX` 以下としてください。空の列では長さ 0、添字列と各位置の長さは空配列になります。元の列は変更しません。

比較関数は `compare(x,y)` が「指定した順序で `x` は `y` より前に来る」を返すものにします。`compare(x,x)` は `false` とし、`compare(x,y)` と `compare(y,z)` が真なら `compare(x,z)` も真にしてください。また、どちら向きの比較も `false` となる2要素を同値としたとき、`x` と `y`、`y` と `z` が同値なら `x` と `z` も同値となる必要があります。狭義の場合は `compare(x,y)`、広義の場合は `!compare(y,x)` を満たす順に要素を選びます。例えば `std::greater<>{}` を渡すと、減少部分列を求められます。

```cpp
#include "lib/sequence/lis.hpp"

std::vector<int> a{5, 3, 4, 2, 1};
auto decreasing = lis(a, std::greater<>{}); // 値は 5, 4, 2, 1

struct Item { int key; int id; };
std::vector<Item> items{Item{3, 0}, Item{1, 1}, Item{2, 2}};
auto indices = lis(items, [](const Item &x, const Item &y) {
	return x.key < y.key;
});
```

要素がコピーできなくても、比較できれば使えます。整数型の上限・下限もそのまま扱えます。

入力長を $$n$$、比較・要素の取得を $$O(1)$$ として、時間は全て $$O(n\log n)$$ です。答えの長さを $$L$$ とすると、`lis_length` の追加空間は $$O(L)$$、`lis`・`lis_lengths` の追加空間は $$O(n)$$ です。空入力はいずれも $$O(1)$$ 時間・空間です。
