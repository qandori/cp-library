---
title: 行列のブロック逆行列・余因子行列
documentation_of: //components/matrix_block_inverse.hpp
library_category: components/math
---

[行列](matrix.md)が使う、素数法の modint 向けの補助部品です。通常は `Matrix<T>::inverse()`・`adjugate()` を利用します。

`matrix_detail::block_inverse(a,n,adjugate)` は、行優先で並べた `n*n` 要素の `const vector<T>&` を受け取ります。`adjugate` が `false` なら逆行列、`true` なら余因子行列の計算を試みます。入力は変更しません。

返り値は `optional<pair<T,vector<T>>>` で、組の先頭は行列式です。この経路を使えない場合は `nullopt` を返すため、呼び出し側で通常の消去法を使います。逆行列が存在しないときの組の配列は空です。

時間は $$O(n^3)$$、追加空間は $$O(n^2)$$ です。modint の四則演算を定数時間とします。
