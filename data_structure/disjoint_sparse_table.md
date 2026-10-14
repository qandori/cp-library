# Disjoint Sparse Table

`DisjointSparseTable<S, op, e>` は更新のない列を集約します。`op` は結合的であればよく、可換性や冪等性は不要です。`e` は省略でき、省略した場合は空区間を問い合わせられません。

```cpp
#include "lib/data_structure/disjoint_sparse_table.hpp"
int op(int a, int b) { return min(a, b); }
int e() { return numeric_limits<int>::max(); }
DisjointSparseTable<int, op, e> table(a);
auto value = table.prod(l, r);
```

`prod(l, r)` は半開区間 `[l, r)` の集約です。構築は `O(n log n)`、問い合わせは演算 1 回の `O(1)`、メモリは `O(n log n)` です。単一要素の区間はその要素を返し、空区間は `e()` を返します。要素は `table[i]`、全体は `table.all_prod()`、再構築は `table.build(a)` で扱えます。`size()` と `empty()` もあります。

`op` と `e` は関数またはキャプチャしない lambda を指定します。文字列や、既定のコンストラクタを持たない型にも対応しています。`O(1)` は `op` と要素のコピーが定数時間の場合です。

未使用の末尾を省き、整数などの自明な型は構築前のゼロ初期化を省きます。行 0 を原配列の保存領域として兼用し、単一要素の問い合わせも演算を呼びません。

`S` はコピー・代入可能な型を指定します。渡された配列はコピーして保持します。
