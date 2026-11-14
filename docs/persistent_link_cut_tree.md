---
title: 永続 Link Cut Tree
documentation_of: //tree/persistent_link_cut_tree.hpp
library_category: data_structure/persistent
---

頂点値を持つ無向森について、辺の追加・削除、頂点値の更新、経路の順に頂点値を `op` でまとめた結果を求めます。各操作は新しい版を返し、以前の版からも更新・照会できます。[Link Cut Tree](link_cut_tree.md) の頂点状態を [永続配列](persistent_array.md) に保存します。

```cpp
#include "lib/tree/persistent_link_cut_tree.hpp"
#include <cassert>
#include <vector>

long long add(long long a, long long b) { return a + b; }
long long zero() { return 0; }

int main() {
	PersistentLinkCutTree<long long, add, zero, true> tree(std::vector<long long>{2, 3, 5, 7});
	auto v0 = tree.initial_root();
	auto [v1, linked01] = tree.link(v0, 0, 1);
	auto [v2, linked12] = tree.link(v1, 1, 2);
	assert(linked01 && linked12);
	auto [v3, total] = tree.prod(v2, 0, 2);
	assert(total == 10);
	auto v4 = tree.set(v3, 1, 30);
	assert(tree.prod(v4, 2, 0).value == 37);
	assert(tree.prod(v2, 0, 2).value == 10);
	auto branch = tree.link(v1, 0, 3);
	assert(branch.value && tree.prod(branch.root, 1, 3).value == 12);
	assert(!tree.same(v1, 1, 3).value);
}
```

型は `PersistentLinkCutTree<S, op, e, commutative = false>` です。`S` はコピー・ムーブ構築と代入が可能な頂点値、`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たす演算です。`e()` は、`op(e(),x)`・`op(x,e())` がどちらも `x` になる値を返します。`S` の既定構築や等値比較は要求しません。第 4 テンプレート引数は、`op(a,b) == op(b,a)` が常に成り立つ場合のみ `true` にしてください。それ以外の演算では、経路の両方向の演算結果を保持します。

`root_type` は、保存した版を指定するための `std::uint32_t` の番号です。`initial_root()` が初期版で、版番号のコピーは時間・追加領域ともに $$O(1)$$ です。同じオブジェクトから返された有効な版番号だけを渡してください。値を自分で作ったり、別オブジェクトの版番号を混ぜたりしません。オブジェクト自体はコピー・ムーブできません。

照会を含む多くの操作は `result_type<T>` を返します。メンバー `root` は操作後の版、`value` は `T` 型の答えです。`auto [next, answer] = tree.prod(version,u,v);` のように構造化束縛で取り出せます。操作結果のメンバー `root` と、森の根頂点を調べる `root(version,v)` は異なります。

照会でも内部の探索用の木を調整し、新しい永続ノードを作る場合があります。引数の版は変わりません。次の操作には返された `root` を引き継ぐと、調整した探索用の木を再利用できます。`get` は探索用の木を変更せず、値だけを返します。

変更が不要な操作では、入力と同じ版番号を返す場合があります。異なる版番号でも、辺集合と頂点値が同じことがあります。

頂点数を $$n$$ として、`0 <= n < INT_MAX`、頂点番号はすべて `0 <= v < n` です。頂点数は構築後に変わりません。

| 操作 | 返り値・内容 |
| --- | --- |
| `PersistentLinkCutTree(int n = 0, span<const pair<int,int>> edges = {})` | 各頂点を `e()` で初期化した森を構築 |
| `PersistentLinkCutTree(const vector<S>& values, span<const pair<int,int>> edges = {})` | `values[v]` を頂点値とする森を構築 |
| `size()` / `empty()` | `int` の頂点数／`bool` の空判定 |
| `initial_root()` | `root_type` の初期版。後の操作でも変わらない |
| `get(version,v)` | `S` 型の頂点値。新しいノードを作らない |
| `set(version,v,value)` | 頂点値を `value` に変更した `root_type` |
| `build_forest(version,edges)` | 頂点値を保持し、辺を `edges` で置き換えた `root_type` |
| `link(version,u,v)` | `result_type<bool>`。別成分の頂点間に辺を追加し、成功なら `true` |
| `cut(version,u,v)` | `result_type<bool>`。辺 `u-v` を削除し、成功なら `true` |
| `same(version,u,v)` | `result_type<bool>`。同じ連結成分か |
| `prod(version,u,v)` | `result_type<S>`。`u` から `v` の順序で、両端を含む頂点値を集約 |
| `path_size(version,u,v)` / `distance(version,u,v)` | `result_type<int>`。経路の頂点数／辺数 |
| `jump(version,u,v,k)` | `result_type<int>`。`u` を 0 番目とする経路上の `k` 番目の頂点。範囲外は `-1` |
| `make_root(version,v)` | `v` をその成分の根にした `root_type` |
| `root(version,v)` | `result_type<int>`。`v` の成分の現在の根頂点 |
| `lca(version,r,u,v)` | `result_type<int>`。`r` を根にしたときの `u,v` の LCA |
| `reserve(count)` | `void`。全版を合わせた永続配列ノード数を予約 |
| `node_count()` / `memory_usage()` | `std::size_t`。未使用を表す特別なノードを除く、作成済み永続配列ノード数／永続領域と操作用配列の予約領域のバイト数 |

構築時と `build_forest` の `edges` は、`vector<pair<int,int>>` などで渡します。自己ループ・多重辺・閉路のない森であることが必要です。各成分の根の選び方は未規定です。根を指定するときは、構築後に `make_root` を使います。`edges` が空なら全頂点が孤立します。空の森も構築でき、`size()` は 0、`empty()` は `true` です。空の森では頂点を指定する操作はできません。

`link` は `u == v` または既に連結な場合に `false`、`cut` は `u == v` または指定した辺がない場合に `false` を返します。失敗しても返された版の探索用の木は調整される場合があります。辺集合と頂点値は変わりません。

`prod`・`path_size`・`distance`・`jump` の両端は連結であることが前提です。`jump` は連結な経路に対し、`k < 0` または `k >= path_size` なら `-1` です。`lca` の 3 頂点も同じ成分に属する必要があります。`u == v` の経路集約はその頂点値、頂点数は 1、距離は 0 です。

経路操作は、返す版で `u` を根にします。`lca` は `r` を根にします。`link`・`cut` は、異なる頂点を指定した場合はまず `u` を根にするため、失敗した場合も根が変わり得ます。成功した `link` の成分の根は、結合前の `v` の成分の根です。`cut` 成功後の 2 成分の根は、それぞれ `u` と `v` です。`same`・`root`・`get`・`set` は森の根を変えません。いずれも入力版の根・辺・値は保持されます。

ノード領域は版ごとに解放しません。不要な版番号を破棄しても、作成済み永続ノードはオブジェクトの破棄まで保持します。`reserve(count)` は `count < UINT32_MAX` とし、再確保後も既存の版番号は有効です。`memory_usage()` は `S` 自体が動的確保した領域を含みません。値のコピーや `op` に費用がかかる場合は、以下の計算量にもその費用が加わります。

頂点数を $$n$$、構築する辺数を $$m$$ とします。初期構築は $$O(n+m)$$ 時間・空間、`build_forest` は $$O(n\log n+m)$$ 時間・$$O(n\log n)$$ 追加領域です。`size`・`empty`・`initial_root`・`node_count`・`memory_usage` は $$O(1)$$、`get` は $$O(\log n)$$ 時間・$$O(1)$$ 追加領域です。`reserve` は再確保する場合、作成済み永続配列ノード数に線形の時間がかかります。

その他の操作で訪問する頂点数を $$k$$、通常の Link Cut Tree における処理量を $$r$$ とすると、操作内のキャッシュと永続配列の更新により時間は $$O(r+k\log n)$$、新しい永続領域は $$O(k\log n)$$ です。各操作の最悪計算量は時間・追加領域ともに $$O(n\log n)$$ です。別に、再利用する操作用領域を $$O(n)$$ 保持します。

`build_forest` と `reserve` を含まず、各操作が直前に返された版を引き継ぐ直線的な列では、1 操作あたりの計算量は償却 $$O(\log^2 n)$$ です。初期状態からの調整に必要な時間も含めると、$$q$$ 回の操作の時間・永続領域の上界は $$O((n+q)\log^2 n)$$ です。任意の過去版から分岐する操作列全体には、この償却保証は適用できません。同じ古い版を毎回照会し、返された版を捨てる場合も同様です。長さ 0・1 の森で有効な頂点操作は定数時間です。
