---
title: Top Tree
documentation_of: //tree/top_tree.hpp
library_category: tree/dynamic
---

辺の追加・削除で形が変わる森の木 DP を管理します。経路をつなぐ `compress` と、同じ頂点へ接続する枝をまとめる `rake` を定義します。結合を取り消す逆演算は不要で、部分木の最小値・直径・独立集合の数なども扱えます。

このページでは、木の一部をまとめて計算する単位をクラスタと呼びます。`Path` は向き付きの 1 本の経路と、そこから伸びる枝を合わせた情報です。この区別して保持する経路を「境界の経路」と呼びます。`Point` は、親の頂点をまだ加えていない枝の情報です。

```cpp
#include "lib/tree/top_tree.hpp"
#include <iostream>

struct SumDP {
	using Vertex = long long;
	using Point = long long;
	using Path = long long;
	static Point point_identity() { return 0; }
	static Point rake(Point a, Point b) { return a + b; }
	static Path compress(Path a, Path b) { return a + b; }
	static Point add_edge(Path a) { return a; }
	static Path add_vertex(Point a, Vertex v) { return a + v; }
};

int main() {
	TopTree<SumDP> tree({1, 2, 3, 4});
	tree.link(0, 1);
	tree.link(1, 2);
	tree.link(1, 3);
	std::cout << tree.all_prod(0) << '\n';        // 10
	std::cout << tree.subtree_prod(0, 1) << '\n'; // 9
	tree.set(2, 7);
	tree.cut(1, 3);
	std::cout << tree.all_prod(2) << '\n';        // 10
	std::cout << tree.all_prod(3) << '\n';        // 4
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `TopTree<DP>()` | 頂点のない森を作る | $$O(1)$$ |
| `TopTree<DP>(values)` | 頂点番号順の `vector<Vertex>` から、辺のない森を作る | $$O(n)$$ |
| `size()` / `empty()` | 頂点数の `int`／頂点がないかの `bool` | $$O(1)$$ |
| `get(v)` | 頂点値の `const Vertex&` | $$O(1)$$ |
| `set(v,value)` | 頂点値を `const Vertex&` で置き換える。返り値は `void` | 償却 $$O(\log n)$$ |
| `same(u,v)` | 同じ連結成分かの `bool` | 償却 $$O(\log n)$$ |
| `link(u,v)` | 無向辺を追加し、成功時に `true` を返す | 償却 $$O(\log n)$$ |
| `cut(u,v)` | 無向辺を削除し、存在した場合に `true` を返す | 償却 $$O(\log n)$$ |
| `all_prod(root)` | `root` を根とする連結成分全体の `Path` | 償却 $$O(\log n)$$ |
| `subtree_prod(root,v)` | `root` を根とする `v` の部分木の `Path` | 償却 $$O(\log n)$$ |
| `prod(u,v)` | `u` から `v` への経路に沿って、枝も含めた連結成分全体を集約した `Path` | 償却 $$O(\log n)$$ |
| `Vertex` / `Point` / `Path` | `DP` に指定した型の別名 | —（型） |

頂点番号は `0 <= v < size()` です。空の森では `size()`・`empty()` のみ使用できます。孤立点も扱えます。`link` は自己ループ・多重辺・閉路を作る場合に `false` を返します。`cut` は辺がない場合や自己ループの指定で `false` を返します。失敗しても頂点値と辺集合は変わりません。頂点の追加・削除、遅延更新は扱いません。

`subtree_prod(root,v)` の 2 頂点と `prod(u,v)` の 2 頂点は連結である必要があります。根はクエリごとに指定でき、以前の操作の根に依存しません。`all_prod(root)` と `subtree_prod(root,v)` は、それぞれ `root`・`v` の 1 頂点を境界の経路とし、そのほかを枝としてまとめた値です。孤立点では `add_vertex(point_identity(), get(v))` になります。

`get` の参照は、そのオブジェクトの破棄・代入まで有効です。`set` は参照先の値を書き換えます。オブジェクトをコピーした場合は独立した森になります。DP の値はコピー・代入・交換可能な型にします。

### DP の定義

| 型・関数 | 引数と返り値 | 計算量 |
| --- | --- | --- |
| `DP::Vertex` | 頂点 1 個の情報 | —（型） |
| `DP::Point` | 同じ親へ接続する、互いに交わらない枝の集約。親の頂点値は含めない | —（型） |
| `DP::Path` | 向きのある経路と、その経路に付く枝の集約 | —（型） |
| `DP::point_identity()` | 枝がない状態の `Point` を返す | $$O(1)$$ を仮定 |
| `DP::rake(a,b)` | `Point` 同士を併合し、`Point` を返す | $$O(1)$$ を仮定 |
| `DP::compress(a,b)` | `Path` の `a` の末尾と `b` の先頭を辺でつなぎ、`a` から `b` の向きの `Path` を返す。頂点の重複はない | $$O(1)$$ を仮定 |
| `DP::add_edge(path)` | `Path` の先頭を親へ接続する 1 本の枝として扱い、`Point` を返す。親は含めない | $$O(1)$$ を仮定 |
| `DP::add_vertex(point,vertex)` | 枝を表す `Point` に親の `Vertex` を加え、その頂点だけを境界の経路とする `Path` を返す | $$O(1)$$ を仮定 |

`rake` は `rake(rake(a,b),c) == rake(a,rake(b,c))` と `rake(a,b) == rake(b,a)` を満たすようにします。`point_identity()` を左右どちらに渡しても他方の値がそのまま返ることが必要です。`compress` は `compress(compress(a,b),c) == compress(a,compress(b,c))` を満たすようにします。空の経路は扱わないため、`path_identity()` は不要です。`compress` の逆方向の値はライブラリ内部で同じ頂点を逆順に集約して保持するので、`reverse()` も不要です。`Point` の枝には順序を持たせられません。

さらに、同じ木を異なる順序で `compress`・`rake` しても、同じ DP を表す必要があります。特に、子の経路を主経路に続ける場合と、枝としてまとめる場合を一致させます。`Point p`・`Vertex v`・`Path q` に対して、次の 2 つは同じ `Point` を表すように定義します。

```cpp
DP::add_edge(DP::compress(DP::add_vertex(p, v), q))
DP::add_edge(DP::add_vertex(DP::rake(p, DP::add_edge(q)), v))
```

`prod(u,v)` は、経路外の枝も含むクラスタです。上の `SumDP` では経路和ではなく連結成分の総和になります。経路上の頂点だけを集約したい場合は、`Point` を情報を持たない型とし、`rake`・`add_edge` で枝の情報を捨て、`add_vertex` で頂点自身だけを使います。[順序付き一次関数合成の DP](https://github.com/qandori/cp-library/blob/main/test/fixtures/top_tree_dp.hpp) がこの例です。経路積だけが必要なら [Link Cut Tree](https://qandori.github.io/cp-library/tree/link_cut_tree.hpp.html) も使用できます。

### 辺の値と直径

辺に値を持たせる場合は、各辺を `u - e - v` のように中間頂点 `e` で分割し、その頂点に辺の値を保存します。根が変わるため、片方の端点に親辺の値を保存する方法は使用できません。辺を消す場合は 2 本の辺を `cut` します。必要な中間頂点は最初から `values` に用意しておきます。

次は、非負の頂点重みのパスについて、重みの和の最大値を管理する例です。元の頂点を重み 0、中間頂点を辺の長さにすれば、辺重み付きの直径を管理できます。

```cpp
#include "lib/tree/top_tree.hpp"
#include <algorithm>
#include <iostream>

struct DiameterDP {
	using Vertex = long long;
	struct Point { long long first = 0, second = 0, diameter = 0; };
	struct Path { long long length, left, right, diameter; };
	static Point point_identity() { return {}; }
	static Point rake(Point a, Point b) {
		return {std::max(a.first, b.first), std::max({a.second, b.second, std::min(a.first, b.first)}), std::max(a.diameter, b.diameter)};
	}
	static Point add_edge(const Path& a) { return {a.left, 0, a.diameter}; }
	static Path add_vertex(Point a, Vertex v) {
		return {v, v + a.first, v + a.first, std::max(a.diameter, v + a.first + a.second)};
	}
	static Path compress(const Path& a, const Path& b) {
		return {a.length + b.length, std::max(a.left, a.length + b.left), std::max(b.right, b.length + a.right), std::max({a.diameter, b.diameter, a.right + b.left})};
	}
};

int main() {
	TopTree<DiameterDP> tree({0, 0, 0, 5, 7});
	tree.link(0, 3);
	tree.link(3, 1);
	tree.link(1, 4);
	tree.link(4, 2);
	std::cout << tree.all_prod(0).diameter << '\n'; // 12
	tree.set(3, 9);
	std::cout << tree.all_prod(2).diameter << '\n'; // 16
	tree.cut(1, 4);
	std::cout << tree.all_prod(0).diameter << '\n'; // 9
}
```

この DP の `length` は境界の経路の重み和、`left`・`right` はそれぞれ先頭・末尾からの最大パス和、`diameter` はクラスタ内の最大パス和です。`Point` は枝の高さの最大 2 個と、枝の内部の最大パス和を保持します。重みは非負とし、加算で `long long` があふれないようにします。

[Toptree 導入編](https://niuez.hatenablog.com/entry/2019/08/04/191420) の compress・rake と、[ei1333 の Lazy Top Tree](https://ei1333.github.io/library/structure/dynamic-tree/lazy-top-tree.hpp) を参考にしています。

頂点数を $$n$$ とし、DP の演算・値のコピーを定数時間・定数空間とすると、構築は $$O(n)$$ 時間、空間は $$O(n)$$ です。`get`・`size`・`empty` は $$O(1)$$、辺の追加・削除、連結判定、頂点更新、集約は償却 $$O(\log n)$$ です。
