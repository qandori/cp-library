---
title: 整数演算の共通部品
documentation_of: //components/integer_arithmetic.hpp
library_category: components/math
---

[整数除算](integer_division.md) と [等比数列の和](geometric_sum.md) が使う内部部品です。通常は各ライブラリの公開 API を使ってください。

全て `integer_arithmetic_detail` 名前空間にあります。

| 型・操作 | 内容 |
| --- | --- |
| `integer<T>` | `bool` 以外の組込み整数型および `__int128_t`・`__uint128_t` なら `true` となる `bool` 定数 |
| `add(a, b)` | 同じ型 `T` の `const T&` を 2 つ受け取り、和を `T` で返す |
| `multiply(a, b)` | 同じ型 `T` の `const T&` を 2 つ受け取り、積を `T` で返す |

`add`・`multiply` は `integer<T>` が `true` の場合、演算結果が `T` に収まらなければ `std::overflow_error` を送出します。それ以外の型はそれぞれ `a + b`・`a * b` を評価し、型自身の演算規則に従います。型の演算が対応していれば `constexpr` で使えます。既定引数はありません。

組込み整数型と 128 bit 整数型では各操作が $$O(1)$$ 時間・追加空間です。他の型では、その型の加算・乗算および返り値の構築に必要な時間・空間を使います。
