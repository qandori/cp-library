---
title: Aho–Corasick
documentation_of: //string/aho_corasick.hpp
library_category: string/pattern_matching
---

複数のパターンを登録し、本文を1回走査して一致を調べます。重なる出現も数えます。

```cpp
#include "lib/string/aho_corasick.hpp"
#include <cassert>
#include <string_view>

int main() {
	AhoCorasick<> ac;
	int he = ac.add("he");
	int she = ac.add("she");
	int e = ac.add("e");
	ac.build();
	std::string_view text = "ushe";
	auto count = ac.occurrences(text);
	assert(count[he] == 1 && count[she] == 1 && count[e] == 1);
	assert(ac.count_matches(text) == 3);
	ac.find_all(text, [&](std::size_t end, int id) {
		std::size_t begin = end - ac.length(id);
		assert(text.substr(begin, end - begin) == (id == he ? "he" : id == she ? "she" : "e"));
	});
	int state = ac.walk(ac.root, "us");
	state = ac.transition(state, 'h');
	state = ac.transition(state, 'e');
	assert(ac.match_count(state) == 3);
}
```

`add` の返り値は追加順のパターン ID です。全てのパターンを追加してから `build()` を呼んでください。同じ文字列を複数回追加すると別の ID になり、それぞれを一致として数えます。`build()` 後にパターンを追加する場合は、`clear()` して全て登録し直します。本文ごとの検索は状態を変更しないので、同じ構築結果を繰り返し使えます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `AhoCorasick<Alphabet, First>()` | 空の [Trie](trie.md)（文字列の共通する接頭辞をまとめた木）。既定は英小文字26文字、`First='a'` | $$O(\sigma)$$ |
| `reserve(total_length, patterns=0)` | パターンの長さの合計と個数を目安に領域を確保。呼び出しは任意 | $$O(V\sigma+P)$$ |
| `clear()` | パターン・構築結果を全て削除。確保済み領域は保持 | $$O(V+P)$$ |
| `add(pattern)` | `string_view` のパターンを追加し、パターン ID を返す。構築前のみ | 償却 $$O(\lvert pattern\rvert+K\sigma)$$ |
| `build()` | suffix link・output link・全遷移を構築。構築済みなら何もしない | 初回 $$O(V\sigma)$$、以降 $$O(1)$$ |
| `size()` / `empty()` | パターンの個数／パターンが未登録か | $$O(1)$$ |
| `node_count()` / `built()` | 根を含む Trie の頂点数／構築済みか | $$O(1)$$ |
| `terminal(id)` / `length(id)` | パターン ID の終端頂点／長さ | $$O(1)$$ |
| `parent(v)` / `depth(v)` | Trie の親／根からの深さ。根の親は根 | $$O(1)$$ |
| `child(v,c)` | Trie の直接の子。存在しない、または文字が範囲外なら -1。構築前後で利用可能 | $$O(1)$$ |
| `suffix_link(v)` | `v` が表す文字列より短い接尾辞のうち、Trie に存在する最長のものの頂点。根からは根 | $$O(1)$$ |
| `output_link(v)` | suffix link をたどって最初に出会うパターンの終端頂点。自身は含まず、存在しなければ -1 | $$O(1)$$ |
| `terminal_count(v)` | 頂点 `v` ちょうどを終端とするパターン数。重複も含む | $$O(1)$$ |
| `match_count(v)` | 頂点 `v` の文字列の末尾で一致するパターン数。自身・重複・空文字列も含む | $$O(1)$$ |
| `transition(v,c)` | 状態 `v` で1文字読み込んだ次の状態。文字が範囲外なら根へ戻る | $$O(1)$$ |
| `walk(v,text)` | 状態 `v` から本文を読み進め、最後の状態を返す | $$O(\lvert text\rvert)$$ |
| `for_each_match(v,visit)` | 状態 `v` で一致する各パターン ID に `visit(id)` を呼ぶ | $$O(Z_v)$$ |
| `find_all(text,visit)` | 全一致について `visit(end,id)` を呼ぶ。`end` は `size_t` の右端を含まない位置 | $$O(n+Z)$$ |
| `count_matches(text)` | 全パターンの出現回数の合計を `uint64_t` で返す。列挙しない | $$O(n)$$ |
| `occurrences(text)` | 各パターン ID の出現回数を `vector<uint64_t>` で返す。列挙しない | $$O(n+V+P)$$ |
| `bfs_order()` | 根からの幅優先順を `const vector<int>&` で返す。根を含み、suffix link の行き先が先に来る | $$O(1)$$ |
| `propagate(values)` | 頂点ごとの値を suffix link の行き先へ、幅優先順の逆順で `+=` する | $$O(V)$$ |
| `root` | 根の頂点番号 0 | $$O(1)$$ |

表の「償却」は、一連の追加にかかる合計時間から評価した1回当たりの時間です。$$K$$ は、その `add` で新しく作られる頂点数です。パターンを追加した順に、まだ存在しない接頭辞へ頂点番号を割り当てます。`terminal`・`length` の引数はパターン ID、それ以外の状態を取る関数の引数は頂点番号です。

`suffix_link`・`output_link`・`match_count` と検索・集計・`bfs_order`・`propagate` は `build()` 後に使います。状態は読み込んだ文字列の接尾辞のうち、Trie に存在する最長のものを表します。本文を分割して読む場合は `walk`・`transition` の返り値を次の呼び出しへ渡します。

`find_all` の一致区間は `[end-length(id),end)` です。右端の昇順に列挙し、同じ右端では長いパターンから、同じパターンの重複は追加順の逆順に列挙します。`visit` の呼び出し中に登録内容や構築結果を変更しないでください。`for_each_match` も同じ長さ順です。

空文字列を追加すると、長さ $$n$$ の本文の前後を含む $$n+1$$ か所で一致します。空のパターン集合も使えます。総出現回数は `uint64_t` の範囲に収めてください。

`propagate` に渡す `vector<T>` の要素数は `node_count()` とし、`T` は `+=` に対応させます。各走査状態に1を加えてから伝播すると、`values[terminal(id)]` が出現回数になります。空パターンも数えるときは、走査開始前に根にも1を加えます。入力配列を書き換えるため、同じ配列へ繰り返すと再加算されます。

文字はバイト単位です。`add` は `[First,First+Alphabet)` の文字だけを受け付けます。本文に範囲外の文字が出た場合は一致を途切れさせ、次の文字から再開します。`AhoCorasick<256,0>` なら NUL や128以上も含む全256バイト、`AhoCorasick<10,'0'>` なら数字を扱えます。パターン数・頂点数は `INT_MAX` 以下としてください。

既定の26文字では、頂点の配列と幅優先順だけで1頂点あたり約132バイトを使います。大きな文字集合を指定すると、その分メモリが増えます。

パターンの長さの合計を $$L$$、個数を $$P$$、Trie の頂点数を $$V\leq L+1$$、文字集合の大きさを $$\sigma$$、本文の長さを $$n$$ とします。登録・構築は合計 $$O(L+P+V\sigma)$$ 時間、保持する空間は $$O(V\sigma+P)$$ です。`occurrences` は $$O(n+V+P)$$ 時間・$$O(V+P)$$ 追加空間、`count_matches` は $$O(n)$$ 時間・$$O(1)$$ 追加空間、全列挙は一致数を $$Z$$ として $$O(n+Z)$$ 時間・$$O(1)$$ 追加空間です。コールバックと `+=` の計算量は1回あたり $$O(1)$$ としています。
