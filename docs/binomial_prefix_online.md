---
title: 二項係数の累積和のオンライン処理部品
documentation_of: //components/binomial_prefix_online.hpp
library_category: components/math
---

[二項係数の累積和](https://qandori.github.io/cp-library/math/binomial_prefix_sum.hpp.html) の通常版・軽量版で共有する内部実装です。利用するときは公開ヘッダーを include してください。

`binomial_prefix_detail::Online<Mint,Table>` は、素数 mod の要素型 `Mint` と階乗・逆階乗・2 の累乗を生成する `Table` を使います。`n,k` を一定間隔で選んだ点に対応する前計算表を格子と呼びます。`max_n` は非負かつ `INT_MAX` と法より小さく、`block_size` は非負にします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Online(max_n=0,block_size=0)` | 上限とブロック幅を指定して構築。幅 0 は平方根を基準に決定 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 前計算を作り直す。返り値は `void` | $$O(N+N^2/B)$$ |
| `query(n,k)` | 右端を含まない累積和を `Mint` で返す | $$O(B)$$ |
| `prod(n,l,r)` | `l <= r` の区間和を `Mint` で返す | $$O(B)$$ |
| `max_n()` | 上限を `int` で返す | $$O(1)$$ |
| `block_size()` | ブロック幅を `int` で返す | $$O(1)$$ |
| `table_size()` | 格子に保存した `Mint` の個数を `std::size_t` で返す | $$O(1)$$ |

`n` は前計算した範囲内で指定し、`k`・`l`・`r` は `std::int64_t` です。負の添字と `n` より大きい添字の二項係数は 0 として扱います。上限 0 にも対応します。

上限を $$N$$、ブロック幅を $$B$$ とすると、構築は $$O(N+N^2/B)$$ 時間、保持空間は $$O(N+N^2/B^2)$$、取得は $$O(B)$$ 時間・$$O(1)$$ 追加空間です。`Mint` の演算を定数時間とし、前計算で使う逆元の計算時間は別に加わります。
