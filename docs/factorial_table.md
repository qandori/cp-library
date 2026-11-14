---
title: 型を指定する階乗・逆階乗テーブル
documentation_of: //components/factorial_table.hpp
library_category: components/math
---

modint 型ごとに階乗・逆階乗を構築する内部部品です。通常は[二項係数ライブラリ](comb.md)を利用します。

`factorial_detail::Table<Mint>(max_n)` は、素数 `Mint::mod()` に対し `0` から `min(max_n,Mint::mod()-1)` まで前計算します。`factorial[i]` が階乗、`inverse_factorial[i]` が逆階乗、`modulus` が構築時の法です。構築後の modint の法は変更しないでください。変更した場合は新しいテーブルを構築します。

`Mint` は整数からの構築、乗算、`mod()` と `inv()` に対応するものを使います。`max_n` は `0` 以上 `INT_MAX` 未満とします。

`factorial_detail::Table<Mint, true>(max_n)` では、同じ範囲の整数の逆元も `std::vector<Mint>` の `inverse` に保存します。`inverse[0]` は便宜上 0 とし、正の `i` では `Mint(i) * inverse[i] == Mint(1)` です。第2テンプレート引数を省略すると `false` となり、`inverse` は空です。

実際の前計算上限を $$N$$、法を $$p$$ とすると、構築は $$O(N+\log p)$$ 時間、$$O(N)$$ 空間です。modint の乗算を $$O(1)$$、逆元計算を $$O(\log p)$$ とします。
