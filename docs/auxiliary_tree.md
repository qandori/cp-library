---
title: Auxiliary Tree
documentation_of: //tree/auxiliary_tree.hpp
redirect_from:
- /graph/virtual_tree.hpp.html
- /graph/auxiliary_tree.hpp.html
library_category: tree
---

大きな木から、指定した頂点と、それらをつなぐために必要な LCA だけを残した小さな木を作ります。通常は、元の木を `build()` してから `AuxiliaryTree` を作り、`vt.build(vertices)` に使いたい頂点番号を渡します。入力の順番を気にする必要はありません。

```cpp
#include "lib/tree/lca.hpp"
#include "lib/tree/auxiliary_tree.hpp"
#include <iostream>

int main() {
	LCA tree(5);
	tree.add_edge(0, 1);
	tree.add_edge(1, 2);
	tree.add_edge(1, 3);
	tree.add_edge(3, 4);
	tree.build(0);

	AuxiliaryTree vt(tree);
	std::vector<int> vertices{2, 4};
	vt.build(vertices);
	for (int i = 0; i < vt.size(); ++i) {
		const auto &node = vt[i];
		if (node.parent == -1) continue;
		std::cout << node.vertex << ' ' << vt[node.parent].vertex
		          << ' ' << node.length << '\n';
	}
}
```

出力は `4 1 2`、`2 1 1` の 2 行です。各行は「元の頂点番号、補助木での親の元の頂点番号、親までの元の辺数」です。頂点 2 と 4 に加え、LCA の頂点 1 が残ります。元の木の頂点 3 は辺の途中にまとめられ、根だった頂点 0 も補助木には入りません。

[LCA](https://qandori.github.io/cp-library/tree/lca.hpp.html) または [HLD](https://qandori.github.io/cp-library/tree/hld.hpp.html) を先に `build` してください。その根に基づく LCA を使い、補助木の根は指定頂点全体の LCA になります。以下では元の頂点数を $$n$$、重複を含む入力列長を $$k$$、補助木の頂点数を $$m$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `AuxiliaryTree(tree)` | 前処理済みの木を参照して作成。型は推論可能 | $$O(1)$$ |
| `build(vertices)` | 頂点列から構築し、`nodes()` と同じ配列への const 参照を返す | LCA: $$O(k)$$、HLD: $$O(k\log n)$$ |
| `build_sorted(vertices)` | 元の木の DFS 番号の小さい順に並んだ頂点列から構築。内部のソートを省く。通常は `build` を使う | LCA: $$O(k)$$、HLD: $$O(k\log n)$$ |
| `size()` | 補助木の頂点数 | $$O(1)$$ |
| `empty()` | 補助木が空か | $$O(1)$$ |
| `root()` | 根の圧縮 ID。空なら -1、そうでなければ `size() - 1` | $$O(1)$$ |
| `operator[](i)` | 圧縮 ID `i` の `Node` への const 参照 | $$O(1)$$ |
| `nodes()` | 全 `Node` の配列への const 参照 | $$O(1)$$ |
| `to_graph()` | 圧縮 ID を頂点番号とする無向木の隣接リストを `vector<vector<int>>` で返す | $$O(m)$$ |

| `Node` の要素 | 内容 |
| --- | --- |
| `vertex` | 元の木での頂点番号 |
| `parent` | 親の圧縮 ID。根は -1 |
| `length` | 親までの元の辺数。根は 0 |
| `selected` | 入力に含まれる頂点か。LCA として追加しただけの頂点は false |

入力は `std::vector<int>`・`std::array<int,k>` などの `std::span<const int>` に変換できる列です。入力を書き換えず、重複は一つにまとめます。空入力にも対応し、異なる指定頂点が $$d>0$$ 個なら補助木は最大 $$2d-1$$ 頂点です。

元の頂点番号と、補助木の配列を参照するための圧縮 ID は別物です。`vt[i].vertex` は元の頂点番号、`vt[i].parent` は補助木の ID です。親の元の頂点番号を取得するには、根でないことを確認して `vt[vt[i].parent].vertex` とします。

圧縮 ID は子から親の順です。根以外は必ず `i < vt[i].parent` なので、ID の昇順で木 DP、降順で根からの伝播ができます。元の頂点番号や DFS の行きがけ順とは異なります。

```cpp
std::vector<std::vector<int>> graph = vt.to_graph();
```

`graph` の頂点数は `vt.size()` で、`graph[i]` には圧縮 ID `i` の親・子の ID が入ります。元の頂点番号には `vt[i].vertex`、根には `vt.root()` を使います。辺の長さは隣接リストに含めず、子の `length` で取得します。空の補助木には空配列、1 頂点には空の隣接リスト 1 つを返します。返り値は独立した配列なので、書き換えや補助木の再構築の影響を互いに受けません。

`build_sorted` を使う場合の「DFS 番号」は、元の木で頂点を初めて訪れる順に 0, 1, 2, ... と付けた番号です。`tree.subtree(v)` はその順序での部分木の区間 `[l,r)` を返し、`.first`、つまり `l` が頂点 `v` の DFS 番号です。頂点番号 `v` でソートすることとは異なります。

例えば、頂点 2 の DFS 番号が 4、頂点 4 の DFS 番号が 3 なら、`build_sorted` に渡せる順番は `{4,2}` です。任意の列をこの順番に直すコードは次のとおりです。上の例の `vt.build(vertices)` と置き換えられます。

```cpp
std::sort(vertices.begin(), vertices.end(), [&](int u, int v) {
	return tree.subtree(u).first < tree.subtree(v).first;
});
vt.build_sorted(vertices);
```

通常の `build` は DFS 番号を整数キーとして基数ソートし、256件未満では比較ソートを使います。自分で上の比較ソートをすると $$O(k\log k)$$ 時間かかります。`build_sorted` が役立つのは、木を DFS 順に走査しながら頂点を集めた場合など、入力が最初からこの順番になっているときです。順番が分からなければ `build` を使ってください。

重み付き距離は、元の木の根からの重み和を `distance[v]` として `distance[vt[i].vertex] - distance[vt[vt[i].parent].vertex]` で得られます。`length` は重みにかかわらず辺数です。

元の木は補助木より長く生存させ、移動・破棄しないでください。元の木の前処理をやり直した後は補助木も再構築してください。返された配列・要素への参照は、次の構築で内容が変わり、再確保で無効になります。作業配列の容量を再利用し、各構築で元の木の全頂点を走査しません。

`Tree` に必要な操作は `lca(u,v)`、部分木の DFS 区間を返す `subtree(v)`、辺数による深さ `depth[v]` です。

元の頂点数を $$n$$、入力列長を $$k$$ とすると、LCA の前処理は時間・空間 $$O(n \log n)$$、補助木の構築は $$O(k)$$ です。HLD は前処理・空間 $$O(n)$$、構築 $$O(k \log n)$$ です。ソート済みの入力でも同じ計算量ですが、`build_sorted` はソートとその作業領域の確保を省けます。補助木の追加空間は最大の入力列長に対し $$O(k)$$、生成後の参照操作は $$O(1)$$ です。`to_graph()` は補助木の頂点数を $$m$$ として $$O(m)$$ 時間・追加空間です。
