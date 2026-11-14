---
title: Dijkstra の内部キュー
documentation_of: //components/dijkstra_queue.hpp
library_category: components/graph
---

`shortest_path_detail::RadixQueue<T>` は [Dijkstra](https://qandori.github.io/cp-library/graph/dijkstra.hpp.html) が使う優先度付きキューです。最後に取り出したキー以上の値だけを追加できます。通常は `dijkstra`・`dijkstra_path` を使います。`T` は 64 bit 以下の整数型、キーは非負です。頂点番号は非負の `int` です。

`shortest_path_detail::DistanceQueue<T>` は、通常の Dijkstra とトポロジカル前処理で共有するキュー型です。64 bit 以下の整数距離には `RadixQueue<T>`、それ以外には二分ヒープの `BinaryQueue<T>` を使います。いずれも `empty()`・`push(key,vertex)`・`pop()` を持ち、`pop()` は最小キーと頂点の `pair<T,int>` を返します。現在の要素数を $$q$$ として、二分ヒープの追加は償却 $$O(\log q)$$、削除は $$O(\log q)$$ 時間です。同時に保持した要素数の最大を $$Q$$ とすると、確保済み容量を含む空間は $$O(Q)$$ です。要素の演算・移動を定数時間とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RadixQueue<T>()` | 下限 0 の空キューを作る | $$O(w)$$ |
| `empty()` | 要素がないか | $$O(1)$$ |
| `push(key,vertex)` | 最後に取り出したキー以上の要素を追加 | 償却 $$O(1)$$ |
| `pop()` | 最小キーと頂点の `pair<T,int>` を返して削除。非空が必要 | 償却 $$O(w)$$ |

キーの bit 数を $$w\le64$$ として、追加は償却 $$O(1)$$、取り出しは償却 $$O(w)$$ 時間です。格納配列の確保容量の合計を $$C$$ として、保持空間は $$O(C+w)$$ です。要素を取り出した後も容量を保持し、同時に保持した要素数の最大を $$Q$$ とすると $$C=O(wQ)$$ です。内部で 32 bit から 64 bit の管理へ切り替わる場合に限り、その時点の要素数に比例する処理が 1 回加わります。
