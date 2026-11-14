---
title: 同じ長さの畳み込みの作業領域
documentation_of: //components/multivariate_convolution_product.hpp
library_category: components/convolution
---

[多変数の切り捨て畳み込み](multivariate_convolution.md)が使う内部処理です。通常は公開関数 `multivariate_convolution(a,b,dimensions)` を使います。

`multivariate_convolution_detail::Product` は、法 998244353 の `mint` 配列の積を計算します。引数を取らずに構築し、`operator()(a,b)` に `const vector<mint>&` を 2 つ渡します。同じオブジェクトへの呼び出しでは、両入力の長さを常に同じ固定値 $$n$$ にしてください。長さの範囲は $$1\le n\le 2^{22}$$ です。入力は変更せず、同じ配列を両方に渡すこともできます。

返り値は長さ $$2n-1$$ の `span<const mint>` です。添字 $$k$$ は $$0\le k<2n-1$$ で、係数は入力の添字 $$i,j$$ が $$i+j=k$$ となる積の和です。返された範囲は、同じオブジェクトを次に呼び出すか破棄するまで有効です。空入力には対応しません。

1 回の呼び出しの時間は $$O(n\log n)$$、保持する作業領域は $$O(n)$$ です。
