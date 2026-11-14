---
title: Persistent Rope Core
documentation_of: //components/persistent_rope.hpp
library_category: components/string
---

文字列などの断片を並べた列を管理し、以前の列を残したまま分割・連結・更新できる内部部品です。このように過去の状態を残す性質を「永続」と呼びます。[永続 deque](persistent_deque.md) の更新や、[動的パターン照合](dynamic_pattern_matching.md) の部分文字列の分割・連結・コピーに使います。

```cpp
#include "lib/components/persistent_rope.hpp"
#include <string>

std::uint64_t length(const std::string &s) { return s.size(); }

int main() {
	PersistentRope<std::string, length> rope;
	int original = rope.build(std::vector<std::string>{"ab", "cde"});
	auto [left, right] = rope.split(original, 3, [](const std::string &s, std::uint64_t p) {
		return std::pair{s.substr(0, p), s.substr(p)};
	});
	int changed = rope.join(right, left);
	std::string text;
	rope.for_each(changed, [&](const std::string &s) { text += s; });
	assert(text == "deabc" && rope.size(original) == 5);
	auto [chunk, offset] = rope.at(original, 2);
	assert(chunk == "cde" && offset == 0);
	PersistentRope<std::string, length> compact;
	int copied = compact.copy_from(rope, changed);
	assert(compact.size(copied) == 5);
}
```

木の根は `int` の節点番号で持ちます。`0` は空の列です。木自身は現在の根を保持しないため、利用側で根番号を保存します。`make`・`build`・`join`・`split` などは新しい根を返し、以前の根の内容を変更しません。根番号を再利用して同じ列を何度も連結できます。

`T` はデフォルト構築・コピー・代入できる型にし、`weight(value)` は断片の正の整数の重みを返す関数にします。重みと列全体の重みは `uint64_t` に収めてください。位置は断片の個数ではなく、先頭からの重みの合計で指定します。

`split(root,p,splitter)` は重みの合計が `p` となる位置で分割します。`0 <= p <= size(root)` が必要です。断片の途中で分割するときだけ `splitter(value,offset)` を呼びます。返す `pair<T,T>` の重みは、それぞれ `offset` と `weight(value)-offset` にしてください。断片の境界では呼びません。`at(root,p)` は、その位置を含む断片と、断片の先頭からその位置までの重みの合計を返します。

`insert` は `0 <= p <= size(root)`、`erase` は `0 <= l <= r <= size(root)` とします。どちらも断片の途中を操作するときだけ、`split` と同じ条件の `splitter` を呼びます。`replace` は `p < size(root)` が必要で、位置を含む断片全体を置き換えます。新しい値の重みは、その断片の重みと同じにしてください。

根番号は、その節点を保持する同じインスタンスで使います。別のインスタンスへ移す場合は `copy_from` を使います。不要になった節点は自動で回収せず、`build` でも既存の領域は残ります。新しいインスタンスへ `copy_from` すると、指定した根から到達する節点だけを、共有関係を保って移せます。保持する節点数は `INT_MAX` 未満にしてください。

$$L$$ を根からたどった列の断片数、$$A$$ を保持する節点数、$$C$$ を分割時に利用者が渡した関数を呼ぶ時間とします。表は `T` のコピー・構築、`weight`、`for_each` に渡す関数が定数時間の場合です。文字列などを `T` にする場合は、そのコピー時間も加わります。† は節点配列の伸長を含む償却計算量で、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PersistentRope<T,weight>()` | 空の節点領域を作る。空列の根は `0` | $$O(1)$$ |
| `make(value)` | 断片 1 つから葉を作り、根番号を返す | † $$O(1)$$ |
| `build(values)` | `vector<T>` の断片列から木を作り、根番号を返す | † $$O(L)$$ |
| `join(a,b)` | 2 つの根の列をこの順に連結した根番号 | † $$O(\log L)$$ |
| `split(root,p,splitter)` | 分割後の左・右の根番号の組 | † $$O(\log L+C)$$ |
| `replace(root,p,value)` | 位置 `p` を含む断片を、同じ重みの `value` に置き換えた根番号 | † $$O(\log L)$$ |
| `insert(root,p,value,splitter)` | 位置 `p` に断片を挿入した根番号 | † $$O(\log L+C)$$ |
| `erase(root,l,r,splitter)` | 重みの区間 `[l,r)` を削除した根番号 | † $$O(\log L+C)$$ |
| `size(root)` | 列の重みの合計。根 `0` なら 0 | $$O(1)$$ |
| `storage_nodes()` | このインスタンスが保持する節点数 | $$O(1)$$ |
| `leaf(node)` | 葉か。`leaf(0)` は false | $$O(1)$$ |
| `value(node)` | 葉が持つ断片をコピーして返す | $$O(1)$$ |
| `front(root)` / `back(root)` | 先頭 / 末尾の断片をコピーして返す | $$O(\log L)$$ |
| `pop_front(root)` | 先頭の断片と、残りの列の根番号の組 | † $$O(\log L)$$ |
| `pop_back(root)` | 残りの列の根番号と、末尾の断片の組 | † $$O(\log L)$$ |
| `at(root,p)` | 位置を含む断片と、断片の先頭からその位置までの重みの合計の組 | $$O(\log L)$$ |
| `for_each(root,visitor)` | 左から各断片に `visitor(value)` を呼ぶ | $$O(L)$$ |
| `copy_from(other,root)` | 別の領域から共有を保ってコピーし、コピー先の根を返す | † $$O(A)$$ 時間・追加空間。ここではコピー元の保持数 |
| コピー構築・コピー代入 | 節点領域全体をコピー。同じ根番号をコピー先でも使える | $$O(A)$$。代入は旧領域の解放も加わる |
| ムーブ構築・ムーブ代入 | 節点領域を移動 | 構築 $$O(1)$$。代入は旧領域の解放も加わる |

`value` は葉にだけ、`front`・`back`・`pop_front`・`pop_back` は空でない根にだけ使います。`at` は `p < size(root)` が必要です。`copy_from` のコピー元は別のインスタンスにしてください。ムーブ後の根は移動先で使います。

共有している断片も `for_each` では列中に現れる回数だけ訪問します。断片の共有を繰り返すと、列の断片数は保持節点数よりはるかに大きくなり得ます。保持空間は $$O(A)$$、連結・分割などで追加する節点数は $$O(\log L)$$ です。対数の項は断片数が 0・1 の場合に定数として扱います。
