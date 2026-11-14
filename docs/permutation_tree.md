---
title: Permutation Tree
documentation_of: //tree/permutation_tree.hpp
redirect_from:
- /data_structure/permutation_tree.hpp.html
library_category: sequence
---

```cpp
#include "lib/tree/permutation_tree.hpp"
#include <iostream>

PermutationTree tree(std::vector<int>{1, 0, 3, 2});
std::cout << tree.count_intervals() << '\n'; // 7
for (int v : tree.children(tree.root())) {
	std::cout << tree[v].l << ' ' << tree[v].r << '\n';
}
// 0 2
// 2 4
```

順列の共通区間分解木を作ります。入力は `0..n-1` を一度ずつ含む順列です。`std::vector<int>`・`std::array<int,n>` などの `std::span<const int>` に変換できる列を渡せます。入力を書き換えず、元の入力配列は構築後に破棄できます。長さは `INT_MAX/2` 以下としてください。

区間 `[l,r)` に現れる値が連続する、つまり `max - min = r - l - 1` となる区間を共通区間と呼びます。木の頂点は、他の共通区間と部分的に重ならない共通区間（strong interval）に対応します。葉は各要素、根は順列全体です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PermutationTree()` | 空の木を作る | $$O(1)$$ |
| `PermutationTree(p)` | 順列 `p` から構築 | $$O(n+b\log b)$$ |
| `build(p)` | 順列から再構築。空列にも対応 | $$O(n+b\log b)$$ |
| `size()` | 葉と内部頂点を合わせた頂点数 | $$O(1)$$ |
| `length()` | 入力順列の長さ | $$O(1)$$ |
| `empty()` | 入力順列が空か | $$O(1)$$ |
| `root()` | 根の頂点 ID。空なら -1 | $$O(1)$$ |
| `leaf(i)` | 入力位置 `i` に対応する葉の ID。`i` と等しい | $$O(1)$$ |
| `operator[](v)` | 頂点 `v` の `Node` への const 参照 | $$O(1)$$ |
| `nodes()` | 全頂点の `vector<Node>` への const 参照 | $$O(1)$$ |
| `children(v)` | 子の ID を位置区間の左端順に並べた `span<const int>` | $$O(1)$$ |
| `order()` | 根からの幅優先順の頂点 ID を返す `span<const int>` | $$O(1)$$ |
| `count_intervals()` | 長さ 1 を含む全共通区間の個数を `int64_t` で返す。空なら 0 | $$O(1)$$ |

| `Node` の要素 | 内容 |
| --- | --- |
| `parent` | 親の ID。根は -1 |
| `l` / `r` | 入力位置の半開区間 `[l,r)` |
| `lo` / `hi` | 値の半開区間 `[lo,hi)`。常に `r-l = hi-lo` |
| `type` | `PermutationTree::Type` による頂点の種類 |

| `Type` の値 | 内容 |
| --- | --- |
| `Type::Leaf` | 長さ 1 の葉 |
| `Type::Increasing` | 子の値区間が左から増加する頂点 |
| `Type::Decreasing` | 子の値区間が左から減少する頂点 |
| `Type::Prime` | それ以外の内部頂点 |

`Increasing`・`Decreasing` の頂点では、連続する任意の子の位置区間を合わせても共通区間になります。全共通区間は、木の頂点の区間と、このような子の連続区間の和集合で表せます。

頂点 ID は構築順なので、親子の ID の大小関係は保証しません。葉の ID は `0..n-1` です。`order()` では必ず親が子より先に現れるため、その逆順で木 DP を行えます。子は必ず左から右の順です。空でなければ頂点数は最大 `2n-1` です。木が深い場合も、構築と走査順の生成に再帰を使いません。

コピー・ムーブに対応します。返された参照や span は再構築・代入・移動・破棄で無効になる場合があります。再構築では出力配列の容量を再利用します。

長さを $$n$$、区間最小値を求める内部表のブロック数を $$b=\lceil n/32\rceil$$ とすると、構築時間・一時空間は $$O(n+b\log b)$$ です。構築後の保持空間は $$O(n)$$、表の参照操作は全て $$O(1)$$ です。空列の構築は $$O(1)$$ です。
