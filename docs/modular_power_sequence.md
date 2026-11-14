---
title: 剰余を取った冪の列
documentation_of: //components/modular_power_sequence.hpp
library_category: components/math
---

乗法畳み込みで使用する内部部品です。通常は [乗法畳み込み](multiplicative_convolution.md) を使ってください。

`modular_power_sequence_detail::generate(n, root, mod)` は、`root^i mod mod` を `i = 0, ..., n-1` の順に並べた `std::vector<std::uint32_t>` を返します。`n` は `std::size_t`、`root`・`mod` は `std::uint32_t` で、`mod >= 1` とします。空の列にも対応し、法 1 では全要素が 0 です。入力の法は素数でなくても構いません。

x86 の AVX2 が使え、法が $$2^{31}$$ 未満なら 8 要素ずつ生成します。それ以外の環境・法では通常の整数演算を使います。

時間・返り値を含む空間は $$O(n)$$ です。空列は定数時間・空間です。
