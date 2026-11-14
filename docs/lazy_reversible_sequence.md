---
title: "Lazy Reversible Sequence（ブロック化された遅延伝搬・反転可能列）"
documentation_of: //data_structure/lazy_reversible_sequence.hpp
library_category: data_structure/sequence
---

数列への挿入・削除、区間更新・反転、区間の計算結果の取得を扱います。ここで「集約」は、区間内の値を左から右の順に `op` でまとめることです。

```cpp
#include "lib/data_structure/lazy_reversible_sequence.hpp"
#include <cassert>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
long long mapping(long long f, long long x, int n) { return x + f * n; }
long long id() { return 0; }

int main() {
	using Sequence = LazyReversibleSequence<long long, op, e, long long, mapping, op, id, true>;
	Sequence sequence(std::vector<long long>{1, 2, 3});
	sequence.insert(1, 10);
	sequence.apply(0, 3, 2);
	sequence.reverse(1, 4);
	assert((sequence.to_vector() == std::vector<long long>{3, 3, 4, 12}));
	assert(sequence.prod(0, 3) == 10);
}
```

`LazyReversibleSequence<S,op,e,F,mapping,composition,id,commutative = false,BlockSize = 128>` として使います。`S` は要素と集約値を表す型、`F` は更新内容を表す型です。`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たします。これは要素の順番を保てば、まとめる括弧の位置を変えても結果が同じになる条件（結合法則）です。 `e()` は空区間の値で、`op(e(),x) == x` と `op(x,e()) == x` を満たします。この値を単位元と呼びます。

長さ `n` の区間を `op` でまとめた結果が `x` のとき、`mapping(f,x,n)` は区間の各要素に更新 `f` を適用した結果を返します。`composition(f,g)` は `g` の後に `f` を適用する更新、`id()` は何もしない更新です。

`mapping(id(),x,n) == x`、`mapping(composition(f,g),x,n) == mapping(f,mapping(g,x,n),n)` が必要です。また、長さ `nx`・`ny` の区間の結果を `x`・`y` とすると、`mapping(f,op(x,y),nx+ny) == op(mapping(f,x,nx),mapping(f,y,ny))` を満たすようにします。

`commutative = true` は常に `op(a,b) == op(b,a)` となる場合だけ指定します。既定では逆順にまとめた結果も持ち、行列積など、順番で結果が変わる演算を扱います。`S,F` はコピー・代入できる型とします。既定コンストラクタは不要です。更新中に演算や値の操作が例外を送出した場合、更新前の状態へ戻す保証はありません。

添字は0始まりで、区間は `[l,r)` です。要素数と予約する要素数は $$2^{30}$$ 未満とします。`BlockSize` は8以上65535以下の整数で、1枚の葉に保持する要素数の上限です。

| 操作 | 内容・返り値 |
| --- | --- |
| `Sequence()` / `Sequence(a)` | 空列／`const vector<S>&` から構築 |
| `size()` / `empty()` | 要素数を `int`／空かを `bool` で返す |
| `insert(p,x)` | `[0,size()]` の位置 `p` の直前へ要素を挿入 |
| `erase(p)` | `[0,size())` の要素を削除し、`S` で返す |
| `get(p)` / `set(p,x)` | 1要素を `S` で取得／代入 |
| `prod(l,r)` / `all_prod()` | 区間／全体の集約を `S` で返す。空区間は `e()` |
| `apply(p,f)` / `apply(l,r,f)` | 1要素／区間へ更新を適用 |
| `reverse(l,r)` | 区間を反転 |
| `rotate(l,m,r)` | 隣接区間 `[l,m)` と `[m,r)` を入れ替える |
| `move(l,r,p)` | 区間を切り取り、削除後の `[0,size()-(r-l)]` の位置 `p` の直前へ挿入 |
| `to_vector()` | 現在の列を独立した `vector<S>` として返す |
| `assign(a)` / `clear()` | 配列で再構築／空列へ戻す |
| `reserve(n)` | `n` 要素を想定して節点領域を予約。葉の要素領域は必要時に確保 |
| `memory_usage()` | 確保済みの節点・要素・作業領域のバイト数を `size_t` で返す |
| `swap(other)` | 内容を交換 |

引数なし・空配列からの構築、空区間への更新・反転・移動に対応します。`get`・`prod`・`all_prod` は `const` オブジェクトでも使用できます。コピーは独立して更新でき、移動元は空列として再利用できます。乱数シードは使いません。`clear()` は節点の確保容量を残し、葉の要素領域を解放します。`memory_usage()` は `S,F` 自身が別途確保する領域を含みません。

頻繁な1要素操作の用途では [Lazy Reversible Treap](lazy_reversible_treap.md) も選べます。こちらは区間境界の葉で最大 `BlockSize` 個の要素を処理し、完全に含まれる部分木はまとめた値と保留中の更新内容だけで処理します。演算・更新が重い型では、ブロックサイズを小さくする選択肢があります。

「償却」は、1 回だけ時間のかかる操作があっても、一連の操作の合計時間から 1 回あたりの計算量を評価するという意味です。

現在の要素数を $$n$$、過去の最大要素数と予約数の最大を $$m$$、ブロックサイズを $$B$$ とします。構築は $$O(n)$$、挿入・削除・1点操作・区間操作は償却 $$O(B+\log n)$$ 時間です。`all_prod`・`size`・`empty`・`memory_usage`・`swap` は $$O(1)$$、`to_vector` は $$O(n)$$、コピー・`assign`・`clear`・`reserve` は $$O(m+n)$$ 時間以内です。使用空間は $$O(m+B)$$ です。演算・更新・値と更新内容のコピーを $$O(1)$$ として数えます。
