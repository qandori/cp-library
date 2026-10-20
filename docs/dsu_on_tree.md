---
title: DSU on Tree
documentation_of: //tree/dsu_on_tree.hpp
---

各頂点を根とする部分木について、色の種類数・最頻値などをまとめて求めます。最大の子の集計を残し、それ以外の子だけを追加し直します。Union-Find を使うライブラリではありません。

次は各部分木に現れる色の種類数を求める例です。`color[v]` は 0 以上 `colors` 未満とします。

```cpp
#include "lib/tree/dsu_on_tree.hpp"

DSUOnTree tree(5);
tree.add_edge(0, 1);
tree.add_edge(0, 2);
tree.add_edge(1, 3);
tree.add_edge(1, 4);
tree.build(0);

std::vector<int> color{0, 1, 0, 2, 1};
int colors = 3, distinct = 0;
std::vector<int> count(colors), answer(tree.size());
tree.run(
	[&](int v) { distinct += count[color[v]]++ == 0; },
	[&](int v) { distinct -= --count[color[v]] == 0; },
	[&](int v) { answer[v] = distinct; }
);
// answer は 3, 2, 1, 1, 1。distinct と count は空の状態に戻る。
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DSUOnTree(n = 0)` | 頂点数を指定。頂点番号は 0 始まり | $$O(n)$$ |
| `size()` / `empty()` / `edge_count()` | 頂点数／空判定／追加済み辺数 | $$O(1)$$ |
| `add_edge(u,v)` | 無向辺を追加し、0 始まりの辺 ID を返す | 償却 $$O(1)$$ |
| `edge(id)` | 追加した辺の両端の組への const 参照 | $$O(1)$$ |
| `build(root = 0)` | 根を指定して前処理・再構築 | $$O(n)$$ |
| `run(add,remove,answer,keep = false)` | 頂点ごとの追加・削除で部分木を集計 | $$O(n\log n)$$ |
| `run_clear(add,answer,clear,keep = false)` | 個別削除の代わりに `clear()` で状態全体を空にする | $$O(n\log n)$$ ＋回答・リセットの合計時間 |
| `root()` / `parent(v)` | 根／親。空の木の根と、根の親は -1 | $$O(1)$$ |
| `heavy_child(v)` | 部分木が最大の子。葉なら -1。同数のときの選択は未規定 | $$O(1)$$ |
| `subtree_size(v)` | 自身を含む部分木の頂点数 | $$O(1)$$ |
| `subtree(v)` | 部分木に対応する `order()` の半開区間 `[l,r)` | $$O(1)$$ |
| `vertex(i)` / `order()` | DFS 順の `i` 番目の頂点／全頂点の `span<const int>` | $$O(1)$$ |
| `vertices(v)` | 部分木の頂点列の `span<const int>` | $$O(1)$$ |

入力は連結な無向木で、辺をちょうど `n-1` 本追加してから `build()` を呼びます。空の木も扱え、その場合の実行ではコールバックを呼びません。根を変える場合は `build(root)` を呼び直します。辺を追加すると前処理は無効になります。返された span は、再構築・オブジェクトの破棄まで有効です。

`add(v)` は頂点を状態に追加し、`remove(v)` は追加済みの頂点を状態から除きます。`answer(v)` が呼ばれる瞬間には、状態に含まれる頂点が `v` の部分木と完全に一致します。各頂点について一度ずつ呼びますが、頂点番号順ではありません。答えは例のように頂点番号で保存してください。追加・削除の順序は未規定です。コールバックから木の再構築や状態を変える再入実行はしないでください。

実行開始時は状態を空にしてください。既定では実行終了時も空に戻るので、異なる集計を同じ木で繰り返せます。`keep=true` なら最後の根の集計を消さず、全頂点を追加した状態で終了します。再実行するときは利用者がその状態を空に戻してください。

個別に値を取り消しにくい集計には `run_clear` を使えます。`clear()` は状態全体を空にする処理です。例えば、触れた色だけをリストに保存しておき、その色の個数と最頻値をリセットします。毎回長さ `n` の配列全体を初期化すると、二乗時間になる場合があります。

```cpp
std::vector<int> count2(colors), touched, mode(tree.size());
int maximum = 0;
tree.run_clear(
	[&](int v) {
		int c = color[v];
		if (count2[c] == 0) touched.push_back(c);
		maximum = std::max(maximum, ++count2[c]);
	},
	[&](int v) { mode[v] = maximum; },
	[&]() {
		for (int c : touched) count2[c] = 0;
		touched.clear();
		maximum = 0;
	}
);
```

各走査は再帰を使わず、一本道の木でもコールスタックを消費しません。重い子を先頭にした DFS 順を逆から走査し、軽い子の追加を連続した区間で行います。頂点ごとのイベント列や仮想関数は使いません。

前処理は時間・空間とも $$O(n)$$。`add`・`remove` がそれぞれ $$O(1)$$ なら `run` は $$O(n\log n)$$、`answer` の呼び出しはちょうど $$n$$ 回です。`run_clear` の追加回数も $$O(n\log n)$$、リセット回数は高々 $$n$$ 回で、実行時間には `answer` と `clear` の合計時間を加えます。各 `clear` が消す頂点数以下の時間なら全体で $$O(n\log n)$$ です。走査自体の追加領域は $$O(1)$$、ライブラリ全体の空間は $$O(n)$$ です。
