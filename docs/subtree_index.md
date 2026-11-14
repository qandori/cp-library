---
title: 部分木の区間番号
documentation_of: //tree/subtree_index.hpp
library_category: tree/query
---

親配列から、各部分木を連続した半開区間に対応付けます。部分木の集計と祖先判定に使えます。

```cpp
#include "lib/tree/subtree_index.hpp"
#include <cassert>
#include <vector>

int main() {
	SubtreeIndex tree({-1, 0, 0, 1, 1});
	auto [l, r] = tree.subtree(1);
	assert(r - l == 3);
	assert(tree.is_ancestor(1, 4) && !tree.is_ancestor(2, 4));
	std::vector<int> value{10, 20, 30, 40, 50}, ordered(tree.size());
	for (int v = 0; v < tree.size(); ++v) ordered[tree.in[v]] = value[v];
	int sum = 0;
	for (int i = l; i < r; ++i) sum += ordered[i];
	assert(sum == 110);
}
```

`SubtreeIndex` は根付き木の部分木を扱います。経路分解・最小共通祖先（LCA）・祖先へのジャンプも必要なら [HLD](hld.md) を使います。

| 操作・メンバー | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SubtreeIndex()` | 空の構造を作る | $$O(1)$$ |
| `SubtreeIndex(parents)` | `std::vector<int>` の親配列から構築 | $$O(n)$$ |
| `build_from_parents(parents)` | 親配列で構築し直す。以前の内容は置き換える。返り値は `void` | $$O(n)$$ |
| `size()` / `empty()` | 頂点数を `int`／空かを `bool` で返す | $$O(1)$$ |
| `root()` | 根の頂点番号。空なら `-1` | $$O(1)$$ |
| `in[v]` / `out[v]` | 頂点 `v` の部分木の左端／右端 | $$O(1)$$ |
| `order[i]` | 位置 `i` にある元の頂点番号 | $$O(1)$$ |
| `subtree(v,edge = false)` | 部分木の区間を `std::pair<int,int>` で返す。`edge` が真なら `v` 自身の位置を除く | $$O(1)$$ |
| `is_ancestor(u,v)` | `u` が `v` の祖先かを `bool` で返す。自分自身も祖先に含む | $$O(1)$$ |
| `clear()` | 全頂点を削除して空にする | $$O(n)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

親配列の長さを頂点数 $$n$$ とし、`n <= INT_MAX` とします。頂点番号は `[0,n)` です。根の親は `-1`、それ以外は親の頂点番号を指定します。非空なら根はちょうど1つで、全頂点が根につながる木を渡してください。親番号と子番号の大小関係は任意です。空配列も扱えます。

`in`・`out`・`order` は長さ `n` の `std::vector<int>` です。各部分木は `[in[v],out[v])` となり、`order[in[v]] == v` です。根は位置0に置かれます。兄弟の順序は未規定です。`subtree(v,true)` は `[in[v]+1,out[v])` で、葉では空区間です。辺値を子の位置へ置いた場合、部分木の内部の辺だけを集計できます。

構築に渡した親配列は変更しません。以降使わない場合は `std::move(parents)` と渡して複製を省けます。公開の配列は読み取りに使い、問い合わせの間は変更しないでください。コピーは独立して更新でき、移動元は空になります。構築・削除・代入後は以前の配列要素への参照を使わないでください。

構築は $$O(n)$$ 時間・追加空間、保存空間は $$O(n)$$ です。コピーは $$O(n)$$、移動・交換は $$O(1)$$ 時間です。
