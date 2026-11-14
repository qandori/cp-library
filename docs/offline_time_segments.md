---
title: オフラインの時間区間処理
documentation_of: //components/offline_time_segments.hpp
library_category: components/data_structure
---

各操作が有効な半開区間 `[left,right)` を登録し、各時刻に有効な操作だけを適用した状態を調べます。途中の状態を保存する `snapshot()` と、保存した状態へ戻す `rollback()` を持つ型を渡します。全ての操作区間を先に登録してから処理するため、入力を先読みできる動的連結性などに使います。

```cpp
#include "lib/components/offline_time_segments.hpp"
#include "lib/data_structure/rollback_union_find.hpp"
#include <cassert>
#include <utility>
#include <vector>

int main() {
	std::vector<std::pair<int,int>> edges = { {0, 1}, {1, 2} };
	OfflineTimeSegments timeline(3, { {0, 2, 0}, {1, 3, 1} });
	RollbackUnionFind uf(3);
	timeline.run(uf, [&](RollbackUnionFind &state, int id) {
		state.merge(edges[id].first, edges[id].second);
	}, [&](int time, const RollbackUnionFind &state) {
		assert(state.same(0, 2) == (time == 1));
	});
	assert(uf.component_count() == 3);
}
```

| 操作・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Interval` | `int left, right, item`。`item` は利用者が決める操作 ID | $$O(1)$$ |
| `OfflineTimeSegments(times, intervals)` | 時刻数と有効区間から構築 | $$O(T+I\log T)$$ |
| `size()` / `entry_count()` | 時刻数／時間セグメント木に置いた操作 ID の総数 | $$O(1)$$ |
| `run(state,apply,visit)` | 有効な操作を適用し、各時刻に問い合わせる。最後は開始時の状態へ戻す | $$O(T+K)$$ 回の状態操作 |

`times` は0以上で、各区間は `0 <= left <= right <= times` としてください。空区間は無視します。同じ ID の複数登録も別の操作として適用します。時刻数0ではコールバックを呼びません。

`state.snapshot()` が現在位置を返し、`state.rollback(saved)` がそこへ戻すことが必要です。`apply(state,item)` は操作を適用し、`visit(time,const_state)` はその時刻の問い合わせを処理します。`visit` は0から `times-1` まで必ず昇順に1回ずつ呼びます。`visit` から snapshot が管理する状態を変更しないでください。

同じ時刻に有効な操作の適用順は保証しません。適用順によらず問い合わせの答えが一致する用途に使ってください。外部データを変更する場合、その復元も `state.rollback` で行います。コールバックでこの時間区間オブジェクトを変更しないでください。

時刻数を $$T$$、区間数を $$I$$、格納した操作 ID の総数を $$K=O(I\log T)$$ とします。構築は $$O(T+I\log T)$$ 時間、空間は $$O(T+K)$$ です。`run` は $$O(T+K)$$ 回の操作を行い、`apply`・snapshot・rollback・`visit` の実際のコストは別に加算します。追加スタックは $$O(\log T)$$ です。空の場合や長さ1の場合にも定数時間かかります。
