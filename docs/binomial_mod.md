---
title: 任意 mod の二項係数
documentation_of: //math/binomial_mod.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/binomial_mod.hpp.html
- /lightweight/math/combinatorics/binomial_mod.hpp.html
---

素数・素数冪・合成数を法として二項係数を求めます。法が 1 の場合も扱えます。返り値は modint ではなく、法で割った余りの整数です。

```cpp
#include "lib/math/binomial_mod.hpp"

int main() {
    BinomialMod composite(72);
    auto a = composite.comb(10, 4); // 210 mod 72 = 66
    auto b = composite(1000000000000000000LL, 2); // 0
    auto c = composite(0, 0); // 1
    auto d = composite(3, 5); // 0

    BinomialMod prime(1000000007, 200000);
    auto e = prime(200000, 1); // 200000
    BinomialMod small(4294967295U, 64);
    auto f = small(20, 10); // 184756
}
```

法を $$m$$、相異なる素因数の個数を $$r$$、各素数冪を $$q_i=p_i^{e_i}$$、前計算上限を $$L$$、$$S=\displaystyle\sum_{i=1}^{r}\min(L,q_i-1)$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BinomialMod(modulus,table_limit=1000000)` | `std::uint32_t` の法と、前計算上限を指定して構築 | Pascal 表を使う場合 $$O(L^2)$$、それ以外は $$O(\sqrt m+S+r\log m)$$ |
| `mod()` | 法を `std::uint32_t` で返す | $$O(1)$$ |
| `max_n()` | 問い合わせ可能な `n` の上限を `std::int64_t` で返す | $$O(1)$$ |
| `comb(n,k)` / `operator()(n,k)` | `std::int64_t` の `n,k` に対し、二項係数の余りを `std::uint32_t` で返す | Pascal 表の範囲は $$O(1)$$、素数冪の表を使う範囲は $$\displaystyle O\!\left(\sum_{i=1}^{r}\log_{p_i}n\right)$$ |

法は `1 <= modulus <= UINT32_MAX` とします。`modulus==0` では `std::invalid_argument` を送出します。返り値は `[0,modulus)` に入り、法が 1 なら常に 0 です。`n<0`、`k<0`、`n<k` の場合も 0 を返します。

有効な問い合わせでは `0 <= n <= max_n()` とします。範囲を超えると `std::out_of_range` を送出します。範囲外の `k` に対する 0 の返却と、法が 1 の処理を先に行うため、これらは `n` の上限にかかわらず使えます。

**`table_limit` と `n` の上限について**

法の素因数分解に現れる各素数冪を $$q=p^e$$ とすると、素数冪ごとの表は `min(table_limit,q-1)` まで前計算します。

- 全ての素数冪で `q-1 <= table_limit` なら、`max_n()` は `INT64_MAX` です。既定値では、法が 1000000 以下なら必ずこの条件を満たし、`n=10^18` のような大きな値を扱えます。
- それ以外では、`max_n()` は `table_limit` です。例えば `BinomialMod(1000000007,200000)` は `n<=200000` の問い合わせを扱います。
- 法が 1 の場合は前計算を行わず、`max_n()` は `INT64_MAX` です。

法が 3 以上で `max_n() == table_limit` かつ `table_limit <= 64` の場合、Pascal の漸化式で `n <= table_limit` の表を作ります。上限なしで `table_limit <= 32` の場合も同じ表を使い、それより大きい `n` は素数冪ごとの表で計算します。法が 2 の場合、有効な問い合わせはビット演算で求めます。

大きな素数の法では、必要な `n` に合わせて `table_limit` を指定してください。`table_limit=0` も指定できます。前計算量は素数冪ごとの上限に比例するため、大きな上限には相応の時間・メモリが必要です。

```cpp
#include "lib/math/binomial_mod.hpp"
#include <limits>

int main() {
    BinomialMod small_mod(1000);
    bool large_n_supported = small_mod.max_n() == std::numeric_limits<std::int64_t>::max(); // true
    BinomialMod bounded(1000000007, 500000);
    auto limit = bounded.max_n(); // 500000
}
```

各オブジェクトは固定した法・前計算表を持ち、異なる法のオブジェクトを同時に使えます。コピーは独立します。法や上限を変更する場合は、新しいオブジェクトを構築してください。既存の [階乗・逆階乗による二項係数](comb.md) とは独立しており、このヘッダーの include だけで全体の階乗表を自動生成することはありません。

Pascal 表を作る場合は構築時間・保持空間ともに $$O(L^2)$$、表の範囲の問い合わせは $$O(1)$$ 時間です。Pascal 表を作らない場合は構築に $$O(\sqrt m+S+r\log m)$$ 時間、保持に $$O(S+\log m)$$ 空間を使います。素数冪の表で答える問い合わせは $$O(\displaystyle\sum_{i=1}^{r}\log_{p_i}n)$$ 時間です。いずれも問い合わせの追加空間は $$O(1)$$ です。法が 1 または 2、範囲外の `k`、`k=0`、`k=n` は定数時間です。
