---
title: LCM 畳み込みの固定 mod 用処理
documentation_of: //components/lcm_convolution_mint.hpp
library_category: components/convolution
---

`lcm_convolution_mint_detail` 名前空間の内部部品です。通常は [LCM 畳み込み](gcd_lcm_convolution.md) から使います。

`multiply(a,b,primes)` は、同じ長さの2つの異なる `std::vector<T>` と、その長さ未満の全素数を昇順に持つ `const std::vector<std::size_t>&` を受け取ります。結果を `a` に格納し、`b` も作業領域として変更します。返り値は `void` です。

配列の長さは1以上で、添字0を0とし、末尾の添字を `UINT32_MAX` 以下にします。`T` は加減算・乗算・既定構築ができる型です。公開ライブラリは十分長い `mint` の配列に対してこの経路を選び、それ以外には汎用経路を使います。

配列の長さを $$n$$ として、時間は $$O(n\log\log n)$$、追加空間は $$O(n)$$ です。
