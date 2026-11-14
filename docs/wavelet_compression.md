---
title: "Wavelet Matrix の座標圧縮"
documentation_of: //components/wavelet_compression.hpp
library_category: components/util
---

通常の Wavelet Matrix と重み付き Wavelet Matrix が共有する内部の座標圧縮です。Wavelet Matrix 本体やビット列には依存しません。

`wavelet_detail::ValueIndex<T>::build(a)` は元の値を昇順・重複なしにした `values` と、各位置の圧縮値 `original` を作ります。`code(x)` は `values` 内で `x` 未満の値の数を返し、`x` が登録されていなくても使えます。

構築は整数型向けの基数ソートと一般型向けの比較ソートを使い分けます。圧縮関数 `compress` は、通常版が利用する値ごとの出現位置と区切りも生成できます。`compress<T,false>` はそれらを生成せず、`ValueIndex` はこの経路を使います。出現位置を生成する呼び出しでは、前回の出力を初期化してから処理するため、同じ格納先を再利用できます。

要素数を $$n$$、異なる値の数を $$\sigma$$ として、構築は整数型では $$O(n)$$、一般型では $$O(n\log n)$$ 時間、空間は $$O(n)$$ です。`code` は $$O(\log\sigma)$$ 時間・$$O(1)$$ 追加空間です。整数型のビット幅を定数とします。
