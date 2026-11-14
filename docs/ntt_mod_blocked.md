---
title: 任意法畳み込みのブロック変換
documentation_of: //components/ntt_mod_blocked.hpp
library_category: components/convolution
---

[任意法・整数畳み込み](https://qandori.github.io/cp-library/convolution/convolution_mod.hpp.html) が使う内部部品です。通常は `convolution_mod(a,b,mod)` または `convolution_integer(a,b)` を使います。

`ntt_mod_detail::multiply_blocked<P,Root>(a,b,factor,workspace = nullptr)` は、二つの非空整数配列の畳み込みに `factor` を掛けた結果を、法 `P` 未満の `vector<uint32_t>` として返します。入力を変更せず、同じ配列の二乗にも対応します。

法 `P` は $$2<P<2^{30}$$ を満たす素数、`Root` は原始根、つまり$$0$$ 乗から $$P-2$$ 乗までが、法 $$P$$ の 1 から $$P-1$$ までの余りを全て一度ずつ取る値です。短い方の入力長を $$s>32$$、長い方を $$l$$ とします。変換長 $$n$$ は、$$l\ge8s$$ なら $$2s-1$$、それ以外は $$s+l-1$$ 以上の最小の 2 の冪です。$$n/8$$ が $$P-1$$ を割り切る必要があります。`factor` は法未満とします。

`workspace` を渡す場合は、32 byte 境界に配置した `blocked_workspace_words(n)` 個以上の生存中の `uint32_t` を、呼び出し中は維持します。省略すると内部で確保します。Linux x86_64 / GCC 用で、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を確認してから使います。

時間は $$O(l\log n+n+\log P)$$、返す配列を除く追加空間は $$O(n)$$ です。
