---
title: Union-Find
documentation_of: //data_structure/union_find.hpp
redirect_from:
- /data_structure/union_find/union_find.hpp.html
- /lightweight/data_structure/union_find/union_find.hpp.html
library_category: data_structure/union_find
---

頂点をグループに分け、2 つのグループの併合や、2 頂点が同じグループかの判定を行います。同じグループを連結成分、その中から代表として選ばれた 1 頂点を代表元と呼びます。

```cpp
#include "lib/data_structure/union_find.hpp"
#include <cassert>

int main() {
	UnionFind uf(5);
	uf.merge(0, 2);
	assert(uf.same(0, 2) && uf.size(0) == 2);
	assert(uf.vertex_count() == 5 && uf.size() == 5);
	assert(uf.component_count() == 4 && uf.count() == 4);
}
```

頂点は 0 始まりです。構築後の頂点数は固定です。`merge(a,b)` は併合後の代表元を返し、既に同じ成分なら変更しません。`leader(v)` は代表元、`size(v)` は所属成分の頂点数です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `size()` / `vertex_count()` | 全頂点数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `count()` / `component_count()` | 現在の連結成分数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |

`groups()` は代表元の昇順に成分を並べ、各成分の頂点を昇順に返します。経路短縮で内部状態を更新するため、検索操作と `groups()` は非 const です。コピーは独立します。頂点数は非負の `int`、頂点引数は範囲内であることが必要です。

頂点数を $$n$$ として、構築・空間は $$O(n)$$、併合・検索・成分サイズは償却 $$O(\alpha(n))$$、全頂点数と成分数は $$O(1)$$、`groups()` は $$O(n\alpha(n))$$ 時間・$$O(n)$$ 追加空間です。

$$\alpha(n)$$ は逆アッカーマン関数です。増加が非常に遅く、実用的な要素数では 5 以下です。「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
