---
title: Dynamic Wavelet Matrix の4値の内部演算
documentation_of: //components/dynamic_quad_wavelet.hpp
---

[Dynamic Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp.html) の既定実装で使う内部部品です。`dynamic_wavelet_detail::QuadMatrix<T,Bits,Block,Branch>` は、一段で 2 bit を処理する動的 Wavelet Matrix です。通常は `DynamicWaveletMatrix<T,Bits>` を使います。

`T` は `bool` を除く 64 bit 以下の整数型、`Bits` は 1 以上かつ `T` の bit 数以下です。符号付き整数では符号 bit を反転して大小関係を保つキーへ変換し、最上位から 2 bit ずつ処理します。段数は $$\lceil\mathrm{Bits}/2\rceil$$ で、奇数 bit 幅では最上位段の有効な桁値が 0・1 です。各段の列は [Dynamic Quad Vector](https://qandori.github.io/cp-library/components/dynamic_quad_vector.hpp.html) に保持します。

`build(a)` は各段の桁値を一括で構築し、その段の個数を使って安定に並べ替えます。全要素で共通する段は並べ替えを省き、クエリでも区間を動かさず次の段へ進みます。挿入・削除では各桁値の開始位置を更新し、代入では旧位置から新位置への移動をまとめます。全要素で共通する桁や、値を変えない代入には専用の経路を使います。

既定は `Block=8192`・`Branch=32` です。葉に更新用の空きを残しつつ、初期構築で木が余計に一段深くなることを避けます。代入時は旧値と新値の間にある桁値の開始位置だけを更新します。

クエリの意味と値域は公開クラスと同じです。`count` は指定値に一致する経路だけをたどります。2境界の `range_freq` は共通する上位桁を共有します。`prev_value` は境界未満の最大値、`next_value` は境界以上の最小値で、条件を満たす値がなければ `nullopt` です。境界に沿って下降し、必要に応じて保存した候補の枝から最大値・最小値を求めます。`select` は候補が一要素になった後、残りの桁の一致を調べてから位置を復元します。

公開する操作は `build`・`insert`・`erase`・`set`・`access`・`count`・`rank`・`select`・`kth_smallest`・`kth_largest`・`range_freq`・`prev_value`・`next_value` と、末尾の更新・サイズ・メモリ・copy／move／swap の操作です。詳細な引数と返り値は公開クラスの説明を参照してください。既定と異なる `BitVector` を渡す二分実装は、公開ヘッダー側に保持しています。

要素数を $$n$$、bit 数を $$B=\mathrm{Bits}$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・クエリは $$O(B\log n)$$ です。`size`・`empty` は $$O(1)$$、`clear`・`memory_usage`・`swap` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
