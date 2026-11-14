---
title: Dynamic Bitset
documentation_of: //data_structure/dynamic_bitset.hpp
library_category: data_structure
---

長さを実行時に指定するビット集合です。64 bit ごとの論理演算・シフト、区間の更新、立っているビットの列挙、集合間の個数計算を扱います。添字 `i` が整数としての $$2^i$$ のビットに対応します。

```cpp
#include "lib/data_structure/dynamic_bitset.hpp"
#include <cassert>
#include <vector>

int main() {
	DynamicBitset a("100101"), b("001111");
	assert(a.size() == 6 && a[0] && a[2] && a[5]);
	assert(a.count() == 3 && a.rank(3) == 2);
	assert(a.count_and(b) == 2);
	assert((a & b).to_string() == "000101");
	assert((a - b).to_string() == "100000");
	assert(a.find_first() == 0 && a.find_next(0) == 2);
	assert(a.find_prev(a.size()) == 5 && a.select(3) == a.size());

	std::vector<int> positions;
	a.for_each([&](int i) { positions.push_back(i); });
	assert((positions == std::vector<int>{0, 2, 5}));
	a.set_range(1, 4);
	assert(a.to_string() == "101111");
	a[5] = false;
	assert(a.to_string() == "001111");
}
```

`DynamicBitset` はテンプレートではありません。長さと添字は `int` で、長さは `0 <= n <= INT_MAX`、要素の添字は `0 <= i < size()` とします。区間はすべて半開区間 `[l,r)` で、`0 <= l <= r <= size()` が必要です。空区間の更新は何もせず、個数は 0、切り出した列は空になります。

文字列との変換では通常の二進表記と同様に、**右端が添字 0** です。例えば `"100101"` の `true` の位置は 0・2・5 です。文字列の先頭の 0 も列長に含みます。空文字列は長さ 0 に対応し、`'0'`・`'1'` 以外を含む入力には `std::invalid_argument` を投げます。

| 構築・サイズの操作 | 内容・返り値 |
| --- | --- |
| `DynamicBitset()` | 長さ 0 の列を作る |
| `DynamicBitset(int n, bool value = false)` | 長さ `n`、全要素が `value` の列を作る |
| `DynamicBitset(std::string_view text)` | 二進表記をコピーして列を作る |
| `DynamicBitset::from_words(int n, std::span<const std::uint64_t> words)` | 64 bit 単位の配列をコピーして列を返す |
| `size()` / `empty()` | 長さを `int` / 長さが 0 かを `bool` で返す |
| `resize(int n, bool value = false)` | 長さを変更。既存部分を保ち、追加部分を `value` にする。`void` |
| `reserve(int n)` | `n` bit 分の領域を予約。長さは変えない。`void` |
| `push_back(bool value)` / `pop_back()` | 添字 `size()` に追加 / 最大の添字のビットを削除。`void`。削除には非空が必要 |
| `clear()` / `swap(other)` | 長さを 0 にする（予約領域は保持）/ 2 つの列を交換。`void` |
| `words()` | 内部配列への `std::span<const std::uint64_t>` |
| `to_string()` | 長さを保った二進表記を `std::string` で返す |

`std::uint64_t` 1 個を、以下ではワードと呼びます。`from_words` の配列長は正確に $$\lceil n/64\rceil$$ とします。`words[0]` が添字 0～63、`words[1]` が 64～127 に対応します。最後のワードのうち長さの外にある上位ビットは読み捨てます。`words()` の配列長も同じで、外側のビットは常に 0 です。この参照を通した変更はできません。

| 一点・全体・区間の操作 | 内容・返り値 |
| --- | --- |
| `test(int i)` / const の `operator[](int i)` | ビット値を `bool` で返す |
| 非 const の `operator[](int i)` | 要素の読み書きを代行する `reference` 型のオブジェクト（プロキシ）。`bool` に変換できる。`bits[i] = value`、プロキシ同士の代入、`bits[i].flip()` が可能 |
| `set(int i, bool value = true)` / `reset(int i)` / `flip(int i)` | 一点を代入 / `false` にする / 反転 |
| `set_all(bool value = true)` / `reset_all()` / `flip_all()` | 全体を代入 / `false` にする / 反転 |
| `set_range(int l, int r, bool value = true)` / `reset_range(l,r)` / `flip_range(l,r)` | 区間を代入 / `false` にする / 反転 |
| `count()` / `count(int l, int r)` / `rank(int r)` | 全体 / `[l,r)` / `[0,r)` の `true` の個数を `int` で返す |
| `any()` / `none()` / `all()` | `true` が存在する / 存在しない / 全要素が `true` かを `bool` で返す |

`set`・`reset`・`flip` とその全体版・区間版は自身への `DynamicBitset&` を返します。空の列では `count() == 0`、`any() == false`、`none() == true`、`all() == true` です。`rank` は区間個数の別表記であり、事前計算した索引を保持する操作ではありません。

テンプレートの `all` マクロは引数の有無で分岐する定義を使ってください。`sort(all(v))` と `bits.all()` をそのまま併用できます。C++20 以降の定義例は[利用者テンプレートとの併用](template_compatibility.md)を参照してください。ヘッダーは利用者のマクロを変更しません。

| 集合間の演算 | 内容・返り値 |
| --- | --- |
| `a & b` / `a \| b` / `a ^ b` / `a - b` | 共通部分 / 和集合 / 対称差 / 差集合を `DynamicBitset` で返す |
| `a &= b` / `a \|= b` / `a ^= b` / `a -= b` | 同じ演算で `a` を更新し、`DynamicBitset&` を返す |
| `~a` | 長さの内側だけを反転した列を返す |
| `count_and(other)` / `count_or(other)` / `count_xor(other)` / `count_and_not(other)` | 各演算の結果の `true` の個数を、結果の列を作らず `int` で返す |
| `intersection_parity(other)` | 共通する `true` の個数が奇数かを `bool` で返す |
| `intersects(other)` | 共通する `true` が存在するかを `bool` で返す |
| `is_subset_of(other)` / `is_proper_subset_of(other)` | 自身が部分集合 / 真部分集合かを `bool` で返す |
| `compare(other)` | 符号なし整数とみなした大小を `int` の `-1`・`0`・`1` で返す |
| `operator==` / `operator!=` | 列長と全ビットが等しいか / 異なるかを `bool` で返す |

`a & b` の長さは `min(a.size(),b.size())`、`a | b`・`a ^ b` の長さは `max(a.size(),b.size())` です。添字 0 側をそろえ、短い側に存在しないビットを `false` として計算します。文字列表現では短い側を左側の 0 で埋めることに相当します。代入演算子の `&=`・`|=`・`^=` も同じ規則で自身の長さを変更します。`count_and`・`count_or`・`count_xor` は、この長さの規則による結果の個数を、列を作らずに返します。

```cpp
#include "lib/data_structure/dynamic_bitset.hpp"
#include <cassert>

int main() {
	DynamicBitset a("101"), b("11011");
	assert((a | b).to_string() == "11111");
	assert((a ^ b).to_string() == "11110");
	assert((a & b).to_string() == "001");
	assert(a.count_or(b) == 5 && a.count_xor(b) == 4 && a.count_and(b) == 1);
	a |= b;
	assert(a.size() == 5 && a.to_string() == "11111");
	a &= DynamicBitset("10");
	assert(a.size() == 2 && a.to_string() == "10");
	assert((b & DynamicBitset{}).empty());
}
```

差集合の `a - b`・`a -= b`、`count_and_not`、`intersection_parity`、`intersects`、部分集合判定、`compare` は列長が一致する必要があります。自身との演算も可能です。長さが一致するとき `a - b` と `count_and_not(b)` は `a & ~b` に対応します。`compare` は大きい添字のビットから比較するため、集合の包含関係とは別の順序です。等値比較は異なる長さも受け付け、末尾がすべて 0 でも長さが違えば等しくありません。

| 検索・列挙 | 内容・返り値 |
| --- | --- |
| `find_first(bool value = true)` / `find_last(bool value = true)` | `value` と等しい最初 / 最後のビットの添字を `int` で返す |
| `find_next(int pos, bool value = true)` | `pos` より大きい添字から最初の一致を返す。`-1 <= pos < size()` |
| `find_prev(int pos, bool value = true)` | `pos` より小さい添字から最後の一致を返す。`0 <= pos <= size()` |
| `select(int k, bool value = true)` | 添字の昇順で 0 始まり `k` 番目の一致を返す。`k >= 0` |
| `for_each(F&& callback)` / `for_each(int l, int r, F&& callback)` | 全体 / 区間内の `true` の添字を昇順に `callback(int)` へ渡す。`void` |

検索結果がなければ、前方向・後方向ともに `size()` を返します。`select` も該当個数が `k` 以下なら `size()` です。空の列ではすべての検索が 0 を返します。`find_next(-1)` は `find_first()`、`find_prev(size())` は `find_last()` に対応します。列挙中はコールバックから同じ列を変更しないでください。

| シフト・組み換え | 内容・返り値 |
| --- | --- |
| `a << shift` / `a >> shift` | 大きい / 小さい添字へシフトした列を返す。空く部分は `false` |
| `a <<= shift` / `a >>= shift` | シフトして自身を更新し、`DynamicBitset&` を返す |
| `or_shift_left(shift)` / `or_shift_right(shift)` | 更新前の列を使って `a \|= a << shift` / `a \|= a >> shift` を行い、`DynamicBitset&` を返す |
| `slice(int l, int r)` | `[l,r)` を添字 0 から始まる長さ `r-l` の列として返す |
| `append(const DynamicBitset& other)` | `other` を大きい添字側へ連結する。自身の連結も可能。`void` |
| `copy_range(int dst, const DynamicBitset& source, int l, int r)` | `source[l,r)` を自身の `[dst,dst+r-l)` へ上書き。長さを変えず、自己参照と重複にも対応。`void` |
| `reverse()` | 添字 `i` の値を `size()-1-i` へ移し、`DynamicBitset&` を返す |
| `rotate_left(shift)` / `rotate_right(shift)` | 大きい / 小さい添字へ巡回移動し、`DynamicBitset&` を返す |

`shift` の型は `std::size_t` です。通常のシフトは列長を変えず、`shift >= size()` なら全ビットが `false` になります。`or_shift_left`・`or_shift_right` は一時的な列を作らずに更新し、この場合は元の列を保ちます。巡回移動では非空の列に対して `shift % size()` を使い、空の列は何も変えません。

`append` 後の長さは `INT_MAX` 以下にしてください。`copy_range` の `[l,r)` は `source` の範囲、`0 <= dst` と `r-l <= size()-dst` は書き込み先の範囲です。重なった部分の読み込みには操作前の値を使います。

```cpp
#include "lib/data_structure/dynamic_bitset.hpp"
#include <cassert>

int main() {
	DynamicBitset bits("1001");
	bits.rotate_left(1);
	assert(bits.to_string() == "0011");
	bits.or_shift_left(2);
	assert(bits.to_string() == "1111");
	bits.append(bits);
	assert(bits.to_string() == "11111111");
	bits.resize(10);
	assert(bits.to_string() == "0011111111");
	auto low = bits.slice(0, 4);
	assert(low.to_string() == "1111");
	bits.copy_range(2, bits, 0, 6);
	assert(bits.count() == 8);
}
```

コピーした列は独立しています。ムーブ後の移動元は長さ 0 になります。`words()` の `span` と要素のプロキシは内部領域を参照するため、領域の再確保・`clear`・代入・破棄後に使わないでください。長さを縮めた場合は、範囲外になった添字のプロキシも使えません。メモリ確保を伴う操作では標準コンテナの例外が発生することがあります。

演算前の長さを $$n$$、ワード数を $$W=\lceil n/64\rceil$$ とします。相手の列がある場合は、その長さを $$m$$、ワード数を $$V=\lceil m/64\rceil$$ とします。要素は $$W$$ ワードに格納し、予約領域も含む保持空間は確保済みの容量に比例します。`reserve` や過去の長さによる余剰領域がなければ $$O(W)$$ です。以下はワードの論理演算・1 のビット数の計算・ビット位置の検索を定数時間とする計算量です。

| 操作 | 時間・追加空間 |
| --- | --- |
| 長さを指定した構築・全体更新・全体個数・差集合・比較・検索・`select` | 最悪 $$O(W)$$ 時間。新しい列を返す演算は $$O(W)$$ 追加空間、それ以外は $$O(1)$$ |
| `a & b` / `a &= b` / `count_and(b)` | $$O(\min(W,V))$$ 時間。`a & b` は $$O(\min(W,V))$$ 追加空間、それ以外は $$O(1)$$ |
| `a \| b` / `a ^ b` | $$O(\max(W,V))$$ 時間・追加空間 |
| `a \|= b` / `a ^= b` | 最悪 $$O(\max(W,V))$$ 時間・追加空間。再確保がなければ $$O(V)$$ 時間・$$O(1)$$ 追加空間 |
| `count_or(b)` / `count_xor(b)` | $$O(\max(W,V))$$ 時間・$$O(1)$$ 追加空間 |
| 文字列からの構築・`to_string` | $$O(n)$$ 時間・空間 |
| 一点の取得・更新、`size`・`empty`・`words`・`swap` | $$O(1)$$ 時間・追加空間 |
| 区間の更新・個数・`rank` | 対象区間に触れるワード数に比例する時間、$$O(1)$$ 追加空間 |
| `for_each` | 対象ワード数と列挙数の合計に比例する時間、$$O(1)$$ 追加空間 |
| シフト・`or_shift_left`・`or_shift_right`・順序反転・巡回移動 | $$O(W)$$ 時間。追加空間は通常の破壊的シフト・シフトとの OR・順序反転が $$O(1)$$、コピーを返すシフトと巡回移動が $$O(W)$$ |
| `copy_range(dst,source,l,r)` | 対象ワード数に比例する時間、自己参照でも $$O(1)$$ 追加空間 |
| `append(other)` | 元のワード数と追加するワード数の合計に対して最悪線形時間・追加空間。再確保がなければ追加ワード数に比例する時間、$$O(1)$$ 追加空間 |
| `resize` | 再確保が発生するときは元のワード数と変更後のワード数の合計に対して最悪線形時間・追加空間 |
| `reserve` | 再確保が発生するときは予約するワード数に対して最悪線形時間・追加空間 |
| `slice(l,r)` | $$O(\lceil(r-l)/64\rceil)$$ 時間・追加空間 |

`any`・`all`・検索・比較は結果が決まった時点で走査を終えます。両方が空の演算・空区間・シフト量 0 などの何もしない場合は定数時間です。再確保のない `resize` は、追加・初期化するワード数に比例する時間で、縮小時は定数時間です。再確保のない `reserve` は定数時間です。`push_back` は償却 $$O(1)$$、`pop_back` は $$O(1)$$ です。コピーは $$O(W)$$、ムーブ構築は $$O(1)$$ で、代入には旧領域の解放も加わります。`clear` は定数時間です。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
