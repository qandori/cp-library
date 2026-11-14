---
title: 辺の値を扱う Link Cut Tree
documentation_of: //tree/edge_link_cut_tree.hpp
library_category: tree/dynamic
---

辺の追加・削除・値の更新を行いながら、経路上の辺の値を順にまとめます。各辺には、通る方向ごとに別の値を指定できます。頂点に値を持たせる場合は [Link Cut Tree](link_cut_tree.md) を使います。

`EdgeLinkCutTree<S,op,e>` の `S` は辺の値の型です。`op(a,b)` は `a` の後に `b` を使った結果とし、`op(op(a,b),c) == op(a,op(b,c))` を満たすように定義します。`e()` は辺を通らない場合の値で、`op(e(),x)` と `op(x,e())` がどちらも `x` になる必要があります。

例えば、各辺のアフィン変換を通過順に合成できます。`forward` と `backward` は独立の変換で、逆関数である必要はありません。係数が 0 の変換も指定できます。

```cpp
#include "lib/tree/edge_link_cut_tree.hpp"
#include "lib/math/mint.hpp"
#include <cassert>

struct Affine { mint a, b; };
Affine op(const Affine &first, const Affine &second) {
	return {second.a * first.a, second.a * first.b + second.b};
}
Affine e() { return {1, 0}; }
mint apply(Affine f, mint x) { return f.a * x + f.b; }

int main() {
	EdgeLinkCutTree<Affine, op, e> tree(3);
	tree.link(0, 1, {2, 3}, {5, 7});
	tree.link(1, 2, {3, 4}, {0, 11});
	assert(apply(tree.prod(0, 2), 5) == mint(43));
	assert(apply(tree.prod(2, 0), 5) == mint(62));
	assert(apply(tree.prod(1, 1), 5) == mint(5));

	tree.set(1, 0, {7, 1}, {4, 9});
	assert(apply(tree.prod(0, 1), 5) == mint(36));
	auto edge = tree.get(1, 0);
	assert(edge && edge->from == 0 && edge->to == 1);
	assert(edge->forward.a == mint(7) && edge->backward.a == mint(4));

	tree.cut(1, 0);
	assert(!tree.same(0, 2));
	tree.link(1, 0, {2, 8}, {3, 5});
	assert(apply(tree.prod(1, 0), 5) == mint(18));
	assert(apply(tree.prod(0, 1), 5) == mint(20));
}
```

`link(u,v,forward,backward)` が成功したときの `u` から `v` を、その辺の順方向とします。`cut`・`set`・`get` の端点は逆順に渡しても構いません。ただし、`set` の `forward`・`backward` は常に開設時の順方向・逆方向に対応します。上の `set(1,0,...)` も、`0 → 1` の変換を `7x+1` に更新します。`get` が返す端点と値の順序も開設時の向きです。

`make_root` や経路の問い合わせで開設時の向きは変わりません。辺を削除してから再び追加した場合は、新しい `link` の端点順を使います。

`link`・`cut`・`get`・`set`・経路の問い合わせは、森の内部の根を変更することがあります。根を指定して操作したい場合は直前に `make_root` を呼びます。`lca(r,u,v)` は引数 `r` を根として計算します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `EdgeLinkCutTree(n = 0)` | `n` 頂点、辺なしの森を構築 | $$O(n)$$ |
| `size()` / `edge_count()` | 頂点数／現在の辺数を `int` で返す | $$O(1)$$ |
| `link(u,v,value)` | 両方向の値が `value` の辺を追加。成功時 `true` | 償却 $$O(\log n)$$ |
| `link(u,v,forward,backward)` | `u → v` と `v → u` に別の値を指定して辺を追加。成功時 `true` | 償却 $$O(\log n)$$ |
| `cut(u,v)` | 両端が `u,v` の辺を削除。存在しなければ `false` | 償却 $$O(\log n)$$ |
| `set(u,v,value)` | 両方向の値を `value` に更新。辺が存在しなければ `false` | 償却 $$O(\log n)$$ |
| `set(u,v,forward,backward)` | 開設時の順方向・逆方向の値を更新。辺が存在しなければ `false` | 償却 $$O(\log n)$$ |
| `get(u,v)` | 開設時の端点と両方向の値を `std::optional<Edge>` で返す。辺が存在しなければ `std::nullopt` | 償却 $$O(\log n)$$ |
| `same(u,v)` | 連結なら `true` | 償却 $$O(\log n)$$ |
| `prod(u,v)` | `u` から `v` へ通る各辺の、その方向の値を順にまとめて `S` で返す | 償却 $$O(\log n)$$ |
| `path_size(u,v)` / `distance(u,v)` | 経路の頂点数／辺数を `int` で返す | 償却 $$O(\log n)$$ |
| `jump(u,v,k)` | `u` を 0 番目とする経路上の第 `k` 頂点を `int` で返す。範囲外は -1 | 償却 $$O(\log n)$$ |
| `make_root(v)` / `root(v)` | 根の変更／根の頂点番号の取得。返り値は `void`／`int` | 償却 $$O(\log n)$$ |
| `lca(r,u,v)` | `r` を根として、`r → u` と `r → v` の共通部分で最も遠い頂点を `int` で返す | 償却 $$O(\log n)$$ |

`n,u,v,r,k` は `int` です。頂点番号は `[0,n)` とし、`0 <= n <= INT_MAX/2` が必要です。辺の値は `S` で渡します。`Edge` は `int from,to` と `S forward,backward` を持ち、`get` はそれらをコピーして返します。

`link` は、自己ループ、既に連結な頂点同士の追加を拒否して `false` を返します。既存の辺を上書きしません。`cut`・`set` は成功時に `true` を返し、存在しない辺を指定した場合は森を変更しません。

`prod`・`path_size`・`distance`・`jump` の両端、および `lca` の 3 頂点は、同じ連結成分に属する必要があります。非連結の可能性がある場合は、先に `same` で調べます。`u == v` では `prod` は `e()`、`path_size` は 1、`distance` は 0 です。`jump(u,v,k)` は `0 <= k < path_size(u,v)` なら頂点番号を返します。空の森は `size() == edge_count() == 0` で、有効な頂点番号はありません。

両方向で値が同じ場合は、1 個の値を渡す形式を使えます。次の例は辺の重みの和です。

```cpp
#include "lib/tree/edge_link_cut_tree.hpp"
#include <cassert>
#include <cstdint>

std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }

int main() {
	EdgeLinkCutTree<std::int64_t, op, e> tree(4);
	tree.link(0, 1, 4);
	tree.link(1, 2, -2);
	assert(tree.prod(0, 2) == 2 && tree.prod(2, 0) == 2);
	assert(tree.distance(0, 2) == 2 && tree.path_size(0, 2) == 3);
	assert(tree.jump(2, 0, 1) == 1 && !tree.same(0, 3));
	tree.set(1, 0, 7);
	assert(tree.prod(0, 2) == 5);
	tree.make_root(2);
	assert(tree.root(0) == 2 && tree.lca(2, 0, 1) == 1);
}
```

頂点数を $$n$$ として、構築・使用空間は $$O(n)$$ です。辺を削除した領域を再利用するため、過去の追加回数ではなく頂点数に比例した領域を使います。構築を除き、`size()`・`edge_count()` 以外の表中の操作は償却 $$O(\log n)$$、1 回の最悪時間は $$O(n)$$ です。「償却」は、一連の操作にかかる合計時間から求めた 1 回あたりの計算量を表します。`op`・`e` と値のコピー・交換は定数時間とします。

コピー構築は $$O(n)$$ 時間・空間で、コピー後は独立して変更できます。コピー代入ではコピー元と代入先の頂点数の和に比例する時間がかかります。移動構築は $$O(1)$$ 時間、移動代入では代入先の旧領域の解放時間が加わります。移動元は頂点数・辺数が 0 の空の森になります。
