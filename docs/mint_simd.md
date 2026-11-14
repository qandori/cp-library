---
title: "modint998244353 の共通 SIMD 演算"
documentation_of: //components/mint_simd.hpp
library_category: components/math
---

NTT と多変数畳み込みが共有する内部演算です。通常は [modint998244353](https://qandori.github.io/cp-library/math/mint.hpp.html) や各畳み込みライブラリを利用します。

SIMD は複数の値を CPU 命令で同時に計算する方式です。ここでは 32 bit 整数を「ワード」、一度に扱うベクトルの各要素を「レーン」と呼びます。各ワードは、法 $$p=998244353$$ に対して整数 $$a$$ を $$a\cdot2^{32}\bmod p$$ と同じ余りで保持する Montgomery 表現です。

`mint_detail::SimdOps` は静的メンバーだけを持ちます。`U` は 32 bit の符号なしワード型、`mod` は 998244353、`twice` はその 2 倍です。以下の操作と、8 本の 32 bit レーンを持つベクトル型 `I8` は x86 で提供します。既定引数はありません。

| 操作 | 内容・返り値 |
| --- | --- |
| `vectorized()` | AVX2 を利用できる場合に `true` を `bool` で返す |
| `load8(const U* p)` | `p[0]` から `p[7]` を読み、`I8` で返す |
| `store8(U* p, I8 x)` | `x` の 8 要素を `p[0]` から `p[7]` へ書く。返り値は `void` |
| `mul8(I8 a, I8 b)` | 各レーンの値を掛け、Montgomery 表現を保った `I8` で返す |
| `add8(I8 a, I8 b)` | 各レーンの和を法 `twice` で還元し、`I8` で返す |
| `reduce8(I8 even, I8 odd)` | 偶数・奇数位置の積を Montgomery 表現へ戻し、8 レーンの `I8` で返す |

`vectorized()` 以外の操作は、AVX2 を利用できる CPU 上で、AVX2 が有効な関数から呼び出してください。読み書きする領域には 8 ワード分の有効な空間が必要です。ポインタを特別なバイト境界に揃える必要はありません。`load8` は入力を変更しません。重複する値もそのまま扱います。

`mul8`・`add8` の入力ワードは `[0,twice)` とし、結果もこの範囲です。`mul8` は各ワードを `mint::v` と同じ Montgomery 表現として扱います。

`reduce8` の各引数は、4 本の 64 bit レーンへ蓄積した積です。補正項を加えても 64 bit の範囲を超えない値を渡してください。偶数位置を `even`、奇数位置を `odd` から復元します。結果の範囲の調整は呼び出し側で行います。

各操作は $$O(1)$$ 時間・追加空間です。動的なメモリ確保や変換表の生成は行いません。
