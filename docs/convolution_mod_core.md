---
title: 任意法畳み込みの共通部品
documentation_of: //components/convolution_mod_core.hpp
library_category: components/convolution
---

[任意法・整数畳み込み](https://qandori.github.io/cp-library/convolution/convolution_mod.hpp.html) と[軽量版畳み込み](https://qandori.github.io/cp-library/convolution/convolution_light.hpp.html) が共有する内部部品です。通常は公開ヘッダーを使います。

入力を法で割った非負の余りに直し、いくつかの素数を法として畳み込んだ後、中国剰余定理（CRT）で係数を復元します。CRT は、複数の法での余りを同時に満たす値を求める方法です。入力は 64 bit 以下の整数型、出力長は $$2^{24}$$ 以下です。空入力では空配列を返し、入力は変更しません。

入力長の和を $$L$$ として、時間は $$O(L\log L)$$、追加空間は $$O(L)$$ です。
