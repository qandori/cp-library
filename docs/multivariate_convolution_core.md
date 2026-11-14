---
title: 多変数畳み込みの共通実装
documentation_of: //components/multivariate_convolution_core.hpp
library_category: components/convolution
---

[多変数の切り捨て畳み込み](multivariate_convolution.md)と[軽量版](multivariate_convolution_light.md)が共有する mod 998244353 用の内部処理です。通常は公開関数 `multivariate_convolution(a,b,dimensions)` を使います。

各変数の指数に上限を設けた多項式の積を計算します。上限以上の指数を含む項は結果から除きます。係数の並べ方と次数の指定方法は公開ページを参照してください。内部の `Core::multiply` に渡す次元列には、公開関数が 1 より大きい上限だけを残します。両入力の長さは、その列の要素の積に一致させます。

1 より大きい次元数を $$D$$、入力長を $$N$$ として、時間は $$O(DN\log N+D^2N)$$、追加空間は $$O(ND)$$ です。
