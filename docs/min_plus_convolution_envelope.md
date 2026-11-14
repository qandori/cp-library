---
title: 凹列と任意列の畳み込み（補助部品）
documentation_of: //components/min_plus_convolution_envelope.hpp
library_category: components/convolution
---

[凸・凹 min-plus / max-plus 畳み込み](min_plus_convolution.md)の整数向け処理です。

`min_plus_convolution_envelope_detail::multiply<T,minimum>(a,b)` は、`minimum=true` なら凹列 `a` と任意列 `b` の min-plus 畳み込み、`false` なら凸列 `a` と任意列 `b` の max-plus 畳み込みを `optional<vector<T>>` で返します。値域が広すぎて内部の 32 bit 整数で計算できない場合など、適用できない入力では `nullopt` を返します。入力は変更しません。

`T` は `bool` を除く 64 bit 以下の整数型です。$$L=\lvert a\rvert+\lvert b\rvert$$ として、時間は $$O(L\log L)$$、追加空間は $$O(L)$$ です。
