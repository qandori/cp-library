---
title: 畳み込みの型別振り分け
documentation_of: //components/convolution_dispatch.hpp
library_category: components/convolution
---

[畳み込み](convolution.md)と派生する畳み込みが共有する部品です。通常は `convolution/convolution.hpp` を読み込みます。

`convolution(a,b)` は同じ要素型の `vector<T>` を受け取り、積の係数を `vector<T>` で返します。空入力があれば空配列を返し、それ以外の長さは `a.size()+b.size()-1` です。要素型ごとの利用条件は公開側の畳み込みと同じです。

法 998244353、その他の固定法 modint、整数、浮動小数点・複素数、その他の加減乗算ができる型に応じて演算を選択します。FPS のクラス自体は読み込みません。

入力長の和を $$L$$ として、NTT・FFT を使う型では $$O(L\log L)$$ 時間です。それ以外の加減乗算を使う型では [一般の環の畳み込み](convolution_ring.md) を呼び、$$O(L^{\log_2 3})$$ 時間です。追加空間はいずれも $$O(L)$$ です。係数の演算を定数時間とします。
