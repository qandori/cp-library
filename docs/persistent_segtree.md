---
title: 永続セグメント木
documentation_of: //data_structure/persistent_segtree.hpp
redirect_from:
- /data_structure/segment_tree/persistent_segtree.hpp.html
- /lightweight/data_structure/segment_tree/persistent_segtree.hpp.html
- /data_structure/persistent/persistent_segtree.hpp.html
- /lightweight/data_structure/persistent/persistent_segtree.hpp.html
library_category: data_structure/persistent
---

配列の過去の状態を残したまま、一点更新・区間積・境界探索を行います。任意の過去のバージョンから分岐でき、別のバージョンの区間をコピーすることもできます。遅延伝搬による区間更新は扱いません。

```cpp
#include "lib/data_structure/persistent_segtree.hpp"
#include <cassert>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
using Tree = PersistentSegtree<long long, op, e>;

int main() {
	Tree seg(std::vector<long long>{1, 2, 3, 4});
	seg.reserve(1000);
	auto v0 = seg.initial_root();
	auto v1 = seg.set(v0, 1, 10);
	auto v2 = seg.multiply(v0, 3, 5);
	auto v3 = seg.copy_range(v1, v2, 2, 4);
	assert(seg.prod(v0, 0, 4) == 10);
	assert(seg.prod(v1, 0, 4) == 18);
	assert(seg.get(v2, 3) == 9);
	assert(seg.all_prod(v3) == 23);
	auto r = seg.max_right(v3, 0, [](long long s) { return s <= 14; });
	assert(r == 3);
}
```

`v0` は最初の配列、`v1` は位置 1 を 10 にしたバージョン、`v2` は最初の配列の位置 3 に 5 を加えたバージョンです。`v1` と `v2` は互いに独立しています。`v3` は `v1` の区間 `[2,4)` だけを `v2` の値で置き換えたバージョンです。更新を呼んでも元のバージョンは変わらず、返されたルート ID を保存して新しいバージョンを使います。

`op` は結合律 `op(op(a,b),c) == op(a,op(b,c))` を満たすようにします。`e()` は空区間の値となる単位元で、`op(e(),x) == x` と `op(x,e()) == x` を満たすようにします。和以外に最小値・最大値・行列積・関数合成などを扱えます。区間積は、添字の小さい順に `op` でまとめた結果です。`op(a,b) == op(b,a)` は不要です。`S` はコピー・ムーブ・代入できる型とし、デフォルト構築と等値比較は不要です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PersistentSegtree(n = 0)` | 長さ `n`、全要素が `e()` の初期バージョンを用意。実ノードはまだ作らない | $$O(1)$$ |
| `PersistentSegtree(a)` | `std::vector<S>` から初期バージョンを構築 | $$O(n)$$ |
| `initial_root()` | コンストラクタで用意した初期バージョンのルート ID | $$O(1)$$ |
| `set(root,p,x)` | バージョン `root` の位置 `p` を `x` に代入した新しいルート ID | 償却 $$O(\log n)$$ |
| `multiply(root,p,x)` | 位置 `p` を `op(現在値,x)` にした新しいルート ID | 償却 $$O(\log n)$$ |
| `get(root,p)` | バージョン `root` の位置 `p` の値 | $$O(\log n)$$ |
| `prod(root,l,r)` | バージョン `root` の半開区間 `[l,r)` の積。空区間は `e()` | $$O(\log n)$$ |
| `all_prod(root)` | バージョン `root` の全体の積 | $$O(1)$$ |
| `copy_range(root,source,l,r)` | `root` の `[l,r)` を `source` の同じ区間で置き換えた新しいルート ID | 償却 $$O(\log n)$$ |
| `max_right(root,l,f)` / `max_right<f>(root,l)` | `f(prod(root,l,r))` が真である最大の右端 `r` | $$O(\log n)$$ |
| `min_left(root,r,f)` / `min_left<f>(root,r)` | `f(prod(root,l,r))` が真である最小の左端 `l` | $$O(\log n)$$ |
| `size()` / `empty()` | 配列の長さ／長さが 0 か | $$O(1)$$ |
| `reserve(count)` | 空ノードを除くノード `count` 個分の容量を予約。既存のルート ID は変わらない | 再確保時は現在のノード数に比例 |
| `node_count()` | 空ノードを除く、全バージョンで作成したノード数 | $$O(1)$$ |
| `memory_usage()` | 予約済みノード領域のバイト数。内部の空ノード 1 個分も含む | $$O(1)$$ |

ルート ID は保存した配列の状態を識別する値で、その型は `root_type`（`std::uint32_t`）、値の型は `value_type`（`S`）です。ルート ID を変数や `std::vector<Tree::root_type>` にコピーするだけなら、時間・追加領域ともに一定です。返される ID は連番の「バージョン番号」ではないので、バージョン番号から ID への対応は利用者が保存してください。

全バージョンを合わせた作成済みノード数を $$K$$ とすると、オブジェクト自体のコピーは $$O(K)$$ 時間・追加空間です。コピー代入には、代入先の旧ノードを破棄する時間も加わります。`reserve` は容量が十分なら $$O(1)$$、再確保時は $$O(K)$$ 時間で、ノードがない場合のコピー・予約は定数時間です。

添字は 0 始まりで、`0 <= n <= INT_MAX`、一点操作は `0 <= p < n`、区間操作は `0 <= l <= r <= n` とします。ルート ID はその木の `initial_root`・更新・区間コピーが返したものを使います。別の木の ID を混ぜることはできません。木のオブジェクト自体をコピーするとノード領域全体を複製します。移動元は再代入してから使ってください。

境界探索の `f` は区間の計算結果を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返すようにします。`f(e()) == true` が必要です。区間を伸ばして一度 `false` になったら、その後は `true` に戻らない条件を使ってください。例えば非負の配列なら「和が指定値以下」を使えます。負の値がある配列の和では、この条件を満たすとは限りません。

`get`・`prod`・`all_prod`・境界探索は読み取り専用で、ノードを増やしません。区間コピーは部分木を共有し、全区間のコピー、空区間のコピー、同じバージョンからのコピーはノードを作りません。バージョンごとのノード解放は行わず、使わなくなったバージョンのノードも木を破棄するまで保持します。

長さが正のとき、$$h=\lceil\log_4 n\rceil$$ とすると、配列からの構築は $$\displaystyle\sum_{i=0}^{h}\lceil n/4^i\rceil$$ ノード、一点更新は $$h+1$$ ノードを作ります。`reserve` の引数は更新回数ではなく、既存分を含むノード数なので、更新回数からこの上限を見積もって予約できます。ノード数の上限は $$2^{32}-2$$ 個です。

演算・値のコピーを $$O(1)$$ とした計算量です。構築は $$O(n)$$、一点更新・区間積・境界探索・区間コピーは $$O(\log n)$$、更新の時間は領域確保を含む償却計算量です。長さ 0・1 の操作は $$O(1)$$ です。構築後に更新・区間コピーを合計 $$u$$ 回行うと、空間は $$O(n+u\log n)$$、長さだけを指定した疎な初期バージョンでは $$O(u\log n)$$ です。予約容量が大きい場合はその容量分を使います。
