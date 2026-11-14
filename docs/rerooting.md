---
title: 全方位木 DP
documentation_of: //tree/rerooting.hpp
redirect_from:
- /graph/rerooting.hpp.html
library_category: tree
---

全頂点をそれぞれ根にした木 DP を計算します。[全方位木 DP の抽象化](https://trap.jp/post/1702/)と同じように、辺の寄与 `E`、頂点の値 `V`、4 つの関数をテンプレート引数に指定します。

## RerootingDP

各頂点から同じ連結成分の全頂点までの距離和を求める例です。

```cpp
#include "lib/tree/rerooting.hpp"
#include <cassert>
#include <utility>
#include <vector>

using Data = std::pair<long long, int>;
std::vector<long long> weight{2, 5};

Data merge(Data a, Data b) {
	return {a.first + b.first, a.second + b.second};
}
Data e() { return {0, 0}; }
Data put_edge(Data x, int idx) {
	return {x.first + weight[idx] * x.second, x.second};
}
Data put_vertex(Data x, int) {
	return {x.first, x.second + 1};
}

int main() {
	RerootingDP<Data, Data, merge, e, put_edge, put_vertex> tree(3);
	tree.add_edge(0, 1, 0, 0);
	tree.add_edge(1, 2, 1, 1);
	auto subtree = tree.build(0);
	auto answer = tree.reroot();
	assert((subtree[1] == Data{5, 2}));
	assert(answer[0].first == 9);
	assert(answer[1].first == 7);
	assert(answer[2].first == 12);
	assert((tree.subtree(0, 1) == Data{5, 2}));
	assert((tree.subtree(2, 1) == Data{2, 2}));
	assert(tree.subtree(1, 1) == answer[1]);
}
```

`Data::first` が距離和、`second` が頂点数です。辺重みや頂点の値は関数から参照します。

| 関数・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RerootingDP<E,V,merge,e,put_edge,put_vertex>(n = 0)` | `n` 頂点の空の森を作る | $$O(n)$$ |
| `add_edge(u,v,idx,xdi)` | 無向辺を追加し、追加順の 0 始まりの辺 ID を `int` で返す。`idx` は `u` から見た `v` 側の寄与、`xdi` は `v` から見た `u` 側の寄与に使う番号 | 償却 $$O(1)$$ |
| `build(root = 0)` | `root` を根にして、各頂点の部分木の DP 値を `vector<V>` で返す | $$O(n)$$ |
| `reroot()` | 各頂点を根にした連結成分全体の DP 値を `vector<V>` で返す。先に `build` が必要 | $$O(n)$$ |
| `subtree(u,v)` | 頂点 `u` を根としたときの頂点 `v` の部分木の DP 値を `const V&` で返す。先に `build`・`reroot` が必要 | $$O(\log n)$$ |
| `size()` / `empty()` / `edge_count()` | 頂点数／頂点数が 0 か／辺数 | $$O(1)$$ |
| `Edge` / `Vertex` | それぞれ `E` / `V` の型別名 | $$O(1)$$ |

| 関数 | 内容・返り値 |
| --- | --- |
| `merge(E a,E b) -> E` | 隣接する成分の情報をまとめる。下記の結合順序の条件が必要 |
| `e() -> E` | 情報がない場合の値。`merge(e(),x)`・`merge(x,e())` がどちらも `x` になる値を返す |
| `put_edge(V x,int idx) -> E` | 隣接する頂点側の DP 値 `x` に、番号 `idx` の辺を付加する |
| `put_vertex(E x,int v) -> V` | 頂点 `v` に集まった寄与に `v` 自身を付加する |

`merge(merge(a,b),c) == merge(a,merge(b,c))` を満たすように定義します。値の並び順を保ったまま、括弧の付け方を変えても同じ値になる、という条件です。

`add_edge(u,v,idx,xdi)` では、`u` の値を求めるときに `put_edge(v 側の値,idx)`、`v` の値を求めるときに `put_edge(u 側の値,xdi)` を使います。番号は任意の `int` で、重複しても構いません。両方向に同じ変換を使う場合は `idx` と `xdi` を同じにします。返される辺 ID とこれらの番号は独立です。

`subtree(u,v)` は頂点 `v` 自身を含み、`v` とその親を結ぶ辺の変換を施す前の値を返します。`u == v` なら `reroot()[v]` と同じ連結成分全体の値です。`u` と `v` は同じ連結成分の頂点とし、どちらも $$0\le u,v<n$$ を満たす必要があります。返された参照は、辺の追加・`build`・`reroot` を再度呼ぶまで利用できます。空の森には問い合わせできません。

同じ頂点の隣接辺は `add_edge` の呼び出し順に結合します。引数を交換すると結果が変わる演算でも使え、結合を取り消す逆演算は不要です。親への辺を除く場合も残った辺の順序を保ちます。各関数は引数と入力データだけで返り値が決まるようにし、`build` から `reroot` までの間は参照する入力データを変更しないでください。

入力は 0 始まりの木・森とし、自己ループ・多重辺・閉路を含めないでください。頂点番号は $$0\le v<n$$ です。森の `build` は `root` の連結成分を先に処理し、残りの連結成分では頂点番号が最小の頂点を根にします。孤立点の値は `put_vertex(e(),v)`、空の森の返り値は空配列です。空でない場合は $$0\le\texttt{root}<n$$ とします。走査に再帰は使いません。

頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下とします。`E` と `V` はコピー・代入可能としてください。`V` が `E` と異なる場合は `V` のデフォルト構築も必要です。辺の追加や入力データの変更後は `build` から呼び直します。`build`・`reroot` は繰り返し呼べます。返り値は独立した配列なので、再構築後も利用できます。

頂点数を $$n$$ とし、各関数と値のコピーが $$O(1)$$ のとき、`build`・`reroot` は $$O(n)$$ 時間・空間、`subtree` は最悪 $$O(\log n)$$ 時間です。

## Rerooting

実行時のラムダを渡す場合や、辺の両方向の寄与も取得する場合は `Rerooting<T,E>` を使います。

全頂点をそれぞれ根にした木 DP を計算します。辺の両方向の寄与も取得できます。次は、各頂点から全頂点までの距離の総和を求める例です。

```cpp
#include "lib/tree/rerooting.hpp"

using Data = std::pair<long long, int>;
Rerooting<Data> tree(3);
std::vector<long long> weight{2, 5};
tree.add_edge(0, 1);
tree.add_edge(1, 2);

const auto &answer = tree.build(Data{0, 0},
	[](Data a, Data b) {
		return Data{a.first + b.first, a.second + b.second};
	},
	[](Data x, int) {
		return Data{x.first, x.second + 1};
	},
	[&](Data x, int, int, int id) {
		return Data{x.first + weight[id] * x.second, x.second};
	});

long long sum = answer[1].first; // 7
auto from_zero = tree.edge_dp(0); // {2, 1}
```

`T` は頂点の DP 値、`E` は辺を通した寄与の型です。`E` を省略すると `T` と同じ型になります。`Data::first` が距離和、`second` が頂点数です。辺重みや頂点の値は、渡した関数から参照します。

| 関数・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Rerooting<T,E = T>(n = 0)` | `n` 頂点の空の森を作る | $$O(n)$$ |
| `add_edge(u,v)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `build(unit,merge,add_vertex,lift,root = 0)` | 全方位木 DP を計算し、全頂点の答えを `const vector<T>&` で返す | $$O(n)$$ |
| `tree[v]` | 頂点 `v` を根とした答えを `const T&` で返す | $$O(1)$$ |
| `values()` | 全頂点の答えを `const vector<T>&` で返す | $$O(1)$$ |
| `edge_dp(id,reverse = false)` | 辺を通した寄与を `const E&` で返す。`add_edge(u,v)` に対して `false` は `u → v`、`true` は `v → u` | $$O(1)$$ |
| `edge(id)` | 入力した辺の両端 `(u,v)` を `const pair<int,int>&` で返す | $$O(1)$$ |
| `size()` / `empty()` / `edge_count()` | 頂点数／頂点数が 0 か／辺数 | $$O(1)$$ |
| `Vertex` / `Edge` | それぞれ `T` / `E` の型別名 | $$O(1)$$ |

| 渡す関数 | 内容・返り値 |
| --- | --- |
| `merge(const E& a,const E& b) -> E` | 隣接する成分の情報をまとめる。`merge(merge(a,b),c) == merge(a,merge(b,c))` が必要。`merge(unit,x)`・`merge(x,unit)` はどちらも `x` にする。引数交換で同じ結果になる必要や、結合を取り消す演算は不要 |
| `add_vertex(const E& x,int v) -> T` | 頂点 `v` に集まった寄与に `v` 自身を加える |
| `lift(const T& x,int from,int to,int id) -> E` | 頂点 `from` を根とした値を、辺 `id` を通して `to` 側の寄与に変換する |

`edge_dp(id)` は、その辺を削除したときの送信元側の成分に対し、送信元の頂点を加え、最後に辺を通す変換を施した値です。受信先の頂点は含みません。例の `from_zero` は頂点 0 だけを含み、頂点 1 から測った距離が 2、頂点数が 1 です。

同じ頂点の隣接辺は `add_edge` の呼び出し順に結合します。親への辺を除くときも、残った辺の順序を変えません。各関数は引数と入力データだけで返り値が決まるようにします。`root` は内部の走査を始める頂点で、答えには影響しません。再帰は使いません。

入力は 0 始まりの木・森とし、自己ループ・多重辺・閉路を含めないでください。森では各頂点の属する連結成分について計算します。孤立点の答えは `add_vertex(unit,v)`、空の森の答えは空配列です。空でない場合は $$0\le\texttt{root}<n$$ とします。

頂点数は `INT_MAX` 未満、辺数は `INT_MAX/2` 以下とします。

値型はコピー・代入可能としてください。`T` と `E` が異なる場合は `T` のデフォルト構築も必要です。戻り値をコピーしたくない場合は `const auto&` で受け取ります。`tree[v]`・`values()`・`edge_dp` は `build` 後に使います。辺の追加や再構築後は、以前取得した参照を使わないでください。入力データや関数を変えて `build` を呼び直すこともできます。

頂点数を $$n$$ とし、各関数・値のコピーを $$O(1)$$ とすると、構築・`build` は $$O(n)$$ 時間・空間です。`add_edge` は償却 $$O(1)$$、そのほかの取得は $$O(1)$$ です。
