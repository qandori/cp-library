---
title: Suffix Tree (静的文字列)
documentation_of: //string/suffix_tree.hpp
library_category: string/substring
---

文字列の全接尾辞（各位置から末尾までの部分文字列）を、共通する先頭部分でまとめた木です。分岐しない経路を1本の辺にまとめるため、1辺が複数の文字を表すことがあります。部分文字列の検索・出現回数・出現位置、接尾辞同士の LCP（先頭から一致する最長の長さ）、木の頂点間の LCA（共通する祖先のうち最も深い頂点）、suffix link を扱います。文字列全体から一括構築する静的版で、末尾追加や文字の変更には再構築が必要です。

```cpp
#include "lib/string/suffix_tree.hpp"

int main() {
	SuffixTree tree("banana");
	assert(tree.contains("ana"));
	assert(tree.count("ana") == 2);
	assert((tree.occurrences("ana") == std::vector<int>{3, 1}));
	assert(tree.distinct_substrings() == 15);
	assert(tree.lcp(1, 3) == 3);
	auto found = tree.find("ana");
	if (found) {
		auto [l, r] = tree.label_range(tree.suffix_link(found->node));
		assert(tree.text().substr(l, r - l) == "na");
	}
	std::vector<int> order{SuffixTree::root};
	for (std::size_t i = 0; i < order.size(); ++i)
		for (int child : tree.children(order[i])) order.push_back(child);
	assert(order.size() == std::size_t(tree.node_count()));
}
```

文字列・検索語は `std::string_view` で渡します。バイト単位で扱い、NUL と 128 以上の値も含めた全 256 種類に対応します。辞書順は符号なしバイトの順序です。入力文字列は内部にコピーするため、構築後に元の文字列を変更・破棄できます。長さは `INT_MAX/2` 未満にしてください。

根は頂点 `0` で、空文字列を表します。根から頂点までの辺の文字列を連結したものが、その頂点の文字列です。`depth(v)` はその長さ、`edge_range(v)` は親からの辺、`label_range(v)` は頂点の文字列全体を取り出せる区間です。どちらも元の文字列上の半開区間で、特別な終端文字は含みません。

接尾辞の終点は内部頂点にもなります。例えば `aaa` では `a`・`aa`・`aaa` の全てが接尾辞で、最初の 2 つの終点にも子があります。終点かどうかは子の有無ではなく `suffix_start(v) != -1` で判定します。空の接尾辞は根で終わり、`suffix_node(size()) == root` です。

`suffix_array()` は空の接尾辞も含む長さ `n+1` の配列です。先頭は必ず `n` で、それ以外が通常の接尾辞配列です。`banana` では `[6,5,3,1,0,4,2]` になります。`lcp_array()[i]` は `suffix_array()[i]` と `suffix_array()[i+1]` の LCP 長で、配列の長さは `n` です。

`occurrence_range(v)` の `[l,r)` は **接尾辞配列上の区間** です。`suffix_array()[l]` から `suffix_array()[r-1]` が、その頂点の文字列の出現位置です。`occurrences(pattern)` も接尾辞配列の順で返し、位置の昇順ではありません。空文字列は位置 `0`～`n` に現れるため、出現回数は `n+1` です。

`find(pattern)` は見つかれば `optional<Locus>`、なければ `nullopt` を返します。`Locus.node` は終点を含む辺の行き先、`Locus.length` は検索語の長さです。辺の途中で終わる場合は `length < depth(node)` になります。どちらの場合も `occurrence_range(node)`・`occurrence_count(node)` を使えます。空の検索語は `{root,0}` です。

`advance(locus,c)` は検索位置から 1 バイト進めます。`transition(v,c)` は頂点から先頭バイトが `c` の辺の行き先を返し、辺全体を 1 バイトとして扱う操作ではありません。`suffix_link(v)` は頂点の文字列の先頭を 1 バイト除いた文字列の頂点です。辺の途中の `Locus` の文字列に対する suffix link ではありません。根の suffix link は根です。

頂点番号には、親が子より小さいなどの順序を仮定できません。走査には例のように `children` を使います。子は辺の先頭バイトの昇順です。`build`・`clear`・代入の後は、それ以前の頂点番号・`Locus`・参照を使わないでください。コピーとムーブに対応し、ムーブ元は空の木になります。

構築時に接尾辞配列・LCP・異なる部分文字列数と木の情報を作ります。LCP・LCA・suffix link 用の、区間の最小値を調べる索引（RMQ 索引）は、必要になった最初のクエリで作成します。終点の対応だけで suffix link を求められる場合や、祖先関係だけで LCA を求められる場合は作りません。通常の部分文字列検索には索引は不要です。

文字数を $$n$$、検索語の長さを $$m$$、出現回数を $$k$$、子の数を $$d$$、RMQ のブロック数を $$b=\lceil n/32\rceil$$ とします。† は索引を初めて必要とする際に追加で $$O(n+b\log b)$$ 時間・空間を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SuffixTree()` / `SuffixTree(s)` | 空の木 / 文字列 `s` から構築 | 空は $$O(1)$$、文字列は $$O(n)$$ |
| `build(s)` / `clear()` | 再構築 / 空の木に戻す | 新旧の文字数の合計に対して $$O(n)$$ |
| `size()` / `empty()` / `text()` | 文字数 / 空判定 / 保存した文字列への `string_view` | $$O(1)$$ |
| `node_count()` / `root` | 頂点数 / 根の番号 `0` | $$O(1)$$ |
| `parent(v)` / `depth(v)` | 親の番号（根は `-1`）/ 根からの文字数 | $$O(1)$$ |
| `children(v)` | 子の番号の `span<const int>` | 取得 $$O(1)$$、走査 $$O(d)$$ |
| `edge_range(v)` / `label_range(v)` | 辺 / 頂点全体を表す元の文字列上の区間 | $$O(1)$$ |
| `transition(v,c)` | 先頭が `c` の辺の行き先。なければ `-1` | $$O(\log\sigma)$$、根は $$O(1)$$ |
| `find(pattern)` / `contains(pattern)` / `count(pattern)` | 検索位置 / 存在判定 / 出現回数 | $$O(m)$$（バイト種類数は固定） |
| `advance(locus,c)` | 検索位置を 1 バイト進める。進めなければ `nullopt` | $$O(\log\sigma)$$、辺の途中は $$O(1)$$ |
| `occurrences(pattern)` | 出現位置を接尾辞配列の順に `vector<int>` で返す | $$O(m+k)$$ |
| `occurrence_range(v)` / `occurrence_count(v)` | 出現する接尾辞配列の区間 / 出現回数 | $$O(1)$$ |
| `suffix_start(v)` | 頂点で終わる接尾辞の開始位置。なければ `-1`。根は `n` | $$O(1)$$ |
| `suffix_node(p)` | `text().substr(p)` の終点。`0 <= p <= n` | $$O(1)$$ |
| `suffix_array()` / `lcp_array()` | 接尾辞配列 / 隣接する接尾辞の LCP 配列への const 参照 | $$O(1)$$ |
| `lcp(p,q)` | 位置 `p`・`q` から始まる接尾辞の LCP 長。両方とも `n` 以下 | † $$O(1)$$ |
| `lca(u,v)` | 木の頂点の最小共通祖先 | † $$O(1)$$ |
| `suffix_link(v)` | 頂点の文字列の先頭を除いた文字列の頂点 | † $$O(1)$$ |
| `distinct_substrings()` | 空でない異なる部分文字列の数を `uint64_t` で返す | $$O(1)$$ |
| `swap(other)` | 内容と索引を交換 | $$O(1)$$ |
| コピー / ムーブ | 独立した複製 / 内容を移動し、移動元を空の木に戻す | コピーは索引を含む保持量に比例、ムーブ構築は $$O(1)$$ |

$$\sigma\leq256$$ はバイトの種類数です。基本の構築時間・空間は $$O(n)$$、遅延索引を含む全構築時間・空間は $$O(n+b\log b)$$ です。
