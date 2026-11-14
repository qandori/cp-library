---
title: 木上の Mo's Algorithm
documentation_of: //tree/tree_mo.hpp
redirect_from:
- /data_structure/range_query/tree_mo.hpp.html
- /lightweight/data_structure/range_query/tree_mo.hpp.html
- /data_structure/tree_mo.hpp.html
- /lightweight/data_structure/tree_mo.hpp.html
library_category: tree/query
---

木の経路に含まれる頂点・辺を追加／削除して、経路上の異なる値の個数や頻度などを一括計算します。木と値は処理中に変更しません。追加・削除する順番によらず答えが同じになるものを扱います。

```cpp
#include "lib/tree/tree_mo.hpp"
#include <vector>

int main() {
    std::vector<std::vector<int>> graph{ {1, 2}, {0, 3, 4}, {0}, {1}, {1} };
    std::vector<int> value{1, 2, 3, 4, 5}, answers(3);
    TreeMo mo(graph);
    mo.add_query(3, 2);
    mo.add_query(3, 4);
    mo.add_query(2, 2);
    int sum = 0;
    mo.run([&](int v) { sum += value[v]; },
        [&](int v) { sum -= value[v]; },
        [&](int id) { answers[id] = sum; });
    // answers = {10, 11, 3}
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `TreeMo(graph,root=0,edge_values=false)` | `std::vector<std::vector<int>>` の無向木から構築 |
| `size()` / `query_count()` | 頂点数・問い合わせ数を `int` で返す |
| `parent(v)` | 指定した根に対する親を `int` で返す。根は `-1` |
| `add_query(u,v)` | `u` と `v` を結ぶ経路を登録し、0 始まりの問い合わせ ID を `int` で返す |
| `order(block_size=0)` | 処理順の問い合わせ ID を `std::vector<int>` で返す |
| `run(add,erase,answer,block_size=0)` | 渡した関数を呼んで処理。返り値は `void` |

頂点番号は 0 始まりです。入力は自己ループ・多重辺のない連結な無向木とし、各辺を両方向の隣接リストに入れます。頂点数は `INT_MAX/2` 以下、空でない場合の `root`・`u`・`v` は `[0,size())` にします。空の木も構築できますが、経路は登録できません。空の木では `root` を使いません。問い合わせ数は `INT_MAX` 以下です。

`edge_values=false` では両端を含む経路上の頂点を扱います。`u==v` の経路は 1 頂点です。`edge_values=true` では経路上の辺を扱い、渡した関数にはその辺の子側の頂点番号を渡します。`u==v` の経路は空です。

辺値の場合は、上の例の `mo` と同じ木に対して次のように使えます。

```cpp
TreeMo edges(graph, 0, true);
std::vector<int> weight{0, 10, 20, 30, 40};
// weight[v] は parent(v) と v を結ぶ辺の値。根の値は使いません。
edges.add_query(3, 2);
int edge_sum = 0, answer = 0;
edges.run([&](int v) { edge_sum += weight[v]; },
    [&](int v) { edge_sum -= weight[v]; },
    [&](int) { answer = edge_sum; });
// answer = 60
```

`run` の前には、追加済みの頂点・辺がない状態にしてください。`add(v)` は頂点または辺を追加し、`erase(v)` は削除します。`answer(id)` の時点で、追加済みの頂点・辺がその経路と一致します。LCA の追加・除外を呼び出し側で行う必要はありません。

`add`・`erase` の呼び出し順は経路上の順番とは限りません。経路の順序で答えが変わる演算には [Path Product](path_product.md) を使ってください。処理中に問い合わせを追加しないでください。再実行は可能ですが、追加済みの頂点・辺がない状態へ戻す必要があります。実行終了時に全要素を削除することは保証しません。

`block_size` は非負とし、0 では [通常の Mo's Algorithm](mo_order.md) と同様に自動選択します。問い合わせがなければ渡した関数を呼びません。

頂点数を $$N$$、問い合わせ数を $$Q$$ として、構築は $$O(N\log N)$$ 時間・空間、`size()`・`query_count()`・`parent(v)` は $$O(1)$$、経路の登録は償却 $$O(1)$$、順序生成は $$O(Q)$$ です。各関数が定数時間なら、自動選択時の処理は $$O(N\sqrt Q+Q)$$ 時間、追加の作業空間は $$O(N+Q)$$ です。幅 $$B$$ を指定する場合の処理時間は $$O(QB+N^2/B+Q)$$ です。
