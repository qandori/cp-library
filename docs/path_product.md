---
title: Tree Path Product
documentation_of: //tree/path_product.hpp
library_category: tree/query
---

固定した木の頂点値を更新し、2 頂点を結ぶ経路上の値を `op` で順にまとめた結果（経路積）を求めます。`op(a,b)` と `op(b,a)` が異なる演算にも使えます。

```cpp
#include "lib/tree/path_product.hpp"
#include <cstdint>

std::int64_t op(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t e() { return 0; }

HLD tree(4);
tree.add_edge(0, 1);
tree.add_edge(1, 2);
tree.add_edge(1, 3);
tree.build();
TreePathProduct<std::int64_t, op, e> values(tree, {1, 2, 3, 4});
assert(values.prod(2, 3) == 9);
values.set(1, 10);
assert(values.prod(2, 3) == 17);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `TreePathProduct<S,op,e,reverse = nullptr>()` | 空の木を作る | $$O(1)$$ |
| `TreePathProduct(tree,values)` | 構築済みの `HLD` と、元の頂点番号順の `vector<S>` から構築 | $$O(n)$$ |
| `build(tree,values)` | 木と値を置き換えて再構築 | $$O(n)$$ |
| `size()` / `empty()` | 頂点数 `int`／空か `bool` | $$O(1)$$ |
| `get(v)` | 頂点 `v` の値 `const S&` | $$O(1)$$ |
| `set(v,value)` | 頂点 `v` の値を変更 | $$O(\log n)$$ |
| `prod(u,v)` | 頂点 `u` から `v` へ向かう順の経路積 `S`。両端を含む | $$O(\log n)$$ |
| `apply(u,v,value,mapping)` | `mapping(経路積,value)` の結果を `value` と同じ型で返す | $$O(\log n)$$ |

頂点番号は `[0,n)` です。`values.size()` は頂点数と一致させます。空の木も構築できますが、頂点を指定する操作は空でない木に対して呼び出してください。構築後に元の `HLD` を変更・破棄しても影響しません。木の辺を変更する場合は再構築します。`get` の参照は再構築・代入・破棄で無効になります。

`S` はコピー・代入可能な型です。`op(op(a,b),c) == op(a,op(b,c))` を満たし、`op(e(),x)`・`op(x,e())` がどちらも `x` になるように定義してください。演算は頂点列の左から右へ適用する順序で定義します。

`apply` の `mapping(segment,value)` は、経路積 `segment` が表す変換を `value` に適用する関数です。`mapping(e(),x) == x` および `mapping(op(a,b),x) == mapping(b,mapping(a,x))` を満たしてください。例えば、`S` が一次関数 `a*x+b` を表すなら、`mapping` はその一次関数の評価を行います。

第 4 テンプレート引数 `reverse` は省略できます。指定する場合は、演算結果 `S` から頂点列を逆順にした演算結果を返す関数とし、`reverse(op(a,b)) == op(reverse(b),reverse(a))`、`reverse(e()) == e()`、各頂点値 `v` について `reverse(v) == v` を満たしてください。順方向・逆方向の両方を持つ型などで指定できます。

頂点数を $$n$$ として、構築は $$O(n)$$ 時間・空間、頂点値更新・経路積・経路積を用いた変換は $$O(\log n)$$ 時間です。値のコピーと演算・`mapping` は定数時間とします。
