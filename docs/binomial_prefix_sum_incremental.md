---
title: 二項係数の prefix sum（差分更新）
documentation_of: //math/combinatorics/binomial_prefix_sum_incremental.hpp
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を、直前のクエリから差分更新して求めます。右端を含みません。クエリは呼び出した順に処理します。

```cpp
#include "lib/math/combinatorics/binomial_prefix_sum_incremental.hpp"

IncrementalBinomialPrefixSum<> sum(500000);
auto other = sum;
auto a = sum.query(5, 3); // 16
auto b = sum.query(6, 3); // 22
auto c = sum.query(6, 4); // 42
auto d = other.query(5, 2); // 6
auto e = sum.binom(5, 2); // 10
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `IncrementalBinomialPrefixSum<Mint>(max_n)` | 上限まで前計算。省略時は 0 | $$O(N)$$ |
| `auto other = sum` / `other = sum` | 表を共有し、現在のクエリ状態をコピー。その後の状態は独立 | $$O(1)$$ |
| `build(max_n)` | このオブジェクトだけ表を作り直し、状態を初期化 | $$O(N)$$ |
| `max_n()` | 前計算の上限 | $$O(1)$$ |
| `query(n,k)` | $$\displaystyle\sum_{i=0}^{k-1}\binom ni$$。右端を含まない | $$O(\lvert\Delta n\rvert+\lvert\Delta k\rvert)$$ |
| `binom(n,k)` | $$\binom nk$$。`n < 0`、`k < 0`、`n < k` なら 0 | $$O(1)$$ |

`Mint` の既定は `mint` です。素数 mod の `mod()`・`inv()` と整数からの構築・加減乗算に対応する modint を指定できます。`n` は 0 以上 `max_n` 以下、`max_n` は `mod()`・`INT_MAX` 未満とし、使用中に法を変更しないでください。`k` は `std::int64_t` で、$$[0,n+1]$$ に収めて扱います。

`build(max_n)` は前計算と状態を作り直し、`max_n()` は前計算の上限を返します。初期状態は $$(n,k)=(0,0)$$ です。同じクエリは $$O(1)$$、離れたクエリほど更新量が増えます。

入力を読んでから、必要な最大の `n` をコンストラクタへ渡してください。include だけでは前計算しません。`comb.hpp`・`COMB_MAX` は不要で、通常の二項係数も `binom(n,k)` から取得できます。

複数の状態を使う場合は、上の `auto other = sum` のようにコピーすると階乗・逆階乗・2の累乗の表を共有できます。`query` はそれぞれ独立して動き、片方の `build` は他方の表や状態を変更しません。`IncrementalBinomialPrefixSum<> first(n), second(n);` と別々に構築した場合は、それぞれ前計算します。

行と右端が同じ方向へ動く部分は斜めの漸化式でまとめ、右端だけが1動く部分は二項係数を直接加減算します。任意順のクエリにも対応し、クエリの並べ替えは行いません。

利用例：[ARC190 B の提出用コード](../test/benchmark/binomial_prefix_incremental_arc190_b.cpp)。小さい入力に固定の大きな前計算を行わず、4つの状態で表を共有します。

`max_n` を $$N$$ として、構築・`build` は $$O(N)$$ 時間・空間と逆元計算1回、`max_n()` は $$O(1)$$ です。

`query` は直前の状態との差を $$\Delta n,\Delta k$$ として $$O(\lvert\Delta n\rvert+\lvert\Delta k\rvert)$$ 時間・$$O(1)$$ 追加空間です。

クエリ数を $$Q$$、差分の総和を $$D$$ とすると全体は $$O(N+Q+D)$$ 時間です。表を共有するオブジェクトが $$P$$ 個なら、合計空間は $$O(N+P)$$ です。コピーは $$O(1)$$ 時間・追加空間、`binom` は $$O(1)$$ 時間・追加空間です。
