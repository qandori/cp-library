---
title: 整数点の凸包（補助部品）
documentation_of: //components/packed_convex_hull.hpp
library_category: components/geometry
---

[凸包](geometry_convex.md)で使う、整数座標の値域を確認してから適用する共通処理です。対応しない値域は呼び出し元の汎用処理を使います。

`packed_convex_hull_detail::build(order,keep_collinear)` は、整数点を格納した配列から凸包を返します。各点の横座標を `x`、縦座標を `y` として、`(uint64_t(x) << 32) | uint32_t(y)` で 1 個の `uint64_t` にまとめます。`x,y` は 0 以上 `INT32_MAX` 以下にしてください。元の座標がこの範囲外でも、全点に同じ値を足し引きして範囲に収められれば利用できます。

入力は `vector<uint64_t>` または `vector<Entry>` で、`Entry` は点の整数表現 `key` と付随する `int index` を持ちます。重複座標は 1 点にまとめ、反時計回りの凸包を返します。`keep_collinear` が `true` なら辺上の点も残し、全点が一直線の場合は座標順の列を返します。空入力には空列を返します。

`diameter(hull)` へは、上と同じ整数表現で `build(order,false)` により得た非空の凸包を渡します。距離が最大となる 2 点の整数表現を `pair<uint64_t,uint64_t>` で返し、1 点だけならその点を両方に返します。同距離の組はどれか一つです。

入力点数を $$n$$、凸包の点数を $$h$$ とすると、`build` は $$O(n)$$ 時間・追加空間、`diameter` は $$O(h)$$ 時間・$$O(1)$$ 追加空間です。座標のビット幅を定数とします。
