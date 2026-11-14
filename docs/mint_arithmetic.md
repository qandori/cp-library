---
title: "modint998244353 の共通剰余演算"
documentation_of: //components/mint_arithmetic.hpp
library_category: components/math
---

`mint` 用の階乗生成・多項式評価・NTT が共有する内部演算です。通常は [modint998244353](https://qandori.github.io/cp-library/math/mint.hpp.html) を利用します。NTT の変換処理や変換表には依存しません。

法を $$p=998244353$$ とすると、整数 $$a$$ を $$a\cdot2^{32}\bmod p$$ と同じ余りの 32 bit 整数で保持する方式が Montgomery 表現です。この 32 bit 整数を「ワード」、複数の整数をまとめて扱うベクトルの各要素を「レーン」と呼びます。`in`・`out` で通常の余りとの間を変換します。

`mint_detail::Ops` は Montgomery 表現のワードを扱い、`mul`・`add`・`sub`、通常表現との変換 `in`・`out`、定数乗算 `fixed_mul` を提供します。`one` は Montgomery 表現の 1、`mod` は 998244353、`twice` はその 2 倍です。

x86 では `vectorized()` で AVX2 の利用可否を確認でき、`load8`・`store8`・`mul8`・`reduce8`・`add8`・`sub8`・`fixed_mul8`・`fixed_even8` を提供します。AVX2 の関数は対応 CPU から呼び出します。

`reduce8(even, odd)` は、偶数・奇数位置ごとに 4 本の 64 bit レーンへ蓄積した積を、Montgomery 表現の 8 本の 32 bit レーンへ戻す内部部品です。補正項を加えても 64 bit でオーバーフローしない範囲で使い、返されたワードの範囲の調整は呼び出し側で行います。

各演算は $$O(1)$$ 時間・追加空間です。変換表の前計算や動的なメモリ確保は行いません。
