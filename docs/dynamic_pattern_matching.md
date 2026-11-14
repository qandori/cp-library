---
title: Dynamic Pattern Matching (固定本文)
documentation_of: //string/dynamic_pattern_matching.hpp
library_category: string/pattern_matching
---

固定した本文に対して、パターンを編集しながら出現回数と出現位置を求めます。文字の挿入・削除・置換、部分文字列の削除・移動・コピーに対応します。照合は厳密で、ハッシュの衝突はありません。

```cpp
#include "lib/string/dynamic_pattern_matching.hpp"

int main() {
	DynamicPatternMatching matching("banana", "ana");
	assert(matching.count() == 2);
	assert((matching.occurrences() == std::vector<int>{3, 1}));
	matching.erase(1);
	assert(matching.str() == "aa" && matching.count() == 0);
	matching.insert(1, 'n');
	assert(matching.count() == 2);
	auto saved = matching;
	matching.copy(0, 3, 3);
	assert(matching.str() == "anaana" && matching.count() == 0);
	assert(saved.str() == "ana" && saved.count() == 2);
	matching.erase(0, 3);
	assert(matching.count() == 2);
	matching.assign("ban");
	matching.move(0, 1, 2);
	assert(matching.str() == "anb");
	matching.clear();
	assert(matching.count() == 7);
	matching.compact();
}
```

本文とパターンは `std::string_view` で渡します。バイト単位で扱い、NUL を含む全 256 種類に対応します。入力した文字列はコピーするため、呼び出し後に元の文字列を変更・破棄できます。本文は `INT_MAX/2` バイト未満、パターン長と位置は `uint64_t` の範囲です。パターンを編集しても本文は変わりません。本文の変更には `build` での再構築が必要です。

添字は全て 0 始まり、区間は `[l,r)` です。`insert(p,...)` と `copy(l,r,p)` の挿入位置は、操作前のパターンの `p` 文字目の直前です。`p == size()` なら末尾に挿入します。`copy` は、操作前の `[l,r)` をコピーするので、`p` がコピー元の内部でも使えます。

`move(l,r,p)` の `p` は **元の区間を削除した後** の位置です。例えば `abcdef` に `move(1,3,4)` を行うと、`bc` を取り除いた `adef` の位置 `4` に挿入し、`adefbc` になります。`0 <= p <= size()-(r-l)` を満たしてください。空区間の削除・移動・コピーは何もしません。コピーや挿入の後の長さが `UINT64_MAX` を超えないようにしてください。

`count()` は重なりを含めた出現回数です。空のパターンは本文の位置 `0`～`text().size()` に現れるため、出現回数は本文長に 1 を足した値です。本文にない文字を含む場合や、パターンが本文より長い場合の出現回数は 0 です。

`occurrences()` は出現位置を **接尾辞の辞書順** に返します。位置の昇順が必要な場合は返り値をソートしてください。`occurrence(i)` はその `i` 番目を配列を作らず取得します。`occurrence_range()` の `[l,r)` は本文上の区間ではなく、`suffix_array()` 上の区間です。出現位置は `suffix_array()[l]`～`suffix_array()[r-1]` です。この接尾辞配列は空の接尾辞も含み、長さは本文長に 1 を足した値、先頭は本文長です。

コピー構築・コピー代入では本文と木を共有します。その後の編集はコピー元に影響しないため、`auto saved = matching;` として状態を保存できます。ムーブ元は本文・パターンとも空になります。文字列を展開する `str()` はパターン長に比例する出力を作るため、巨大なコピーの後は `count()`・`operator[]` などを使ってください。

部分文字列のコピーは木を共有して処理します。編集で不要になった木の節点はその場で破棄せず、記憶領域に残します。`clear()`・`assign()` でも節点の記憶領域は残ります。長期間編集を続ける場合は、適宜 `compact()` で現在の状態から到達できない節点を取り除いてください。保存した別のコピーがある場合、そのコピーの記憶領域は解放されません。`storage_nodes()` は共有する記憶領域の節点数を返し、現在のパターンの断片数とは異なります。節点番号は内部で `int` を使うため、保持節点数は `INT_MAX` 未満にしてください。

参考：[String Matching with a Dynamic Pattern](https://arxiv.org/html/2506.11318v1)。このライブラリは固定した本文を扱い、論文第7節の本文への文字追加には対応しません。

本文長を $$n$$、操作前後のパターン長の最大を $$m$$、渡す文字列の長さを $$s$$、出現回数を $$k$$、共有して保持する節点数を $$A$$ とします。対数の項は長さが 0・1 の場合に定数として扱います。† は節点を格納する配列の伸長も含め、一連の操作の合計から評価した1回当たりの計算量（償却計算量）です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicPatternMatching()` | 本文・パターンとも空の状態を作る | $$O(1)$$ |
| `DynamicPatternMatching(text,pattern={})` | 本文の索引と初期パターンを作る | $$O(n+s)$$ |
| `build(text,pattern={})` | 本文とパターンを作り直す | 新規構築 $$O(n+s)$$ と旧記憶領域の解放 |
| `assign(pattern)` | パターンを渡した文字列に置き換える | † $$O(s)$$ |
| `clear()` | パターンを空にする | $$O(1)$$ |
| `size()` / `empty()` | パターン長 / パターンが空か | $$O(1)$$ |
| `text()` | 固定した本文の `string_view` | $$O(1)$$ |
| `insert(p,c)` | 位置 `p` に 1 バイト挿入 | † $$O(\log n+\log m)$$ |
| `insert(p,pattern)` | 位置 `p` に文字列を挿入 | † $$O(s+\log n+\log m)$$ |
| `erase(p)` / `erase(l,r)` | 1 バイト / 区間を削除 | † $$O(\log n+\log m)$$ |
| `set(p,c)` | 位置 `p` を 1 バイトに置換 | † $$O(\log n+\log m)$$ |
| `move(l,r,p)` | 区間を削除後の位置 `p` に移動 | † $$O(\log n+\log m)$$ |
| `copy(l,r,p)` | 区間のコピーを操作前の位置 `p` に挿入 | † $$O(\log n+\log m)$$ |
| `count()` | 出現回数を `int` で返す | $$O(1)$$ |
| `occurrence(i)` | 接尾辞配列の順で `i` 番目の出現位置 | $$O(1)$$ |
| `occurrences()` | 全出現位置の `vector<int>` | $$O(k)$$ |
| `occurrence_range()` | 出現する接尾辞配列の区間 `[l,r)` | $$O(1)$$ |
| `suffix_array()` | 空の接尾辞を含む接尾辞配列への const 参照 | $$O(1)$$ |
| `operator[](p)` | パターンの位置 `p` のバイト | $$O(\log m)$$ |
| `str()` | 現在のパターン全体を `string` に展開 | $$O(m)$$ |
| `storage_nodes()` | 共有する節点記憶領域の保持節点数 | $$O(1)$$ |
| `compact()` | 本文の索引は共有し、現在必要な節点だけを新しい記憶領域に移す | $$O(A)$$ 時間・追加空間 |
| コピー構築 / コピー代入 | 本文・木を共有して状態を保存 | 構築 $$O(1)$$、代入は旧記憶領域の解放が加わる |
| ムーブ構築 / ムーブ代入 | 状態を移し、移動元を空に戻す | 構築 $$O(1)$$、代入は旧記憶領域の解放が加わる |
| `swap(other)` | 状態を交換する | $$O(1)$$ |

本文の索引は $$O(n)$$ 空間です。初期パターンには高々 $$O(s)$$ 節点を使います。文字の編集・部分文字列の操作 1 回につき追加する節点数は $$O(\log m)$$、長さ $$s$$ の文字列の代入・挿入ではさらに $$O(s)$$ です。最後に共有状態が破棄される際は、その記憶領域の解放が発生します。パターン長を本文長以下に保つ場合、編集の計算量は論文と同じ † $$O(\log n)$$ です。
