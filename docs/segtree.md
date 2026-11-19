---
title: セグメント木
documentation_of: //data_structure/segtree.hpp
library_category: data_structure/segment_tree
redirect_from:
- /data_structure/segment_tree/segtree.hpp.html
- /lightweight/data_structure/segment_tree/segtree.hpp.html
---

一点代入・一点への演算と、区間の要素を左から右の順に `op` でまとめた結果の取得を扱います。

`op` は結合律 `op(op(a,b),c) == op(a,op(b,c))` を満たすようにします。

`e()` は空区間の値となる単位元で、`op(e(),x) == x` と `op(x,e()) == x` を満たすようにします。`op(a,b) == op(b,a)` は不要なので、行列積や関数合成のように順番で結果が変わる演算にも使えます。

```cpp
#include "lib/data_structure/segtree.hpp"
#include <cassert>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
bool at_most_six(long long x) { return x <= 6; }

int main() {
	Segtree<long long, op, e> tree(std::vector<long long>{1, 2, 3});
	tree.set(1, 5);
	assert(tree.prod(0, 3) == 9);
	assert(tree.max_right<at_most_six>(0) == 2);
	assert(tree.min_left<at_most_six>(3) == 2);
	tree.multiply(1, 2);
	assert(tree.get(1) == 7);
}
```

`Segtree(n)` は全要素を `e()` で初期化します。添字は 0 始まり、`[l,r)` は `l` 以上 `r` 未満の添字を含む区間です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Segtree(n = 0)` / `Segtree(values)` | 長さ／値の配列から構築 | $$O(n)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `set(p,value)` / `get(p)` | 一点代入／取得 | $$O(\log n)$$ / $$O(1)$$ |
| `multiply(p,value)` | `a[p]` を `op(a[p],value)` に置き換える。返り値は `void` | $$O(\log n)$$ |
| `prod(l,r)` | 左から右の順の区間積を取得。空区間なら `e()` | $$O(\log n)$$ |
| `all_prod()` | 全体の積。空列なら `e()` | $$O(1)$$ |
| `max_right(l,pred)` / `max_right<pred>(l)` | `pred(prod(l,r)) == true` となる最大の `r` | $$O(\log n)$$ |
| `min_left(r,pred)` / `min_left<pred>(r)` | `pred(prod(l,r)) == true` となる最小の `l` | $$O(\log n)$$ |

`<pred>` には関数名やキャプチャのないラムダ式を指定できます。実行時の値をキャプチャするラムダは `max_right(l,pred)`・`min_left(r,pred)` に渡します。

`pred` は区間の計算結果を 1 つ受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返し、`pred(e()) == true` が必要です。`max_right` では右へ、`min_left` では左へ区間を伸ばして、一度 `false` になったら `true` に戻らない条件を使います。

例えば使用例の非負の数列では、和が 6 以下かどうかを判定しています。`[0,2)` の和は 6、次の要素まで含めた `[0,3)` の和は 9 なので、`max_right` は 2 を返します。`max_right(n,pred)` は `n`、`min_left(0,pred)` は 0 です。詳しい境界の定義は [遅延セグメント木の境界探索](lazy_segtree.md#境界を探す) と同じです。

長さは非負の `int`、一点引数は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。空列に対応し、コピーは独立します。更新以外の操作は `const` な木にも使えます。演算結果は指定した型で表現できるようにしてください。

長さを $$n$$ として、構築・空間は $$O(n)$$、一点更新・区間積・境界検索は $$O(\log n)$$、一点取得・全体の積・要素数の取得は $$O(1)$$ です。演算・コピーは $$O(1)$$ とします。
