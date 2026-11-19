# ライブラリの配置とサイトの分類

ヘッダーは次の大分類の直下に置きます。細かい分類は GitHub Pages の一覧に表示します。

| フォルダ | 内容 |
| --- | --- |
| `flow/` | 最大流、最小費用流、Project Selection |
| `tree/` | 木の分解、経路・部分木クエリ、木 DP、動的木 |
| `graph/` | 一般のグラフの最短路、連結性、マッチングなど |
| `data_structure/` | 集合・区間の更新や問い合わせ。セグメント木、Union-Find、永続データ構造、Wavelet Matrix、区間 Mo など |
| `math/` | modint、整数論、二項係数、Floor Sum、行列など |
| `convolution/` | 畳み込み |
| `fps/` | 形式的冪級数、多項式 |
| `sequence/` | 列全体の解析・変換。最長増加部分列（LIS）、転倒数、順列の順位、連長圧縮（RLE） |
| `geometry/` | 点・直線・線分・円・多角形・凸包・最近点対 |
| `string/` | 文字列のアルゴリズム・データ構造 |
| `components/` | 共有する内部部品 |

重心分解・HLD・Static Top Tree・木上 Mo は `tree/` にあります。静的区間転倒数など、列の区間を指定して答えを求めるものは `data_structure/` にあります。

Block Cut Tree のように一般のグラフを入力にするものは `graph/`、Trie や Suffix Tree のように文字列を扱うものは `string/` から探せます。「応用ライブラリ」の区分は、このフォルダ配置とは独立しています。

リポジトリを `lib/` として配置した場合、次のように include します。

```cpp
#include "lib/math/comb.hpp"
#include "lib/flow/max_flow.hpp"
#include "lib/graph/dijkstra.hpp"
#include "lib/data_structure/persistent_segtree.hpp"
#include "lib/data_structure/union_find.hpp"
```

通常版と軽量版は同じフォルダに配置し、実装が異なる場合はファイル名の `_light` で区別します。

サイトの一覧では「最短路」「フロー」「永続データ構造」「整数論」などの細分類から探せます。

## AtCoder Library 形式での呼び出し

対応するヘッダーを include したうえで `cp_acl::` を付けると、[AtCoder Library の公開操作](https://atcoder.github.io/ac-library/production/document_ja/) と同じ名前・引数順で呼び出せます。使用条件と計算量はリンク先の各ライブラリを参照してください。

| `cp_acl::` から使える名前 | include するライブラリ |
| --- | --- |
| `segtree<S,op,e>`、`lazy_segtree<S,op,e,F,mapping,composition,id>` | [セグメント木](segtree.md)、[遅延セグメント木](lazy_segtree.md) |
| `fenwick_tree<T>`、`dsu` | [Fenwick Tree](fenwick_tree.md)、[Union-Find](union_find.md) |
| `mf_graph<Cap>`、`mcf_graph<Cap,Cost>` | [最大流](max_flow.md)、[最小費用流](min_cost_flow.md) |
| `scc_graph`、`two_sat` | [強連結成分分解](scc.md)、[2-SAT](two_sat.md) |
| `static_modint<Mod>`、`dynamic_modint<Id>`、`modint998244353`、`modint1000000007`、`modint` | [modint](modint.md) |
| `pow_mod(x,n,m)`、`inv_mod(x,m)`、`crt(r,m)`、`floor_sum(n,m,a,b)` | [累乗・逆元](modular_arithmetic.md)、[CRT](crt.md)、[Floor Sum](floor_sum.md) |
| `convolution<Mod>(a,b)`、`convolution(a,b)`、`convolution_ll(a,b)` | [畳み込み](convolution.md) または [軽量版](convolution_light.md) |
| `suffix_array`、`lcp_array`、`z_algorithm` | [Suffix Array](suffix_array.md)、[LCP Array](lcp_array.md)、[Z Algorithm](z_algorithm.md) |
