---
title: Rollback Union Find
documentation_of: //data_structure/rollback_union_find.hpp
redirect_from:
- /data_structure/union_find/rollback_union_find.hpp.html
- /lightweight/data_structure/union_find/rollback_union_find.hpp.html
library_category: data_structure/union_find
---

頂点を併合し、後から追加順の逆に取り消せる Union Find です。辺の追加・削除を先読みして連結性を調べる処理や、分岐する状態を DFS で調べるときに使えます。

```cpp
#include "lib/data_structure/rollback_union_find.hpp"
#include <cassert>

int main() {
	RollbackUnionFind uf(4);
	assert(uf.size() == 4 && uf.vertex_count() == 4);
	uf.merge(0, 1);
	auto state = uf.snapshot();
	uf.merge(1, 2);
	assert(uf.same(0, 2) && uf.size(0) == 3);
	uf.rollback(state);
	assert(uf.same(0, 1) && !uf.same(0, 2));
	uf.merge(0, 1);
	uf.undo();
	assert(uf.same(0, 1));
	uf.undo();
	assert(uf.component_count() == 4);
	assert(uf.count() == 4);
}
```

以下では、頂点数を $$n$$、保持中の `merge` の回数を $$H$$、`rollback` で取り消す回数を $$k$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RollbackUnionFind(n=0)` | `n` 頂点を独立した成分として構築 | $$O(n)$$ |
| `reset(n)` | 頂点数を変更して、全頂点を独立に戻す。履歴も削除 | $$O(n+H)$$ |
| `reserve(updates)` | 保持する `merge` の回数を目安に履歴領域を確保。呼び出しは任意 | $$O(H)$$ |
| `size()` / `vertex_count()` | 全頂点数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `count()` / `component_count()` | 現在の連結成分数を `int` で返す同じ操作 | 時間・追加空間とも $$O(1)$$ |
| `leader(x)` / `same(x,y)` / `size(x)` | 代表元／連結判定／`x` の連結成分の頂点数 | $$O(\log n)$$ |
| `merge(x,y)` | 2頂点を併合。異なる成分を併合したときだけ true | 償却 $$O(\log n)$$ |
| `snapshot()` | 現在の履歴の長さを `size_t` で返す | $$O(1)$$ |
| `undo()` | 直前の `merge` 1回を取り消す。履歴が1つ以上必要 | $$O(1)$$ |
| `rollback(state=0)` | `snapshot()` で保存した状態へ戻す。省略時は全ての `merge` を取り消す | $$O(k)$$ |
| `groups()` | 現在の成分を `vector<vector<int>>` で返す | $$O(n\log n)$$ |

頂点番号は `[0,n)` です。`merge` は成分の小さい根を大きい根へ付け、経路圧縮はしません。`leader`・`same`・`size`・`groups` は読み取りだけで、履歴を増やしません。代表元の番号に特別な意味はありませんが、巻き戻すと元の代表元も復元します。

**既に連結な頂点への `merge` や `merge(x,x)` も、1回の操作として記録します。** その直後の `undo()` は、この何も変わらなかった操作だけを取り消します。成功した併合の回数を数えず、`merge` と `undo` を1対1に対応させられます。

`snapshot()` はオブジェクトをコピーせず、履歴の長さだけを返します。`rollback(state)` には、同じオブジェクトの現在の履歴に残っている状態を渡してください。巻き戻した先から別の併合を行うと、取り消した側の snapshot は再利用できません。数値が同じでも、別の分岐や別のオブジェクトの状態は復元できません。`reset` すると、以前の snapshot も使えなくなります。

独立した状態を同時に保持したい場合は `auto other = uf;` とコピーします。コピーには $$O(n+H)$$ 時間・空間が必要で、その後の併合・巻き戻しは独立です。このライブラリ自体は、過去の任意の状態に直接問い合わせる永続データ構造ではありません。

`groups()` は各成分の頂点を昇順で並べ、成分も最小の頂点が小さい順で返します。頂点数0では空の配列です。

履歴は1回の `merge` あたり整数2個です。併合された子の頂点と元の成分サイズを保存し、親のサイズは取り消し時に復元します。通常の32 bit `int` では履歴1回あたり8バイトです。`reset`・巻き戻しは確保済み領域を保持するため、実際の容量は過去の頂点数・履歴長・`reserve` の指定にも依存します。

利用例：[Persistent Unionfind の検証コード](../test/rollback_union_find.test.cpp)。クエリを先読みしてバージョンの木を作り、併合して下の状態を調べ、戻るときに `undo` します。このドライバーも再帰を使いません。Rollback Union Find 単体の利用では、クエリの先読みは不要です。

構築は $$O(n)$$ 時間、使用中のデータは $$O(n+H)$$ 空間です。`leader`・`same`・`size`・`merge` は $$O(\log n)$$ 時間で、`merge` の履歴確保だけは償却です。`undo` は $$O(1)$$、`rollback` は $$O(k)$$ 時間、`snapshot` は $$O(1)$$ 時間・追加空間です。`groups` は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間です。空の状態の構築や何もしない巻き戻しにも定数時間かかります。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
