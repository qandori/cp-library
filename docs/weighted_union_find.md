---
title: 重み付き Union-Find
documentation_of: //data_structure/weighted_union_find.hpp
redirect_from:
- /data_structure/union_find/weighted_union_find.hpp.html
- /lightweight/data_structure/union_find/weighted_union_find.hpp.html
library_category: data_structure/union_find
---

各頂点に対応する値を `potential` と書き、頂点どうしの値の差を管理します。絶対的な値を決めずに、「頂点 `y` の値が頂点 `x` より `w` 大きい」という関係を追加できます。

```cpp
#include "lib/data_structure/weighted_union_find.hpp"
#include <cassert>

int main() {
	WeightedUnionFind<std::int64_t> uf(3);
	assert(uf.merge(0, 1, 5));
	assert(uf.diff(0, 1) == 5);
	assert(uf.size() == 3 && uf.vertex_count() == 3);
	assert(uf.count() == 2 && uf.component_count() == 2);
	assert(!uf.merge(0, 1, 4));
	assert(uf.component_count() == 2);
}
```

以下では頂点数を $$n$$ とします。$$\alpha(n)$$ は逆アッカーマン関数で、増加が非常に遅く、実用的な頂点数では 5 以下です。`T` の演算・コピーは定数時間とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WeightedUnionFind<T>(n)` | `n` 頂点を独立した成分として構築 | $$O(n)$$ |
| `size()` / `vertex_count()` | 全頂点数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `count()` / `component_count()` | 現在の連結成分数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `merge(x,y,w)` | $$\mathrm{potential}[y]-\mathrm{potential}[x]=w$$ を追加。矛盾時に false | 償却 $$O(\alpha(n))$$ |
| `same(x,y)` | 連結判定 | 償却 $$O(\alpha(n))$$ |
| `leader(x)` / `size(x)` | 代表元／連結成分の頂点数 | 償却 $$O(\alpha(n))$$ |
| `weight(x)` | 代表頂点の値を 0 としたときの `x` の値 | 償却 $$O(\alpha(n))$$ |
| `diff(x,y)` | 連結な頂点間の $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ | 償却 $$O(\alpha(n))$$ |

`merge(x, y, w)` は $$\mathrm{potential}[y]-\mathrm{potential}[x]=w$$ を追加し、矛盾する場合に `false` を返します。`diff(x, y)` は連結な頂点間の $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ を返します。

`same(x, y)` で連結判定、`leader(x)` で代表元、`size(x)` で連結成分の頂点数を取得できます。頂点番号は 0 始まりです。

頂点数は `0 <= n <= INT_MAX`、頂点の引数は `0 <= x,y < n` とします。`T` はコピー・代入・ゼロによる構築・加減算・符号反転・等値比較に対応し、`T{}` と `T(0)` がともに 0 を表す型を使います。符号付き整数や modint を利用でき、差の計算や更新途中の値も `T` に収まる必要があります。検索時に内部状態を更新するため、問い合わせも非 `const` です。

例では頂点 1 の値が頂点 0 より 5 大きいという関係を登録しています。絶対的な値を指定する必要はありません。`diff` の前に `same` で連結を確認してください。既存の関係と矛盾する `merge` は false を返し、関係を変更しません。

構築・空間は $$O(n)$$、頂点数・成分数の取得は $$O(1)$$ 時間・追加空間です。その他の操作は償却 $$O(\alpha(n))$$ です。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
