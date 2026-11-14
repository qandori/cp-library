---
title: Dynamic Wavelet Matrix の4値の内部演算
documentation_of: //components/dynamic_quad_wavelet.hpp
library_category: components/util
---

[Dynamic Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/dynamic_wavelet_matrix.hpp.html) の既定実装で使う内部部品です。`dynamic_wavelet_detail::QuadMatrix<T,Bits,Block,Branch>` は、一段で 2 bit を処理する動的 Wavelet Matrix です。通常は `DynamicWaveletMatrix<T,Bits>` を使います。

`T` は `bool` を除く 64 bit 以下の整数型、`Bits` は 1 以上かつ `T` の bit 数以下です。符号付き整数では符号 bit を反転して大小関係を保つキーへ変換し、最上位から 2 bit ずつ処理します。段数は $$\lceil\mathrm{Bits}/2\rceil$$ で、奇数 bit 幅では最上位段の有効な桁値が 0・1 です。各段の列は [Dynamic Quad Vector](https://qandori.github.io/cp-library/components/dynamic_quad_vector.hpp.html) に保持します。

既定は `Block=8192`・`Branch=32` です。`Block` は一つの葉に保持する値の個数、`Branch` は各節点が持つ子の最大数です。

クエリの意味と値域は公開クラスと同じです。`count` は指定値の個数、`prev_value` は境界未満の最大値、`next_value` は境界以上の最小値で、条件を満たす値がなければ `nullopt` です。`rank` は指定位置より前にある指定値の個数、`select` は指定値の 0 始まりで何番目の出現かを渡し、その位置を求めます。

公開する操作は `build`・`insert`・`erase`・`set`・`access`・`count`・`rank`・`select`・`kth_smallest`・`kth_largest`・`range_freq`・`prev_value`・`next_value` と、末尾の更新・サイズ・メモリ・copy／move／swap の操作です。詳細な引数と返り値は公開クラスの説明を参照してください。

要素数を $$n$$、bit 数を $$B=\mathrm{Bits}$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・クエリは $$O(B\log n)$$ です。`size`・`empty` は $$O(1)$$、`clear`・`memory_usage`・`swap` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。

償却計算量は、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。
