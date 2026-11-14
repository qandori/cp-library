---
title: Trie
documentation_of: //string/trie.hpp
library_category: string
---

文字列を先頭から1文字ずつたどる木で、共通する先頭部分を共有して文字列の重複付き集合を管理します。接頭辞ごとの個数、辞書順の順位・選択、LCP（先頭から一致する最長の長さ）の集計に加え、各頂点に任意のデータを持たせて木 DP に使えます。

```cpp
#include "lib/string/trie.hpp"

Trie<> trie;
int apple = trie.insert("apple", 2);
trie.insert("app");
trie.insert("bat");
assert(trie.size() == 4 && trie.distinct_size() == 3);
assert(trie.count("apple") == 2);
assert(trie.prefix_count("ap") == 3);
assert(trie.kth(1).value() == "apple"); // 0 始まり。重複も数える
assert(trie.order_of_key("bat") == 3);
assert(trie.lcp("apply") == 4);
assert(trie.lcp_sum("apply") == 11);   // 4+4+3+0
assert(trie.restore(apple) == "apple");
trie.erase("apple");                 // 1 個削除
```

`Trie<Alphabet,First,Data>` の既定値は英小文字 26 文字と追加データなしです。`Trie<2,'A'>` は `A,B`、`Trie<10,'0'>` は数字、`Trie<256,0>` は全 256 バイトに対応します。文字は `[First,First+Alphabet)` の範囲とし、辞書順は符号なしバイトの昇順です。空文字列も登録できます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Trie()` / `clear()` | 空の Trie を作成／全ての頂点・登録・追加データを初期化 | $$O(\sigma)$$ / $$O(V+\sigma)$$ |
| `reserve(total_length)` | 文字数の合計を目安に頂点の領域を予約 | $$O(V\sigma)$$ |
| `insert(s,copies=1)` | `s` を `copies` 個追加し、終端頂点を返す | 償却 $$O(L+K\sigma)$$ |
| `erase(s,copies=1)` | 指定個数を削除。足りなければ変更せず `false` | $$O(L)$$ |
| `count(s)` / `contains(s)` | 完全一致する個数／登録されているか | $$O(L)$$ |
| `prefix_count(s)` | `s` を接頭辞に持つ登録文字列の個数 | $$O(L)$$ |
| `size()` / `distinct_size()` / `empty()` | 重複込みの個数／異なる文字列数／未登録か | $$O(1)$$ |
| `for_each_prefix(s,visit)` | `s` の接頭辞として登録された各終端頂点に `visit(v)`。短い順、重複はまとめて 1 回 | $$O(L)$$ |
| `longest_prefix(s)` | `s` の接頭辞として登録された最長の文字列の頂点。なければ `-1` | $$O(L)$$ |
| `lcp(s)` | `s` と登録文字列の LCP 長の最大値。未登録なら `0` | $$O(L)$$ |
| `lcp_sum(s)` | `s` と全登録文字列の LCP 長の総和。重複も数える | $$O(L)$$ |
| `order_of_key(s)` | `s` より辞書順で小さい登録文字列の個数 | $$O(L\sigma)$$ |
| `kth_node(k)` | 辞書順で 0 始まり `k` 番目の終端頂点。重複も数え、範囲外なら `-1` | $$O(H\sigma)$$ |
| `kth(k)` | 同じ選択結果を `optional<string>` で返す。範囲外なら `nullopt` | $$O(H\sigma)$$ |
| `lower_bound(s)` | `s` 以上で最小の登録文字列を `optional<string>` で返す | $$O((L+H)\sigma)$$ |

`copies` は正としてください。個数と LCP 長の総和は符号付きの `int64_t` です。個数・総和がその範囲を超える場合は `std::overflow_error` を送出します。追加する文字列は指定した文字範囲内とし、検索文字列には範囲外のバイトも使えます。NUL を含む文字列は `string` または長さを指定した `string_view` で渡してください。

頂点番号は根が `0`、新しい接頭辞が作られた順に増えます。同じ文字列の追加では同じ終端頂点を返します。削除しても頂点と追加データは残り、再追加でも同じ番号を使います。`clear()` 以外では既存の頂点番号は変わりません。

| 頂点を扱う操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `root` / `node_count()` | 根の頂点番号 `0`／根を含む頂点数 | $$O(1)$$ |
| `ensure(s)` | 経路だけを作り、終端頂点を返す。登録数は増やさない | 償却 $$O(L+K\sigma)$$ |
| `find(s)` / `walk(v,s)` | 根／頂点 `v` から `s` をたどった頂点。経路がなければ `-1` | $$O(L)$$ |
| `child(v,c)` | 文字 `c` に対応する子。存在しない、または範囲外なら `-1` | $$O(1)$$ |
| `children(v)` | 各文字の子の `const array<int,Alphabet>&`。添字は `c-First`、欠けた子は `-1` | $$O(1)$$ |
| `parent(v)` / `depth(v)` / `letter(v)` | 親／接頭辞の長さ／親からの文字。根の親は `-1`、根に `letter` は使えない | $$O(1)$$ |
| `terminal_count(v)` | 頂点 `v` で終わる登録文字列の個数 | $$O(1)$$ |
| `prefix_count(v)` | 頂点 `v` の部分木の登録文字列数。自身も含む | $$O(1)$$ |
| `insert_node(v,copies=1)` / `erase_node(v,copies=1)` | 既存頂点の文字列を追加／削除。返り値は `insert`／`erase` と同じ | $$O(d)$$ |
| `restore(v)` | 頂点が表す接頭辞を `string` で返す | $$O(d)$$ |
| `data(v)` | 頂点の追加データを `Data&` で取得。const Trie では `const Data&` | $$O(1)$$ |
| `for_each_ancestor(v,visit)` | `v` から根まで、下から順に `visit(u)` を呼ぶ。自身と根を含む | $$O(d)$$ |

`find`・`child`・`children` は構造を参照する操作です。登録されているかは `contains`・`terminal_count`、その接頭辞の文字列が残っているかは `prefix_count` で判定してください。`ensure` だけで作った経路や、全て削除した後の経路も参照できます。

`Data` は初期化可能で、`vector` に格納できる型とします。各頂点の初期値は `Data{}` です。登録・削除で個数は自動更新しますが、追加データの計算は利用者が行います。`visit` の呼び出し中では `data(v)` を変更できます。走査中の文字列の追加・削除・`clear()` は行わないでください。`data`・`children` の参照は、頂点追加や `reserve` による再確保・`clear()`・代入・破棄で無効になります。

表の「償却」は、一連の追加にかかる合計時間から評価した1回当たりの時間です。$$L$$ は引数の文字列長、$$H$$ は選ばれた文字列長、$$d$$ は対象頂点の深さ、$$K$$ は新しく作る頂点数、$$V$$ は根を含む頂点数、$$\sigma$$ は `Alphabet` です。`Data` の初期化・移動・破棄を定数時間としています。頂点数は `INT_MAX` 以下、保持する領域は $$O(V\sigma)$$ です。再帰は使いません。
