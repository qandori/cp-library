---
title: 素数判定の共通演算
documentation_of: //components/is_prime_core.hpp
library_category: components/math
---

[素数判定](is_prime.md)と[軽量な素数判定](is_prime_light.md)で使う Miller–Rabin 法の内部部品です。通常は公開ヘッダーを include してください。

`is_prime_detail::prime(m)` は、`Montgomery32` または `Montgomery64` の演算器 `m` が持つ法を判定し、素数なら `true` を返します。この関数へ直接渡す法は 3 以上の奇数にしてください。`check(n,prime)` は `std::uint64_t n` の小さい値と小さい素数の倍数を先に処理し、必要な場合だけ判定用の関数 `prime(m)` を呼びます。公開の素数判定は、この組み合わせにより 0・1・偶数を含む全範囲を扱います。

`witness`・`witness2`・`witnesses` は、奇数の法と、その法から作った値を使う補助処理です。整数を `m.in(a)` で変換した値を Montgomery 表現と呼びます。詳しい表現は [32 bit 版](montgomery32.md)・[64 bit 版](montgomery.md) を参照してください。法を $$n$$、$$n-1=2^sd$$ として、`d` は正の奇数、`s >= 1` とします。`witness(m,x,s,multiply)` の `x` は、判定する基数の $$d$$ 乗をこの表現にした値です。`witness2` は基数 2、`witnesses` は渡した配列の各基数を調べます。`witness2` の法は $$n<2^{62}$$ に限ります。いずれも、この基数では合成数と判定できなければ `true` を返します。乗算用に渡す関数は、同じ Montgomery 表現の積を返すものとします。

整数の乗算・剰余を定数時間として、`prime` は $$O(\log n)$$ 時間・$$O(1)$$ 追加空間です。基数の個数を $$k\ge1$$ とすると、`witness` は $$O(s)$$、`witness2` は $$O(\log d+s)$$、`witnesses` は $$O(k(\log d+s))$$ 時間です。前二者の追加空間は $$O(1)$$、`witnesses` は $$O(k)$$ です。`check` 自身の処理は $$O(1)$$ 時間・追加空間で、判定用の関数の時間・空間が別途加わります。
