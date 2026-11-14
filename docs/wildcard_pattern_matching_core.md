---
title: ワイルドカード照合の内部部品
documentation_of: //components/wildcard_pattern_matching.hpp
library_category: components/string
---

[ワイルドカード付きパターンマッチング](https://qandori.github.io/cp-library/string/wildcard_pattern_matching.hpp.html) の内部部品です。通常は公開ヘッダーを使います。

`wildcard_matching_detail::filter_ntt(text,pattern,matches,bound,text_has_wildcard,pattern_has_wildcard)` は、符号化した本文・パターンを照合し、`matches` の一致しない位置を `false` にします。既に `false` の位置を `true` に戻すことはありません。文字列は `std::vector<uint16_t>` で、`0` はワイルドカード、`1`～`256` は通常の文字を表します。同じ文字には同じ値、異なる通常の文字には異なる値を使ってください。本文長を $$n$$、パターン長を $$m$$ とし、`matches` の長さは `n-m+1` とします。

`bound` は各位置における、両方が通常の文字である組の値の差を二乗した総和の上限です。`uint64_t` で渡します。この総和は非負で、一致する場合だけ 0 になります。`text_has_wildcard`・`pattern_has_wildcard` は、それぞれの入力に `0` が含まれるかを表す `bool` です。返り値は `void` で、入力の本文・パターンは変更しません。

$$1\le m\le n\le \mathrm{INT\_MAX}$$、$$m\le 2^{22}$$ を満たしてください。全ての文字を比較可能な値で符号化した場合、上限として $$65025m$$ を使えます。より小さな符号を使えば、それに応じた上限を指定できます。上限は内部で使う二つの法の積 $$468937312667959297$$ より小さくなければなりません。パターン長が $$2^{22}$$ を超えた場合は `std::length_error` を送出します。

時間計算量は $$O(n\log m)$$、`matches` と入力を除く追加空間は $$O(m)$$ です。$$m=1$$ の対数の項は定数として扱います。前計算や共有表を呼び出し後に保持しません。
