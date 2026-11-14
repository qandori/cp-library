---
title: Bitset Word Operations
documentation_of: //components/bitset_ops.hpp
library_category: components/data_structure
---

64 bit 整数（ワード）の配列に対して、集合演算と立っているビット数の合計を求める内部部品です。`DynamicBitset` の論理演算・個数計算に使います。

```cpp
#include "lib/components/bitset_ops.hpp"
#include <array>
#include <cassert>

int main() {
	using bitset_ops_detail::Operation;
	std::array<std::uint64_t, 2> a{0b1011, 0b1100};
	std::array<std::uint64_t, 2> b{0b0101, 0b1010};
	assert(bitset_ops_detail::count<Operation::identity>(a.data(), nullptr, a.size()) == 5);
	assert(bitset_ops_detail::count<Operation::bit_and>(a.data(), b.data(), a.size()) == 2);
	bitset_ops_detail::apply<Operation::bit_xor>(a.data(), b.data(), a.size());
	assert(a[0] == 0b1110 && a[1] == 0b0110);
}
```

`bitset_ops_detail::Operation` は `identity`・`bit_and`・`bit_or`・`bit_xor`・`and_not` を持ちます。`and_not` は第一引数のビット集合から第二引数の集合を引く演算、すなわち `a & ~b` です。

| 関数 | 引数・返り値 |
| --- | --- |
| `count<Op>(a,b,n)` | `const std::uint64_t* a,b` とワード数 `std::size_t n` を受け取り、各ワードに `Op` を適用した結果の立っているビット数の合計を `std::size_t` で返す |
| `apply<Op>(a,b,n)` | `std::uint64_t* a`、`const std::uint64_t* b` とワード数 `std::size_t n` を受け取り、`a[i]` を演算結果へ置き換える。返り値は `void` |

引数の既定値はありません。`n > 0` の場合は、ポインタがそれぞれ `n` ワードの有効な領域を指す必要があります。`count<identity>` では `b` を読み取らず、`nullptr` を指定できます。`apply` に `identity` は指定できません。`n == 0` のときは配列にアクセスせず、ポインタは `nullptr` でも構いません。`count` は 0 を返し、`apply` は何もしません。

配列の境界はワード単位で、最終ワードの全 64 bit を対象にします。論理的なビット長の端数を無視したい場合は、呼び出し側で余剰ビットを 0 にしてください。ビット数の合計は `std::size_t` に収まる必要があり、オーバーフローの検査は行いません。

`apply` は `a == b` の自己参照に対応します。異なる領域を渡す場合は、二つの `n` ワード領域が重ならないようにしてください。自己 XOR・自己差集合はすべて 0、自己 AND・自己 OR は元の値になります。通常の `std::uint64_t` 配列より厳しいアドレスの配置条件はありません。配列の所有権は変更せず、メモリ確保も行いません。

ワード数を $$n$$ として、時間は $$O(n)$$、追加空間は $$O(1)$$ です。空配列では定数時間です。
