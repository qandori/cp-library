---
title: 素数判定の基数表
documentation_of: //components/is_prime_bases.hpp
library_category: components/math
---

[素数判定](is_prime.md)で使う Bradley Berg の基数表です。ここでの基数は、Miller–Rabin 法の累乗による判定に使う底のことです。32 bit 用の 256 要素と 64 bit 用の 16,384 要素を持ち、各要素は `std::uint16_t` です。通常は公開ヘッダーを include してください。

出典は Bradley Berg の [32 bit 用の表](https://techneon.com/download/is.prime.32.base.data)・[64 bit 用の表](https://techneon.com/download/is.prime.64.base.data)です。再配布時はヘッダーに含まれる著作権表示と利用許諾を残してください。

`is_prime_detail::bases32[i]` は `0 <= i < 256`、`bases64[i]` は `0 <= i < 16384` で参照します。要素参照は $$O(1)$$ 時間・追加空間です。2 つの表の保持領域は合計 33280 バイトで、実行時の前計算・動的なメモリ確保はありません。
