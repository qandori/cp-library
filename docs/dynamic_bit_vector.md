---
title: "Dynamic Bit Vector"
documentation_of: //data_structure/dynamic_bit_vector.hpp
---

```cpp
#include "lib/data_structure/dynamic_bit_vector.hpp"

DynamicBitVector<> bits(std::vector<bool>{0, 1, 0});
bits.insert(1, true);
bits.erase(0);
bits.set(0, false);
auto count = bits.rank1(3);
auto position = bits.select1(0);
```

添字と `select*` の順位は 0 始まりです。`rank1(r)`・`rank0(r)` は $$[0,r)$$ の個数、`rank1(l,r)` は両端の `rank1` の組です。`select1(k)`・`select0(k)` は該当する bit の位置を返し、なければ `-1` です。

`access(i)`・`operator[]` は bit の取得、`count()` は全体の 1 の個数です。`insert(i,b)` は挿入前の `rank1(i)`、`erase(i)` は削除した bit を返します。`access_rank(i)`・`erase_rank(i)`・`set_rank(i,b)` は操作前の `rank1(i)` と bit の組を返します。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。`push_back(b)`・`pop_back()` も利用できます。

要素数を $$n$$ として、構築は $$O(n)$$、挿入は償却 $$O(\log n)$$、削除・更新・順位検索・取得は $$O(\log n)$$ です。サイズ・全体の個数・`clear` は $$O(1)$$、`reserve(N)` は $$O(N)$$、空間は $$O(N)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
