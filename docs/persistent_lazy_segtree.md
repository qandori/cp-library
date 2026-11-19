---
title: 永続遅延セグメント木
documentation_of: //data_structure/persistent_lazy_segtree.hpp
redirect_from:
- /data_structure/segment_tree/persistent_lazy_segtree.hpp.html
- /lightweight/data_structure/segment_tree/persistent_lazy_segtree.hpp.html
- /data_structure/persistent/persistent_lazy_segtree.hpp.html
- /lightweight/data_structure/persistent/persistent_lazy_segtree.hpp.html
library_category: data_structure/persistent
---

更新前の配列も残し、区間更新・区間積・境界探索をそれぞれの状態ごとに行います。区間積は、区間内の要素を添字順に `op` でまとめた結果です。任意の過去のバージョンから分岐でき、別のバージョンの同じ区間をコピーできます。一点更新だけなら [永続セグメント木](persistent_segtree.md) を使えます。

```cpp
#include "lib/data_structure/persistent_lazy_segtree.hpp"
#include <cassert>

struct F { long long a, b; };
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(F f, long long x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
using Tree = PersistentLazySegtree<long long, op, e, F, mapping, composition, id>;

int main() {
	Tree seg(std::vector<long long>{1, 2, 3, 4});
	auto v0 = seg.initial_root();
	auto v1 = seg.apply(v0, 1, 4, {2, 1});
	auto v2 = seg.copy_range(v1, v0, 2, 4);
	assert(seg.prod(v0, 0, 4) == 10);
	assert(seg.all_prod(v1) == 22 && seg.all_prod(v2) == 13);
	assert(seg.max_right(v2, 0, [](long long s) { return s <= 9; }) == 3);
}
```

各更新は、新しいバージョンを識別する `root_type` 型の値（ルート）を返し、引数のバージョンは変わりません。`initial_root()` が最初のバージョンです。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去のバージョンから分岐できます。同じオブジェクトから得たルートだけを渡してください。バージョンごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

`op` は結合律 `op(op(a,b),c) == op(a,op(b,c))` を満たすようにします。`e()` は空区間の値となる単位元で、`op(e(),x) == x` と `op(x,e()) == x` を満たすようにします。`op(a,b) == op(b,a)` は不要です。

長さ `n` の区間を `op` でまとめた結果が `x` のとき、`mapping(f,x,n)` は区間の各要素に更新 `f` を適用した結果を返します。`composition(f,g)` は `g` の後に `f` を適用する更新、`id()` は何もしない更新です。

`mapping(id(),x,n) == x`、`mapping(composition(f,g),x,n) == mapping(f,mapping(g,x,n),n)` が必要です。また、長さ `nx`・`ny` の区間の結果を `x`・`y` とすると、`mapping(f,op(x,y),nx+ny) == op(mapping(f,x,nx),mapping(f,y,ny))` を満たすようにします。`S`・`F` はコピー・代入可能な型とし、等値比較・デフォルト構築は不要です。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `Tree(n = 0)` | 長さ `n`、各要素 `e()` の初期バージョン。未更新区間のノードは作らない | $$O(1)$$ |
| `Tree(a)` | `vector<S>` から初期バージョンを作る | $$O(n)$$ |
| `set(root,p,x)` | 位置 `p` に `x` を代入したバージョン | 償却 $$O(\log n)$$ |
| `apply(root,l,r,f)` | `[l,r)` に更新 `f` を適用したバージョン | 償却 $$O(\log n)$$ |
| `apply(root,p,f)` | 位置 `p` に更新 `f` を適用したバージョン | 償却 $$O(\log n)$$ |
| `get(root,p)` | 位置 `p` の値 | $$O(\log n)$$ |
| `prod(root,l,r)` | `[l,r)` の積。空区間は `e()` | $$O(\log n)$$ |
| `all_prod(root)` | 全体の積 | $$O(1)$$ |
| `copy_range(root,source,l,r)` | `[l,r)` を `source` の同じ区間で置換したバージョン | 償却 $$O(\log n)$$ |
| `max_right(root,l,f)` / `max_right<f>(root,l)` | `f(prod(root,l,r))` が真となる最大の右端 | $$O(\log n)$$ |
| `min_left(root,r,f)` / `min_left<f>(root,r)` | `f(prod(root,l,r))` が真となる最小の左端 | $$O(\log n)$$ |
| `size()` / `empty()` | 固定長／長さが 0 か | $$O(1)$$ |

`0 <= n <= INT_MAX`、一点操作は `0 <= p < n`、区間操作は `0 <= l <= r <= n` とします。境界探索の `f` は区間の計算結果を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返すようにします。`f(e()) == true` が必要です。区間を伸ばして一度 `false` になったら、その後は `true` に戻らない条件を使ってください。空区間・同じバージョン・全区間のコピーはノードを作りません。取得・区間積・境界探索もノードを作りません。

`reserve(count)` は全バージョンを合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新・区間コピーの時間は、領域確保を含む償却計算量です。

全バージョンを合わせた作成済みノード数を $$K$$、確保容量を $$C$$ とすると、保持空間は $$O(C)$$、オブジェクト自体のコピーは $$O(K)$$ 時間・追加空間です。コピー代入には、代入先の旧ノードを破棄する時間も加わります。`initial_root()`・`node_count()`・`memory_usage()` は $$O(1)$$、`reserve(count)` は容量が十分なら $$O(1)$$、再確保時は $$O(K)$$ 時間です。作成済みノード数と `count` は $$2^{32}-2$$ 以下とし、ノードがない場合にもコピー・予約には定数時間かかります。

構築は配列指定なら $$O(n)$$、長さ指定なら $$O(1)$$ です。各更新・区間コピーは償却 $$O(\log n)$$ 時間、区間積・境界探索は $$O(\log n)$$ 時間、更新・コピーごとの追加領域は $$O(\log n)$$ です。長さ 0・1 の操作は定数時間です。
