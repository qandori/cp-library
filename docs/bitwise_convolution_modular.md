---
title: ビット演算畳み込みの modint 用共通実装
documentation_of: //components/bitwise_convolution_modular.hpp
library_category: components/convolution
---

[AND・OR・XOR 畳み込み](bitwise_convolution.md)が共有する内部処理です。通常は公開関数 `and_convolution`・`or_convolution`・`xor_convolution` を使います。

入力長を 2 の冪に切り上げた値を $$N$$ として、時間は $$O(N\log N)$$、追加空間は $$O(N)$$ です。
