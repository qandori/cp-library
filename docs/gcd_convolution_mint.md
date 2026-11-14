---
title: modint998244353 の GCD 畳み込み
documentation_of: //components/gcd_convolution_mint.hpp
library_category: components/convolution
---

[GCD・LCM 畳み込み](gcd_lcm_convolution.md)が使用する `mint` 用の計算部品です。通常は `convolution/gcd_lcm_convolution.hpp` を読み込み、`gcd_convolution(a,b)` を使います。

`gcd_convolution_mint_detail::multiply(a,b,primes)` は同じ長さの別々の `std::vector<mint>&` を 2 つ受け取り、GCD 畳み込みを `a` に書き込みます。`b` の内容も変更します。長さは 4096 以上、最大添字は `UINT32_MAX` 以下、両配列の添字 0 は 0 とし、`primes` には `prime_sieve(a.size())` の結果を渡します。

配列長を $$N$$ として、時間は $$O(N\log\log N)$$、追加空間は $$O(N)$$ です。
