---
title: Segtree Beats
documentation_of: //data_structure/segtree_beats.hpp
redirect_from:
- /data_structure/advanced/generic_segtree_beats.hpp.html
- /data_structure/generic_segtree_beats.hpp.html
- /data_structure/segment_tree/generic_segtree_beats.hpp.html
- /docs/generic_segtree_beats.html
- /data_structure/segment_tree/segtree_beats.hpp.html
- /lightweight/data_structure/segment_tree/segtree_beats.hpp.html
library_category: data_structure/segment_tree
---

区間の要素を `op` でまとめた結果と更新方法を定義して使う汎用の Segtree Beats です。区間除算、bitwise AND/OR など、通常の遅延セグメント木では一度に処理できない更新を扱います。[chmin・chmax・加算・代入の専用版](https://qandori.github.io/cp-library/data_structure/range_chmin_chmax_add_sum.hpp.html) もそのまま使えます。

以下は、区間内の各要素を指定した上限以下にする更新（chmin）と、区間和を定義する例です。

```cpp
#include "lib/data_structure/segtree_beats.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <vector>

struct S {
	long long sum, hi;
	std::optional<long long> second;
	int count;
};
using F = long long;
S e() { return {0, 0, std::nullopt, 0}; }
S op(S a, S b) {
	if (!a.count) return b;
	if (!b.count) return a;
	if (a.hi < b.hi) std::swap(a, b);
	a.sum += b.sum;
	if (a.hi == b.hi) {
		a.count += b.count;
		if (b.second && (!a.second || *a.second < *b.second)) a.second = b.second;
	} else if (!a.second || *a.second < b.hi) {
		a.second = b.hi;
	}
	return a;
}
F id() { return std::numeric_limits<F>::max(); }
F composition(F f, F g) { return std::min(f, g); }
bool mapping(F f, S &a) {
	if (!a.count || a.hi <= f) return true;
	if (a.second && f <= *a.second) return false;
	a.sum -= (a.hi - f) * a.count;
	a.hi = f;
	return true;
}
using Tree = SegtreeBeats<S, op, e, F, mapping, composition, id>;

int main() {
	Tree seg(std::vector<S>{ {2, 2, std::nullopt, 1}, {5, 5, std::nullopt, 1}, {6, 6, std::nullopt, 1} });
	seg.apply(0, 3, 4);
	assert(seg.prod(0, 3).sum == 10);
	seg.apply(1, 3, 1);
	assert(seg.all_prod().sum == 4);
}
```

`second` は最大値より小さい値のうち最大のものを持ち、そのような値がなければ `nullopt` です。`count` は最大値の個数です。和とその更新の中間演算は `long long` に収まる範囲で使います。

| テンプレート引数・関数 | 定義 |
| --- | --- |
| `S` | 区間積の型 |
| `op(a,b)` | 左の区間 `a` と右の区間 `b` を結合。括弧の位置を変えても同じ結果になることが必要。要素の順序で結果が変わってもよい |
| `e()` | `op` の単位元。空区間の値 |
| `F` | 更新と、未伝播の更新の合成を表す型 |
| `mapping(f,s)` | `S &s` に更新 `f` を適用し、成功なら true、子へ降りる必要があれば false |
| `composition(f,g)` | `g` の後に `f` を適用する更新を返す |
| `id()` | 何もしない更新 |

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SegtreeBeats()` | 空で作成 | $$O(1)$$ |
| `SegtreeBeats(n,value = e())` | 長さ `n`、各葉の値を `value` にして構築 | $$O(n)$$ |
| `SegtreeBeats(a)` | `vector<S>` から構築 | $$O(n)$$ |
| `build(a)` | `vector<S>` で構築し直す | $$O(n)$$ |
| `apply(l,r,f)` | 半開区間 `[l,r)` に更新 `f` を適用 | $$O(\log n+E_i)$$ |
| `apply(f)` | 全体に更新 `f` を適用。空なら何もしない | $$O(1+E_i)$$ |
| `apply(p,f)` | 1 点に更新 `f` を適用 | $$O(\log n+E_i)$$ |
| `set(p,value)` | 位置 `p` の値を `value` に置き換える | $$O(\log n+E_i)$$ |
| `get(p)` | 1 点の値を `S` で返す | $$O(\log n+E_i)$$ |
| `prod(l,r)` | `[l,r)` の区間積。空なら `e()` | $$O(\log n+E_i)$$ |
| `all_prod()` | 全体の積。空なら `e()` | $$O(1)$$ |
| `max_right(l,predicate)` / `max_right<predicate>(l)` | `predicate(prod(l,r))` が true である最大の `r` | $$O(\log n+E_i)$$ |
| `min_left(r,predicate)` / `min_left<predicate>(r)` | `predicate(prod(l,r))` が true である最小の `l` | $$O(\log n+E_i)$$ |
| `values()` | 遅延を伝播し、全葉の値を `vector<S>` で返す | $$O(n+E_i)$$ |
| `size()` / `empty()` | 要素数／空か | $$O(1)$$ |

`S`・`F` はコピーと代入ができる型にします。デフォルト構築や `F` 同士の等値比較は不要です。`n` での構築は各葉に `e()` を置きます。上の例では、各葉の値と最大値の個数 `count = 1` を初期値として指定してください。

`mapping` が成功した場合、実際に全要素を更新した区間積と一致させます。単位更新は値を変えず、空区間と 1 要素の区間では必ず成功させてください。失敗時は `s` を変更しても構いません。合成した更新を子へ伝え、子の値から再計算します。遅延伝播中に子で失敗した場合も、さらに降りて処理します。

`op(op(a,b),c) == op(a,op(b,c))`（結合律）、`op(e(),x) == x` と `op(x,e()) == x`（単位元の条件）が必要です。`composition` は、区間内の各要素に `g`・`f` をこの順に適用する更新を表すようにします。計算途中のオーバーフローにも注意してください。

`predicate` は、区間積を受け取り、条件を満たせば `true`、満たさなければ `false` を返す関数です。同じ引数には同じ結果を返し、`predicate(e()) == true` が必要です。`max_right` では右へ、`min_left` では左へ区間を延ばして、一度 `false` になったら `true` に戻らない条件を使います。`all_prod`・`size`・`empty` 以外は、取得でも内部の遅延状態を変更します。

関数・型のコピーが定数時間であるとします。構築・空間は $$O(n)$$、`all_prod`・`size`・`empty` は $$O(1)$$ です。区間操作・1 点操作・境界探索は、通常の $$O(\log n)$$ に、`mapping` が `false` を返したことによって追加で訪問するノード数が加わります。表の $$E_i$$ はその操作での追加訪問数です。全 $$q$$ 操作での追加訪問数を $$E$$ とすると合計 $$O(n+q\log n+E)$$ です。`values()` は $$O(n)$$ に追加訪問数を加えた時間です。$$E$$ の上限は、利用する更新ごとに別途求める必要があります。
