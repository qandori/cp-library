---
title: "Quad Bit Vector"
documentation_of: //components/quad_bit_vector.hpp
redirect_from: /data_structure/quad_bit_vector.hpp.html
library_category: components/data_structure
---

各整数から連続する 2 bit を取り出し、0〜3 の列として個数を調べる部品です。64 要素ごとの区画をブロックと呼びます。

```cpp
#include "lib/components/quad_bit_vector.hpp"

std::vector<std::uint32_t> a = {0, 3, 1, 3};
QuadBitVector bits(a.size());
bits.build(a, 0);
auto count = bits.rank(4, 3);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `QuadBitVector(n = 0)` | 長さを指定して領域を準備 | $$O(n)$$ |
| `build(a,shift)` | 各要素の下から `shift` 番目と次の bit で構築 | $$O(n)$$ |
| `rank(r,digit)` | $$[0,r)$$ の `digit` の個数 | $$O(1)$$ |
| `below(r,digit)` | $$[0,r)$$ の `digit` 未満の個数 | $$O(1)$$ |
| `ranks(r)` | $$[0,r)$$ の 0・1・2・3 の個数を配列で返す | $$O(1)$$ |
| `mask(block,digit)` | 1 ブロック内で `digit` に一致する位置の 64 bit マスク | $$O(1)$$ |

`build(a, shift)` は各要素の下から `shift` 番目と次のビットを取り出します。最下位 bit を 0 番目と数えます。`a.size()` は構築時の長さと一致させ、`shift` は 0〜30 とします。

`rank(r, digit)` は $$[0,r)$$ にある `digit` の個数、`below(r, digit)` は `digit` 未満の個数、`ranks(r)` は 4 種類すべての個数です。`digit` は 0〜3 です。

長さを $$n$$ として、構築は $$O(n)$$、クエリは $$O(1)$$、空間は $$O(n)$$ bit です。
