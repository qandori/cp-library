---
title: Closest Pair Core
documentation_of: //components/closest_pair.hpp
library_category: components/geometry
---

[最近点対](geometry_convex.md)の整数座標用の内部処理です。通常は `geometry::closest_pair(points)` を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `closest_pair_detail::packed(points,max_x,budget)` | 上位32 bitを縦座標、下位32 bitを横座標とした点列から、最近点対を返す。候補走査の予算を使い切ったら `nullopt` | 時間 $$O(n+\text{budget})$$、追加空間 $$O(n)$$ |

点数は2以上 `INT32_MAX` 以下、各座標は0以上 `INT32_MAX` 以下とし、`max_x` には横座標の最大値を渡します。返す2点の座標が同じ場合も、異なる入力要素の組です。`budget` は帯分割後の候補走査における距離評価回数の上限です。初期上界を求めるサンプリングには別途 $$O(n)$$ 時間を使います。`nullopt` の場合は呼び出し側で別の厳密な手法を使います。
