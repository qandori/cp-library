---
title: modint998244353 の共有畳み込み
documentation_of: //components/mint_convolution.hpp
library_category: components/convolution
---

[FPS](fps.md)と[畳み込み](convolution.md)が共有する、法 998244353 の積の計算です。通常は公開側のライブラリを使います。

`mint_convolution_detail::multiply(a,b)` は `const vector<mint>&` を 2 つ受け取り、積の係数を `vector<mint>` で返します。入力は変更しません。空入力があれば空配列、それ以外は `a.size()+b.size()-1` 項です。同じ配列を両方に渡すこともできます。

NTT を使う場合、変換長は $$2^{23}$$ 以下にしてください。AVX2 が使えなくても同じ係数を返します。

入力長を $$n,m$$、NTT の変換長を $$N$$ とすると、時間は $$O(N\log N)$$、追加空間は $$O(N)$$ です。短い入力に対する直接計算では $$O(nm)$$ 時間です。
