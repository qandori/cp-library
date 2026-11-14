---
title: Basic Substring Structure
documentation_of: //string/basic_substring_structure.hpp
library_category: string/substring
---

固定した文字列の異なる非空部分文字列を、階段状のブロックのセルへ一意に配置します。同じブロックの部分文字列は出現回数が等しく、先頭・末尾の 1 バイト削除をセル間の移動で扱えます。全セルを展開せず、ブロックと行・列の情報を線形空間で保持します。

```cpp
#include "lib/string/basic_substring_structure.hpp"
#include <cassert>

int main() {
	BasicSubstringStructure structure("banana");
	auto first = structure.locate(1, 4);
	auto second = structure.locate(3, 6);
	assert(first && second && *first == *second);
	assert(structure.occurrence_count(*first) == 2);
	assert(structure.distinct_substrings() == 15);

	auto [l, r] = structure.substring_range(*first);
	assert(structure.text().substr(l, r - l) == "ana");
	assert(structure.erase_first(*first) == structure.locate(2, 4));
	assert(structure.erase_last(*first) == structure.locate(1, 3));
	assert(!structure.erase_first(*structure.locate(1, 2)));
	assert(!structure.locate(2, 2));
}
```

入力は `std::string_view` で渡し、内部にコピーします。構築後に入力を変更・破棄しても使えます。NUL を含む全 256 バイトに対応し、位置と長さはバイト単位です。本文長を $$n$$ として `n < INT_MAX/2` を満たす必要があります。

`locate(l,r)` は本文の半開区間 `[l,r)` を表すセルを返します。異なる位置に同じ部分文字列が現れる場合も、返すセルは同じです。空文字列はセルに含めず、`l == r` なら `std::nullopt` です。本文外の文字列を検索する API はありません。

`Position` は `int row` と `int column` を持ち、等値比較ができます。行と列は全ブロックを通した番号です。`shape()` が返す `(H,W)` に対して行番号は `[0,H)`、列番号は `[0,W)` ですが、この長方形の全セルが有効とは限りません。`valid(position)` で判定できます。

`Block` は次の `int` フィールドを持ちます。

| フィールド | 意味 |
| --- | --- |
| `left`, `right` | このブロックの代表文字列を取り出す本文上の半開区間 `[left,right)` |
| `row_begin`, `row_end` | ブロックの行番号の半開区間 |
| `column_begin`, `column_end` | ブロックの列番号の半開区間 |
| `occurrences` | ブロック内の各部分文字列の出現回数。重なる出現も数える |

ブロック `b` 内のセル `(row_begin + dx, column_begin + dy)` は、本文の `[left + dy, right - dx)` に対応します。右へ進むと先頭を削除し、下へ進むと末尾を削除します。各行の有効なセルは左端から `width(row)` 個、各列では上端から `height(column)` 個です。行の幅と列の高さは、ブロック内で番号が増えるにつれて非増加になります。

例えば本文が `abab` のとき、代表文字列 `ab`、出現回数 2 のブロックは次の形です。右下は無効なセルです。

| 局所行・列 | `dy = 0` | `dy = 1` |
| --- | --- | --- |
| `dx = 0` | `ab` | `b` |
| `dx = 1` | `a` | — |

このブロックでは幅も高さも `[2,1]` です。残る `abab`・`bab`・`aba`・`ba` は出現回数 1 のブロックに入ります。同じ出現回数でも、必ず同じブロックになるわけではありません。

ブロック番号は `[0,block_count())` です。出現回数の降順に並び、同じ回数のブロック同士の順序は未規定です。非空の本文では最後のブロックの代表が本文全体となり、出現回数 1 の部分文字列をすべて含みます。代表区間は出現位置の一つであり、非空区間の `substring_range(*locate(l,r))` が元の `(l,r)` と一致するとは限りません。

`right(row)` は、その行の左端の文字列から `width(row)` バイトを先頭から削除した後の行番号です。返り値が `t >= 0` なら、移動先のセルは `{t, block(row_block(t)).column_begin}` です。`down(column)` は、その列の上端の文字列から `height(column)` バイトを末尾から削除した後の列番号で、移動先は `{block(column_block(t)).row_begin, t}` です。削除後が空なら、どちらも `-1` を返します。

これらは行・列の端から別のブロックへ移るためのリンクです。移動先では出現回数が増え、ブロック番号が小さくなります。任意のセルから 1 バイトだけ削除する場合は `erase_first`・`erase_last` を使います。ブロック内なら隣接セルへ移り、端ならリンク先へ移ります。

| 操作 | 内容・返り値 |
| --- | --- |
| `BasicSubstringStructure()` | 空の本文で構築 |
| `BasicSubstringStructure(text)` | `std::string_view` の本文で構築 |
| `build(text)` / `clear()` | 本文を置き換えて再構築 / 空にする。返り値は `void` |
| `size()` / `empty()` / `text()` | 本文長を `int` / 空判定を `bool` / 保存した本文を `std::string_view` で返す |
| `block_count()` / `shape()` | ブロック数を `int` / 総行数・総列数を `std::pair<int,int>` で返す |
| `blocks()` / `block(b)` | 全ブロックを `std::span<const Block>` / 番号 `b` のブロックを `const Block&` で返す |
| `row_block(row)` / `column_block(column)` | 行 / 列が属するブロックの番号を `int` で返す |
| `width(row)` / `height(column)` | 行の幅 / 列の高さを `int` で返す |
| `right(row)` / `down(column)` | 別ブロックへの行 / 列のリンクを `int` で返す。空文字列へのリンクは `-1` |
| `valid(position)` | 有効なセルかを `bool` で返す。任意の `int` 座標を渡せる |
| `locate(l,r)` | 本文の `[l,r)` に対応する `std::optional<Position>`。空区間なら `std::nullopt` |
| `substring_range(position)` | セルの文字列を取り出せる本文の半開区間を `std::pair<int,int>` で返す |
| `occurrence_count(position)` | セルの文字列の出現回数を `int` で返す |
| `erase_first(position)` / `erase_last(position)` | 先頭 / 末尾を 1 バイト削除したセルを `std::optional<Position>` で返す。元が 1 バイトなら `std::nullopt` |
| `distinct_substrings()` | 異なる非空部分文字列の数を `std::uint64_t` で返す |
| `swap(other)` | 2 つの構造の内容を交換。返り値は `void` |

添字引数は `int` です。`locate` には `0 <= l <= r <= size()`、ブロック・行・列の取得には各範囲内の番号を渡します。`substring_range`・`occurrence_count`・`erase_first`・`erase_last` には有効な `Position` が必要です。`valid` 以外では無効な番号を扱いません。検索・取得は `const` なオブジェクトから呼べます。

空の本文ではブロック・行・列・異なる非空部分文字列の数がすべて 0 です。`blocks()` は空、`shape()` は `(0,0)`、`locate(0,0)` は `std::nullopt` になります。

`build`・`clear`・代入後は、それ以前のブロック番号・セル・参照・`span` を使わないでください。`build(structure.text())` のように保存した本文を渡して再構築することもできます。コピーしたオブジェクトは独立して使えます。ムーブ後の移動元は空になります。

行の幅を合計すると、異なる部分文字列の数になります。さらに各行が属するブロックの出現回数を掛ければ、本文中の非空区間の総数を得られます。次の例は全セルを展開せず、行の情報だけを走査します。

```cpp
#include "lib/string/basic_substring_structure.hpp"
#include <cassert>
#include <cstdint>

int main() {
	BasicSubstringStructure structure("abab");
	std::uint64_t distinct = 0, intervals = 0;
	for (const auto &b : structure.blocks()) {
		for (int row = b.row_begin; row < b.row_end; ++row) {
			auto cells = std::uint64_t(structure.width(row));
			distinct += cells;
			intervals += cells * b.occurrences;
		}
	}
	assert(distinct == 7 && distinct == structure.distinct_substrings());
	assert(intervals == 10);
}
```

参考: [Basic Substring Structure に関する論文（arXiv:2312.11873）](https://arxiv.org/abs/2312.11873)。部分文字列の同一性判定にハッシュや乱数は使いません。

構築・`build` は $$O(n\log n)$$ 時間、保持空間と構築時の追加空間は $$O(n)$$ です。`locate` は $$O(\log n)$$ 時間、それ以外の取得・判定・削除操作は $$O(1)$$ 時間です。空入力と 1 バイトの入力では対数の項を定数として扱います。再構築・`clear`・代入では旧領域の解放も加わります。コピーは保持量に比例し、ムーブ構築は $$O(1)$$ 時間です。

非空の本文では総行数・総列数はそれぞれ高々 $$2n-1$$ で、ブロック数も $$O(n)$$ です。上の行走査は $$O(n)$$ 時間ですが、全有効セルを列挙すると異なる部分文字列数に比例し、最悪 $$O(n^2)$$ 時間かかります。
