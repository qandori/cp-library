---
title: Suffix Locus Index
documentation_of: //components/suffix_locus.hpp
library_category: components/string
---

構築済みの `SuffixTree` に対して、本文中の半開区間 `[l,r)` が表す部分文字列の位置を求める内部部品です。文字列の比較やハッシュを使わず、木の上で目的の部分文字列の長さに対応する位置を探します。

```cpp
#include "lib/components/suffix_locus.hpp"

int main() {
	SuffixTree tree("banana");
	SuffixLocusIndex index(tree);
	auto locus = index.locate(1, 4);
	assert(locus.length == 3);
	assert(tree.occurrence_count(locus.node) == 2);
	auto same = index.locate(3, 6);
	assert(locus.node == same.node && locus.length == same.length);
	assert(index.locate(6, 6).node == SuffixTree::root);
}
```

`locate(l,r)` は `SuffixTree::Locus` を返します。`node` は部分文字列の終点が属する辺の行き先、`length` は `r-l` です。終点が頂点と一致するときはその頂点になります。区間には `0 <= l <= r <= size()` を渡してください。空区間は位置にかかわらず `{0,0}`、同じ内容を持つ重複した部分文字列は同じ位置になります。本文の区間を指定するため、不在という結果はありません。

`SuffixLocusIndex()` はメモリを確保せず、空の本文に対応する索引を作ります。`SuffixLocusIndex(const SuffixTree &tree)` と `build(const SuffixTree &tree)` は、その時点の木の頂点番号・深さ・接尾辞の終点をコピーして索引を作ります。引数の既定値はありません。本文の長さとバイトの扱いは `SuffixTree` に従い、全 256 バイトに対応し、長さは `INT_MAX/2` 未満です。

索引は元の木への参照を保持せず、元の木の変更・破棄後も検索できます。ただし、返された頂点番号を木の操作に渡す場合は、索引の構築時と同じ頂点番号を保持する木を使ってください。別の本文から再構築した木には渡せません。`build` がメモリ確保に失敗した場合は元の索引を維持します。ムーブと `swap` は `noexcept` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SuffixLocusIndex()` | 空の索引 | $$O(1)$$ |
| `SuffixLocusIndex(tree)` | 木から索引を構築 | $$O(n)$$ |
| `build(tree)` | 木から再構築、返り値は `void` | $$O(n)$$、旧索引の解放時間も加わる |
| `size()` | 本文長を `int` で返す | $$O(1)$$ |
| `locate(l,r)` | 区間の `SuffixTree::Locus` | $$O(\log n)$$、空区間は $$O(1)$$ |
| `swap(other)` | 2 つの索引を交換、返り値は `void` | $$O(1)$$ |
| コピー構築・コピー代入 | 独立した索引を作る | $$O(n)$$、代入は旧索引の解放時間も加わる |
| ムーブ構築・ムーブ代入 | 索引を移し、移動元を空の索引に戻す | 構築 $$O(1)$$、代入は旧索引の解放時間も加わる |

$$n$$ は本文長です。空入力と長さ 1 では検索時間を定数として扱います。構築時に `SuffixTree` の木構造がまだ生成されていなければ、その生成も行います。構築時間・保持空間・構築時の補助空間はいずれも $$O(n)$$ です。
