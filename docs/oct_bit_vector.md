---
title: "Oct Bit Vector"
documentation_of: //components/oct_bit_vector.hpp
---

```cpp
#include "lib/components/oct_bit_vector.hpp"

OctBitVector bits(4);
bits.build(std::vector<std::uint32_t>{0, 7, 1, 7}, 0);
auto count = bits.rank(4, 7);
```

32 要素ごとに 8 種類の累積マスクと累積個数を保持します。`a.size()` は構築時の長さ、`shift` は 0〜30、端点は 0〜`n` とします。

| 操作 | 内容・返り値 |
| --- | --- |
| `OctBitVector(n = 0)` | 長さを指定して領域を準備 |
| `build(a,shift)` | 下から `shift` 番目以降の 3 bit で構築。CPU に応じて処理を選択 |
| `build_scalar(a,shift)` / `build_avx2(a,shift)` | 処理を指定して構築 |
| `rank(r,digit)` | $$[0,r)$$ にある `digit` の個数。`digit` は 0〜7 |
| `below(r,digit)` | $$[0,r)$$ にある `digit` 未満の個数。`digit` は 0〜8 |
| `ranks(r)` | $$[0,r)$$ の 8 種類の個数を配列で返す |
| `route_scalar(l,r,k)` / `route_avx2(l,r,k)` | 区間の 0 始まり `k` 番目の digit と、次の階層に使う個数・順位を返す |
| `select_avx2(l,r,k)` | 区間の 0 始まり `k` 番目の digit だけを返す |
| `has_avx2()` | AVX2・POPCNT を利用できるかを返す |
| `mask(r)` | 下位 `r % 32` bit のマスク |
| `cumulative(low,mid,high,valid)` | 3 個の bit 面から 8 種類の累積マスクを作る |
| `popcounts(x)` / `prefix_counts(block,r)` | AVX2 による内部の並列計数 |

`Route` の `digit` は選択された値、`a`・`b` は元の $$[0,l)$$・$$[0,r)$$ にあるその値の個数、`k` はその値の中での順位です。`route_*`・`select_avx2` には $$0 \le k < r-l$$ を渡します。

`avx2` を含む関数と `popcounts`・`prefix_counts` は x86 専用です。直接使う場合は `has_avx2()` が真であることを確認し、AVX2 対応関数から呼び出します。通常は `build` と Wavelet Matrix を使います。

長さを $$n$$ として、構築は $$O(n)$$、問い合わせは $$O(1)$$、空間は $$O(n)$$ bit です。
