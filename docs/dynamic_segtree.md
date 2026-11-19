---
title: Dynamic Segtree
documentation_of: //data_structure/dynamic_segtree.hpp
redirect_from:
- /data_structure/segment_tree/dynamic_segtree.hpp.html
- /lightweight/data_structure/segment_tree/dynamic_segtree.hpp.html
library_category: data_structure/segment_tree
---

大きな座標範囲に対して、一点更新と区間の計算結果の取得を行います。演算 `op` は要素の左から右の順に適用します。

```cpp
#include "lib/data_structure/dynamic_segtree.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

DynamicSegtree<long long, op, e> seg(1LL << 60);
seg.reserve(200000);
seg.set(123, 10);
seg.multiply(123, 5);
auto x = seg.get(123);
auto sum = seg.prod(0, 1000);
auto r = seg.max_right(0, [](long long s) { return s <= 10; });
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicSegtree(n = 0)` / `DynamicSegtree(l,r)` | $$[0,n)$$／$$[l,r)$$ を単位元で初期化 | $$O(1)$$ |
| `set(p,x)` | 座標 `p` に代入 | 償却 $$O(\min(B,k))$$ |
| `multiply(p,x)` | `op(現在値,x)` に更新 | 償却 $$O(\min(B,k))$$ |
| `get(p)` | 1 点の値。未更新なら `e()` | $$O(\min(B,k))$$ |
| `prod(l,r)` / `all_prod()` | $$[l,r)$$／全体の積 | prod: $$O(\min(B,k))$$、all_prod: $$O(1)$$ |
| `max_right(l,f)` / `min_left(r,f)` / `max_right<f>(l)` / `min_left<f>(r)` | 区間積が `f` を満たす最大の右端／最小の左端 | $$O(\min(B,k))$$ |
| `size()` / `left_bound()` / `right_bound()` | 座標範囲の長さ／左端／右端 | $$O(1)$$ |
| `node_count()` | 更新済みの異なる座標数 | $$O(1)$$ |
| `reserve(k)` / `clear()` | 領域予約／容量を残して全座標を単位元に戻す | $$O(k)$$ |
| `memory_usage()` | 確保済みノード領域のバイト数 | $$O(1)$$ |
| `swap(other)` | 内容と座標範囲を交換 | $$O(1)$$ |

`op` は結合律 `op(op(a,b),c) == op(a,op(b,c))` を満たすようにします。`e()` は空区間の値となる単位元で、`op(e(),x) == x` と `op(x,e()) == x` を満たすようにします。`op(a,b)` と `op(b,a)` が異なる演算にも対応します。未更新の座標の値は `e()` です。

構築範囲は `DynamicSegtree(n)` が $$[0,n)$$、`DynamicSegtree(l,r)` が $$[l,r)$$ です。座標型は第 4 テンプレート引数で指定でき、既定は `std::int64_t` です。

座標型は `bool` を除く 64 bit 以下の整数型です。構築範囲の両端はこの型で表し、左端は右端以下とします。一点の引数は `left_bound() <= p < right_bound()`、区間の引数は `left_bound() <= l <= r <= right_bound()` とし、境界探索の開始位置には両端も指定できます。空の構築範囲と空区間に対応し、空区間の積は `e()` です。`S` はコピー・移動・代入できる型を使い、演算途中の値もその型に収めてください。

`set(p,x)` は代入、`multiply(p,x)` は現在値を `op(現在値,x)` に更新します。`prod(l,r)` は $$[l,r)$$ の積、`all_prod()` は全体の積です。

`max_right(l,f)`・`min_left(r,f)` は区間積が `f` を満たす境界を返します。`f` は区間の計算結果を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返すようにします。`f(e()) == true` が必要です。区間を伸ばして一度 `false` になったら、その後は `true` に戻らない条件を使ってください。

`reserve(k)` は更新する異なる座標数を目安に領域を予約します。`node_count()` はその個数、`memory_usage()` は確保済みノード領域のバイト数です。更新する異なる座標数の上限は $$2^{26}-1$$ です。`clear()` は確保領域を残して初期状態に戻します。

座標型の bit 数を $$B$$、更新済み座標数を $$k$$ として、更新は償却 $$O(\min(B,k))$$、`get`・`prod`・境界探索は $$O(\min(B,k))$$ です。構築・`all_prod`・サイズ取得は $$O(1)$$、`reserve`・`clear` は $$O(k)$$ です。空間は $$O(k)$$（予約容量が大きい場合はその容量分）です。

`reserve` は容量が十分なら $$O(1)$$、コピーは $$O(k)$$ 時間・追加空間、移動構築は $$O(1)$$ 時間です。代入には移動先の旧要素を破棄する時間も加わります。$$k\le1$$ の操作は定数時間です。以上は値のコピー・演算・境界の判定を定数時間とした計算量です。
