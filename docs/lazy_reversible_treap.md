---
title: "Lazy Reversible Treap（遅延伝搬・反転可能乱択平衡二分木）"
documentation_of: //data_structure/lazy_reversible_treap.hpp
library_category: data_structure/sequence
---

挿入・削除・区間更新・反転ができる数列です。区間内の値を左から右の順に `op` でまとめた結果（集約値）を取得できます。

```cpp
#include "lib/data_structure/lazy_reversible_treap.hpp"
#include "lib/math/mint.hpp"

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }

using Tree = LazyReversibleTreap<mint, op, e, F, mapping, composition, id, true>;
Tree tree(std::vector<mint>{1, 2, 3});
tree.insert(1, 10);
tree.apply(0, 3, {2, 1});
tree.reverse(1, 4);
auto sum = tree.prod(0, 3);
```

添字は 0 始まり、区間は $$[l,r)$$ です。引数なしで空列、`Tree(vector<S>, seed = 0)` で初期列を構築します。`Tree(seed)` は空列を作り、木の形を決める乱数の初期値（シード）を指定します。

`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たします。これは要素の順番を保てば、まとめる括弧の位置を変えても結果が同じになる条件（結合法則）です。 `e()` は空区間の値で、`op(e(),x) == x` と `op(x,e()) == x` を満たします。この値を単位元と呼びます。

長さ `n` の区間を `op` でまとめた結果が `x` のとき、`mapping(f,x,n)` は区間の各要素に更新 `f` を適用した結果を返します。`composition(f,g)` は `g` の後に `f` を適用する更新、`id()` は何もしない更新です。

`mapping(id(),x,n) == x`、`mapping(composition(f,g),x,n) == mapping(f,mapping(g,x,n),n)` が必要です。また、長さ `nx`・`ny` の区間の結果を `x`・`y` とすると、`mapping(f,op(x,y),nx+ny) == op(mapping(f,x,nx),mapping(f,y,ny))` を満たすようにします。

最後のテンプレート引数は、常に `op(a,b) == op(b,a)` となる場合だけ `true` にします。既定の `false` では逆順にまとめた結果も保持し、行列積など、順番で結果が変わる演算に対応します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Tree(seed = 0)` / `Tree(a,seed = 0)` | 空／配列から構築。`Tree` はテンプレート引数を指定した型 | 空: $$O(1)$$、配列: $$O(n)$$ |
| `insert(p,x)` | 位置 `p` の直前に挿入 | 期待 $$O(\log n)$$ |
| `erase(p)` | 要素を削除し、その値を返す | 期待 $$O(\log n)$$ |
| `get(p)` / `set(p,x)` | 1 点の取得／代入 | 期待 $$O(\log n)$$ |
| `apply(p,f)` / `apply(l,r,f)` | 1 点／区間に更新を適用 | 期待 $$O(\log n)$$ |
| `reverse(l,r)` | 区間を反転 | 期待 $$O(\log n)$$ |
| `prod(l,r)` / `all_prod()` | 区間／全体の集約 | prod: 期待 $$O(\log n)$$、all_prod: $$O(1)$$ |
| `rotate(l,m,r)` | $$[l,m)$$ と $$[m,r)$$ を入れ替える | 期待 $$O(\log n)$$ |
| `move(l,r,p)` | 区間を切り取り、削除後の位置 `p` の直前へ挿入 | 期待 $$O(\log n)$$ |
| `to_vector()` | 全要素を取得 | $$O(n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `assign(a)` / `clear()` | 配列で再構築／容量を残して空にする | assign: $$O(m+n)$$、clear: $$O(m)$$ |
| `reserve(n)` / `memory_usage()` | ノード領域を予約／確保済み領域のバイト数 | reserve: $$O(m)$$、memory_usage: $$O(1)$$ |
| `swap(other)` | 内容と乱数生成器の状態を交換 | $$O(1)$$ |

要素数と予約容量は $$2^{30}$$ 未満です。削除したノードは再利用します。`clear()` は確保容量を残します。

「期待」は、木の形を決める乱数について平均した時間です。

現在の要素数を $$n$$、最大要素数を $$m$$ とすると、構築は $$O(n)$$、`assign` は $$O(m+n)$$、挿入・削除・1 点操作・区間操作は期待 $$O(\log n)$$、`all_prod`・`size`・`empty`・`memory_usage` は $$O(1)$$、`to_vector` は $$O(n)$$、`reserve`・`clear` は $$O(m)$$ です。空間は $$O(m)$$（予約容量が大きい場合はその容量分）です。演算・更新・値のコピーを $$O(1)$$ とします。
