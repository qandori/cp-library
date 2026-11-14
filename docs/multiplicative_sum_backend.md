---
title: 乗法的関数の和の内部処理
documentation_of: //components/multiplicative_sum_backend.hpp
library_category: components/math
---

[乗法的関数の累積和](../math/multiplicative_prefix_sum.hpp.html) が、大きい上限での素数和と乗法的関数の和に使う補助部品です。通常は直接使う必要はありません。

この処理には 3 を掛けると 1 になる値（3 の逆元）が必要です。それを持たない型では、公開ライブラリが別の計算方法を選びます。

入力する関数・表の条件は公開ライブラリと同じです。上限を $$n$$ として、素数上の和の復元は $$\displaystyle O(\sqrt n\log\log n)$$ 時間です。

追加空間は $$\displaystyle O(\sqrt n)$$ です。
