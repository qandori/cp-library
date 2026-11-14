---
title: 軽量な階乗・逆階乗テーブル
documentation_of: //components/factorial_table_light.hpp
library_category: components/math
---

[軽量な二項係数](comb_light.md)の共通部品です。通常版 FPS を読み込まず、`light_factorial_detail::Table<Mint>(max_n)` で階乗・逆階乗を計算します。

素数法 $$p=\texttt{Mint::mod()}$$ に対し、上限は $$N=\min(\texttt{max\_n},p-1)$$ です。`factorial[i]`、`inverse_factorial[i]`、構築時の `modulus` を持ちます。`max_n` は非負で `INT_MAX` 未満にし、構築した表を使っている間は法を変更しません。

`Mint` は整数からの構築、乗算、`mod()` と `inv()` に対応する型を指定します。階乗は前から、逆階乗は末尾の逆元を 1 回だけ求めて後ろから生成します。

`light_factorial_detail::Table<Mint, true>(max_n)` では、同じ範囲の整数の逆元も `std::vector<Mint>` の `inverse` に保存します。`inverse[0]` は便宜上 0 とし、正の `i` では `Mint(i) * inverse[i] == Mint(1)` です。第2テンプレート引数を省略すると `false` となり、`inverse` は空です。

構築は $$O(N+\log p)$$ 時間、$$O(N)$$ 空間です。逆元計算を $$O(\log p)$$ とします。
