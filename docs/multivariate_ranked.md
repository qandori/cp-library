---
title: 小さい次元の多変数畳み込み
documentation_of: //components/multivariate_ranked.hpp
library_category: components/convolution
---

[多変数の切り捨て畳み込み](multivariate_convolution.md)が使う内部処理です。各変数の次元が 2 または 3 の場合に、次数ごとの変換と積をまとめて計算します。通常は公開関数 `multivariate_convolution(a,b,dimensions)` を使います。

直接使う場合は、`multivariate_ranked_detail::multiply(a,b,dimensions)` に `std::vector<mint>` を 2 つと `std::vector<int>` の次元列を渡します。返り値は同じ長さの `std::vector<mint>` で、入力は変更しません。各次元は 2 または 3、次元 2 が 3 個以上、各次元から 1 を引いた値の総和は 24 未満とし、入力長は両方とも全次元の積にしてください。長さは $$2^{22}$$ 以下です。空の入力・次元列は使えません。添字と切り捨ての規約は公開関数と共通です。x86 の GCC・Clang で、AVX2 を利用できる CPU が必要です。内部関数ではこれらの条件を検査しません。

1 より大きい次元数を $$D$$、入力長を $$N$$ として、時間は $$O(D^2N)$$、追加空間は $$O(DN)$$ です。
