---
title: "Fenwick Tree"
documentation_of: //data_structure/fenwick_tree.hpp
library_category: data_structure/range_query
---

```cpp
#include "lib/data_structure/fenwick_tree.hpp"

FenwickTree<long long> tree(std::vector<long long>{1, 2, 3, 4});
tree.add(1, 5);
auto sum = tree.sum(1, 3); // 10
auto prefix = tree.sum(3); // 11
```

`FenwickTree<T>(n)` は長さ `n` の零列、ベクトルを渡す構築はその列で初期化します。添字は 0 始まり、`add(p,x)` は一点加算、`sum(l,r)` は半開区間の和、`sum(r)` は `[0,r)` の和、`size()` は要素数です。空列・空区間にも対応します。コピーは独立します。

`T{}` が加算の 0 を表し、加算・減算ができる型を指定します。加算の順序や括弧の位置を変えても同じ結果になり、加えた値を引けば元の値に戻る演算を使います。整数、modint などを指定できます。整数型は内部を符号なし整数にし、符号付きオーバーフローを避けます。結果は型のビット幅に応じた剰余です。`bool` は指定できません。浮動小数点型は和の順序によって丸め誤差が変わります。

長さは非負の `int`、更新位置は `[0,n)`、区間は `0 <= l <= r <= n` が必要です。

既定では内部配列の 1024 要素ごとに 1 要素分の余白を入れ、メモリアクセスが特定箇所に集中することによる速度低下を抑えます。`FenwickTree<T, false>` で無効にできます。効果は区間長・CPU・コンパイル条件に依存します。

長さを $$n$$ として、構築・空間は $$O(n)$$、加算・和は $$O(\log n)$$、`size()` は $$O(1)$$ です。型の演算・コピーは $$O(1)$$ とします。
