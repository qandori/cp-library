---
title: "Oct Bit Vector"
documentation_of: //components/oct_bit_vector.hpp
library_category: components/data_structure
---

```cpp
#include "lib/components/oct_bit_vector.hpp"

OctBitVector bits(4);
bits.build(std::vector<std::uint32_t>{0, 7, 1, 7}, 0);
auto count = bits.rank(4, 7);
```

各整数から連続する 3 bit を取り出し、0〜7 の列として個数や順位を調べる部品です。`a.size()` は構築時の長さ、`shift` は 0〜30、端点は 0〜`n` とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `OctBitVector(n = 0)` | 長さを指定して領域を準備 | $$O(n)$$ |
| `build(a,shift)` | 下から `shift` 番目以降の 3 bit で構築。CPU に応じて処理を選択 | $$O(n)$$ |
| `build_scalar(a,shift)` / `build_avx2(a,shift)` / `build_avx512(a,shift)` | 処理を指定して構築 | $$O(n)$$ |
| `rank(r,digit)` | $$[0,r)$$ にある `digit` の個数。`digit` は 0〜7 | $$O(1)$$ |
| `below(r,digit)` | $$[0,r)$$ にある `digit` 未満の個数。`digit` は 0〜8 | $$O(1)$$ |
| `ranks(r)` | $$[0,r)$$ の 8 種類の個数を配列で返す | $$O(1)$$ |
| `route_scalar(l,r,k)` / `route_avx2(l,r,k)` / `route_avx512(l,r,k)` | 区間の 0 始まり `k` 番目の digit と、次の階層に使う個数・順位を返す | $$O(1)$$ |
| `select_avx2(l,r,k)` / `select_avx512(l,r,k)` | 区間の 0 始まり `k` 番目の digit だけを返す | $$O(1)$$ |
| `has_avx2()` | AVX2・POPCNT を利用できるかを返す | $$O(1)$$ |
| `has_avx512()` | AVX2・POPCNT と AVX512F・VL・VPOPCNTDQ を利用できるかを返す | $$O(1)$$ |
| `mask(r)` | 下位 `r % 32` bit のマスク | $$O(1)$$ |
| `cumulative(low,mid,high,valid)` | 各要素の下位・中央・上位 bit をそれぞれ集めた 3 個の整数と、有効位置のマスク `valid` から、値が `d` 以下である位置に bit が立つマスクを `d=0,...,7` について作る | $$O(1)$$ |
| `popcounts(x)` / `prefix_counts(block,r)` / `prefix_counts_avx512(block,r)` | SIMD による内部の並列計数 | $$O(1)$$ |

`Route` の `digit` は選択された値、`a`・`b` は元の $$[0,l)$$・$$[0,r)$$ にあるその値の個数、`k` はその値の中での順位です。`route_*`・`select_*` は `[l,r)` を昇順に並べたときの 0 始まり `k` 番目を調べ、$$0 \le k < r-l$$ を渡します。

複数の値を同時に計算する CPU 命令（SIMD）を指定する関数と `popcounts`・`prefix_counts` は x86 専用です。直接使う場合は対応する `has_avx2()`・`has_avx512()` を確認し、同じ命令セットに対応する関数から呼び出します。通常は `build` と Wavelet Matrix を使います。

長さを $$n$$ として、構築は $$O(n)$$、クエリは $$O(1)$$、空間は $$O(n)$$ bit です。
