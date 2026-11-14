---
title: ワイルドカード付きパターンマッチング
documentation_of: //string/wildcard_pattern_matching.hpp
library_category: string/pattern_matching
---

本文 `text` の中で、パターン `pattern` が一致する位置を求めます。ワイルドカードは任意の **1 バイト** に一致します。長さの異なる文字列に一致する機能ではありません。本文・パターンの双方にワイルドカードを含められ、判定に誤差や乱数による誤判定はありません。

```cpp
#include <cassert>
#include <optional>
#include <vector>
#include "lib/string/wildcard_pattern_matching.hpp"

int main() {
	auto positions = wildcard_find_all("abacaba", "a?a", '?');
	assert((positions == std::vector<int>{0, 2, 4}));
	auto matches = wildcard_pattern_matching("a*ca", "ab");
	assert((matches == std::vector<bool>{true, false, false}));

	WildcardOptions options;
	options.text_wildcard = std::nullopt;
	options.pattern_wildcard = '?';
	assert((wildcard_find_all("a?aa", "a?", options) == std::vector<int>{0, 2}));
	assert((wildcard_find_all("ab", "") == std::vector<int>{0, 1, 2}));
}
```

引数の順序は本文、パターン、ワイルドカードの指定です。文字列は `std::string_view` で受け取り、変更しません。NUL を含む全 256 種類のバイトを扱えます。UTF-8 の文字もバイト単位で比較するため、ワイルドカード 1 個が日本語などの 1 文字全体に一致するとは限りません。

第 3 引数を省略すると、本文・パターンの `'*'` がワイルドカードになります。`char wildcard` を渡すと、そのバイトを双方のワイルドカードにします。`WildcardOptions` を渡す場合は次の設定を使います。

| メンバー | 型・既定値 | 内容 |
| --- | --- | --- |
| `text_wildcard` | `std::optional<unsigned char>`、`'*'` | 本文側のワイルドカード。`std::nullopt` なら本文の全バイトを通常の文字として扱う |
| `pattern_wildcard` | `std::optional<unsigned char>`、`'*'` | パターン側のワイルドカード。`std::nullopt` ならパターンの全バイトを通常の文字として扱う |

双方で異なるバイトを指定することもできます。ワイルドカードを指定した側では、そのバイトを通常の文字として区別しません。例えば本文側の指定を無効にすれば、本文中の `'*'` 自体を検索できます。

本文長を $$n$$、パターン長を $$m$$ とします。位置は 0 始まりです。`wildcard_pattern_matching` の返り値は `std::vector<bool>` で、要素 `i` は本文の区間 `[i,i+m)` とパターンが一致するかを表します。`wildcard_find_all` は一致した位置を昇順の `std::vector<int>` で返し、重なる一致も含みます。

パターンが本文以下の長さなら、判定する位置は `0`～`n-m` です。パターンが本文より長い場合は双方とも空配列です。空パターンは `0`～`n` の全てに一致します。本文・パターンとも空の場合も、位置 `0` の 1 件が一致します。

本文長は `INT_MAX` 以下、パターン長は $$2^{22}$$ 以下とします。超えた場合は `std::length_error` を送出します。この長さの検査は、空入力やパターンが本文より長い場合にも行います。

| 操作 | 返り値 | 時間計算量 | 使用空間 |
| --- | --- | --- | --- |
| `WildcardOptions()` | 双方のワイルドカードを `'*'` にした設定 | $$O(1)$$ | $$O(1)$$ |
| `wildcard_pattern_matching(text,pattern,wildcard='*')` | 全位置の一致判定 `std::vector<bool>` | $$O(n\log m)$$ | $$O(n+m)$$ |
| `wildcard_pattern_matching(text,pattern,options)` | 独立した設定による全位置の一致判定 `std::vector<bool>` | $$O(n\log m)$$ | $$O(n+m)$$ |
| `wildcard_find_all(text,pattern,wildcard='*')` | 一致位置の `std::vector<int>` | $$O(n\log m)$$ | $$O(n+m)$$ |
| `wildcard_find_all(text,pattern,options)` | 独立した設定による一致位置の `std::vector<int>` | $$O(n\log m)$$ | $$O(n+m)$$ |

計算量は $$1\le m\le n$$ の場合の最悪値で、$$m=1$$ の対数の項は定数として扱います。長さ 64 以下のパターンや、双方の入力にワイルドカードが現れない場合の時間は $$O(n+m)$$ です。空パターンでは時間・使用空間とも $$O(n)$$、パターンが本文より長い場合は時間・使用空間とも $$O(1)$$ です。使用空間には返り値を含みます。前計算や共有表を呼び出し後に保持しません。
