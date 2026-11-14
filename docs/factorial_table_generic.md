---
title: 汎用 modint の階乗・逆階乗生成
documentation_of: //components/factorial_table_generic.hpp
library_category: components/math
---

通常版・軽量版の[二項係数](comb.md)が共有する表の生成処理です。利用者向けの階乗・逆階乗は二項係数ライブラリから取得できます。

`factorial_table_detail::build<Mint, Canonical = true>(factorial,inverse_factorial,n)` は、各 `Mint*` の先頭 `n+1` 要素に階乗・逆階乗を書き込みます。領域は重ならない有効な配列とし、素数法 `p = Mint::mod()` に対して `0 <= n < p` を満たしてください。

`Mint` は整数からの構築・乗算・`inv()` に対応させます。`Canonical = true` かつ整数の `val()` を持つ型では、`val()` がその値と法で割った余りの等しい整数を返す必要があります。負の整数を返す型にも対応します。`false` では型自身の乗算を使います。表を生成している間は法を変更しません。

`factorial_table_detail::inverses<Mint>(inverse,factorial,inverse_factorial,n)` は、前計算済みの階乗・逆階乗から `0` 以上 `n` 以下の整数の逆元を `Mint* inverse` に書き込みます。`inverse[0]` は便宜上 0 とします。各配列は先頭 `n+1` 要素が有効で、出力領域は入力領域と重ならないものとします。`n` と法の条件は `build` と同じです。時間は $$O(n)$$、出力配列を除く追加空間は $$O(1)$$ です。

逆元計算を $$O(\log p)$$ として、時間は $$O(n+\log p)$$、出力配列を除く追加空間は $$O(1)$$ です。
