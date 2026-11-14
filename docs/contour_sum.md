---
title: Tree Contour Sum
documentation_of: //tree/contour_sum.hpp
library_category: tree/query
---

無重みの木で頂点値を更新し、指定した頂点からの距離が区間に入る頂点の値の和を求めます。

```cpp
#include "lib/tree/contour_sum.hpp"

std::vector<std::pair<int, int>> edges{ {0, 1}, {1, 2}, {1, 3} };
TreeContourSum<> sums(4, edges, {1, 2, 3, 4});
assert(sums.prod(1, 1, 2) == 8);
sums.add(1, 10);
assert(sums.prod(0, 0, 3) == 20);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `TreeContourSum<T = std::int64_t>()` | 空の木を作る | $$O(1)$$ |
| `TreeContourSum(n,edges,values)` | 頂点数、無向辺の `span<const pair<int,int>>`、頂点番号順の `vector<T>` から構築 | $$O(n\log n)$$ |
| `build(n,edges,values)` | 木と値を置き換えて再構築 | $$O(n\log n)$$ |
| `size()` / `empty()` | 頂点数 `int`／空か `bool` | $$O(1)$$ |
| `get(v)` | 頂点 `v` の値 `const T&` | $$O(1)$$ |
| `add(v,x)` | 頂点 `v` の値に `x` を加算 | $$O(\log^2 n)$$ |
| `set(v,x)` | 頂点 `v` の値を `x` に変更 | $$O(\log^2 n)$$ |
| `prod(v,l,r)` | 距離 `d` が `l <= d < r` を満たす全頂点の値の和 `T` | $$O(\log^2 n)$$ |

頂点番号は `[0,n)` です。`edges` には `vector<pair<int,int>>` も渡せます。辺は向きを区別せず各 1 回渡し、非空なら `n-1` 本で連結な木にします。自己ループ・多重辺・重み付き辺は扱いません。空の木は `n=0`、空の辺と値で構築できます。`values.size()` は `n` と一致させ、頂点を指定する操作は空でない木に対して呼び出してください。

距離は辺の本数です。`l,r` の型は `std::int64_t` で、負の値も指定できます。該当する頂点がない場合や `l >= r` の場合は `T{}` を返します。構築後に入力の辺や値を変更・破棄しても影響しません。木の辺を変更する場合は再構築します。`get` の参照は再構築・代入・破棄で無効になります。

`T` はコピー・代入でき、`T{}` を 0 として加減算できる型にします。和は計算順序によらず同じ値となり、`(x + y) - y == x` のように加算を取り消せることが必要です。整数・modint などを指定でき、`bool` は指定できません。整数では更新後の各頂点値と取得する和が `T` に収まるようにしてください。

頂点数は `INT_MAX/2` 以下とします。頂点数を $$n$$ として、構築は $$O(n\log n)$$ 時間・空間、値の更新と距離区間の和は $$O(\log^2 n)$$ 時間です。値のコピーと加減算は定数時間とします。
