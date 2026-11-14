---
title: Link Cut Tree の共通処理
documentation_of: //components/link_cut_tree_core.hpp
library_category: components/tree
---

[頂点値を扱う版](link_cut_tree.md)と[辺値を扱う版](edge_link_cut_tree.md)が共有する内部処理です。通常はそれぞれの公開ヘッダーを使います。

`link_cut_tree_detail::Tree<S,op,e,commutative=false,directional=false>` は、`Tree(n=0)` または `Tree(values)` で辺のない森を作ります。頂点番号は `[0,n)` です。`op` は `op(op(a,b),c) == op(a,op(b,c))` を満たし、`e()` は左右どちらから `op` で結合しても値を変えないものとします。`commutative` は `op(a,b) == op(b,a)` が常に成り立つ場合だけ `true` にします。

`directional == false` の操作・引数・返り値は頂点版と同じです。`get(v)` は `const` な森でも呼べ、`S` を返します。`build_forest(edges)` はこの場合に使えます。

`directional == true` では、各要素に現在の根から見た順方向・逆方向の値を保持します。`set(v,forward,backward)` はこの2つの値を設定し、`get(v)` は `std::pair<S,S>` で返します。根を変えると向きも変わるため、辺版では取得・更新の前に開設時の始点を根にします。これらの操作は森の内部状態を変更します。初期値は両方向とも `e()`、配列から構築した場合は両方向ともその要素の値です。経路の集約と反転では、要素の順序と各要素自身の向きの両方を扱います。

要素数を $$n$$ として、構築は $$O(n)$$ 時間・空間です。`size()` は $$O(1)$$、方向なしの `get(v)` は $$O(1)$$、`build_forest` は辺数を $$m$$ として $$O(n+m)$$ です。他の操作は償却 $$O(\log n)$$ 時間、1回の最悪は $$O(n)$$ 時間です。「償却」は一連の操作の合計時間から求めた1回あたりの計算量です。`op`・`e` と値のコピー・交換を $$O(1)$$ とします。
