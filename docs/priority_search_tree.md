---
title: Priority Search Tree（三辺矩形内の点の列挙）
documentation_of: //data_structure/priority_search_tree.hpp
library_category: data_structure/rectangle_query
---

二次元の点について、`left <= x < right` かつ `y < upper` を満たす点を列挙します。x 区間内で y が最小の点の検索、点の挿入・削除、y 座標の変更にも対応します。

x 座標は構築時に登録し、各点を入力順の 0 始まりの ID で指定します。同じ x 座標や同じ座標の点も、異なる ID として登録できます。構築後に新しい x 座標や ID を追加する場合は `build` または `build_coordinates` で作り直します。

```cpp
#include "lib/data_structure/priority_search_tree.hpp"
#include <iostream>

std::vector<std::pair<int, int>> points = { {1, 5}, {3, 2}, {3, 7}, {6, 1} };
PrioritySearchTree<int> tree(points);
auto ids = tree.report(1, 6, 6); // ID 0, 1（順序は未規定）
auto best = tree.min_y(1, 6);    // optional<int>(1)
tree.set(2, 0);                 // ID 2 の点を (3, 0) に変更
tree.erase(1);
tree.enumerate(0, 10, 3, [&](int id) {
	std::cout << id << ' ' << tree.x(id) << ' ' << *tree.get(id) << '\n';
});

PrioritySearchTree<int> active(std::vector<int>{2, 4, 6}); // x 座標だけ登録
active.insert(1, 8); // ID 1 の点 (4, 8) を挿入
```

登録した点数を $$n$$、列挙する点数を $$k$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PrioritySearchTree<X=int64_t,Y=X>()` | 登録点のない構造を作る | $$O(1)$$ |
| `PrioritySearchTree(points)` / `build(points)` | `vector<pair<X,Y>>` を登録し、全点を有効にする | $$O(n\log n)$$ |
| `PrioritySearchTree(coordinates)` / `build_coordinates(coordinates)` | `vector<X>` を登録し、全点を無効にする | $$O(n\log n)$$ |
| `point_count()` / `size()` / `empty()` | 登録点数 / 有効な点数 / 有効な点がないか | $$O(1)$$ |
| `x(id)` / `get(id)` / `contains(id)` | x 座標 / `optional<Y>` / 有効かどうか | $$O(1)$$ |
| `insert(id,y)` | 無効な ID に点を挿入し `true` を返す。既に有効なら変更せず `false` | $$O(\log n)$$ |
| `erase(id)` | 点を無効にする。削除できたかを返す | $$O(\log n)$$ |
| `set(id,y)` | y 座標を変更する。無効なら挿入する | $$O(\log n)$$ |
| `min_y(left,right) const` | `[left,right)` 内で y が最小の有効な点の ID。存在しなければ `nullopt` | $$O(\log n)$$ |
| `report(left,right,upper) const` | 条件を満たす有効な点の ID を `vector<int>` で返す | $$O(\log n+k)$$ |
| `enumerate(left,right,upper,visit) const` | 条件を満たす各 ID に `visit(id)` を呼ぶ | $$O(\log n+k)$$ |
| `clear()` | 登録した x 座標と ID を保ち、全点を無効にする | $$O(n)$$ |
| `swap(other)` | 内容を交換する | $$O(1)$$ |

`min_y` で y が等しい場合は最小の ID を返します。列挙の順序は未規定で、同じ ID は一度だけ列挙します。`left >= right` の問い合わせは空です。`enumerate` のコールバック内で対象の木を変更しないでください。

ID の引数は `0 <= id < point_count()` とします。`x` と `get` は読み取り専用の参照を返します。ただし `X=bool` の `x` は値を返します。`X`・`Y` にはコピーと `<` による大小比較が必要です。大小比較では `a < a` を `false` とし、`a < b` かつ `b < c` なら `a < c` とします。どちら向きの `<` も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。デフォルト構築や無限大の値は不要です。浮動小数点型では NaN を渡さないでください。登録点数は `INT_MAX` 以下とします。

保持空間は $$O(n)$$ です。`enumerate`・`min_y` の追加空間は $$O(\log n)$$、`report` は返り値を含め $$O(\log n+k)$$ です。比較・コピーとコールバックを定数時間とします。空の構造への問い合わせも利用できます。

参考: [McCreight, Priority Search Trees](https://epubs.siam.org/doi/10.1137/0214021)。
