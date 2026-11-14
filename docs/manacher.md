---
title: Manacher（回文半径）
documentation_of: //string/manacher.hpp
library_category: string/palindrome
---

各位置を中心とする最長の回文を線形時間で求めます。前計算後は、任意の区間が回文かを定数時間で判定できます。

```cpp
#include "lib/string/manacher.hpp"

Manacher palindrome("abacabba");
assert(palindrome.odd[1] == 2);  // "aba"
assert(palindrome.even[6] == 2); // "abba"
assert(palindrome.is_palindrome(4, 8));
auto [l, r] = palindrome.longest_palindrome(); // [1,6): "bacab"
auto count = palindrome.palindrome_count();   // 13 (int64_t)
auto lengths = Manacher("aba").lengths();    // {1,0,3,0,1}

std::vector<int> a = {10, -3, -3, 10};
Manacher sequence(a);
assert(sequence.is_palindrome(0, 4));
```

最大回文長の列だけが必要なら、`palindrome_lengths(s)` を使います。文字と文字の間を含む全ての中心について、最長の回文の長さを左から並べた `vector<int>` を返します。

```cpp
#include "lib/string/manacher.hpp"
#include <cassert>
#include <vector>

int main() {
	auto lengths = palindrome_lengths("aba");
	assert((lengths == std::vector<int>{1, 0, 3, 0, 1}));
}
```

返り値の `2*i` 番目は文字 `s[i]` を中心とする長さ、`2*i+1` 番目は `s[i]` と `s[i+1]` の間を中心とする長さです。文字間に回文がなければ 0 です。長さ $$n>0$$ の入力に対して $$2n-1$$ 要素、空入力に対して空配列を返します。入力の列・型・長さに関する条件は `Manacher` と同じです。入力は変更せず、計算量と使用空間は $$O(n)$$ です。

添字は 0 始まり、区間は半開区間 `[l,r)` です。`odd[i]` は中心 `i` を含めた片側の長さ、`even[i]` は `i-1` と `i` の間を中心とする片側の長さです。

| 半径 | 最長の回文の区間 | 長さ |
| --- | --- | --- |
| `odd[i] = k` | `[i-k+1,i+k)` | $$2k-1$$ |
| `even[i] = k` | `[i-k,i+k)` | $$2k$$ |

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Manacher(s)` / `build(s)` | 列 `s` の回文半径を構築／再構築 | $$O(n)$$ |
| `odd[i]` / `even[i]` | 奇数長／偶数長の回文半径 | $$O(1)$$ |
| `is_palindrome(l,r)` | `[l,r)` が回文か。空区間も `true` | $$O(1)$$ |
| `longest_palindrome()` | 最長の回文の `[l,r)` を `pair<int,int>` で返す。同長なら開始位置が最小のもの | $$O(n)$$ |
| `palindrome_count()` | 空でない回文の出現数を `int64_t` で返す。同じ文字列でも位置が異なれば別に数える | $$O(n)$$ |
| `lengths()` | 文字・文字間の中心を左から並べた最長回文長の `vector<int>` | $$O(n)$$ |
| `size()` / `empty()` | 入力の長さ／空か | $$O(1)$$ |

`std::string`・`std::string_view`・文字列リテラルのほか、`size()`・添字アクセス・要素の等値比較が使える `vector`・`array`・`span` などを渡せます。これらの操作は定数時間とします。入力は保持しないので、構築後に変更・破棄して構いません。公開配列 `odd`・`even` は直接書き換えないでください。

空列にも対応し、最長回文は `[0,0)`、出現数は `0`、`lengths()` は空配列になります。文字列はバイト単位で扱い、NUL を含む全てのバイトに対応します。埋め込み NUL は長さを指定した `string_view` または `string` で渡してください。入力長は `INT_MAX` 以下とします。

入力長を $$n$$ とすると、構築は $$O(n)$$ 時間・空間、区間の回文判定は $$O(1)$$ 時間です。
