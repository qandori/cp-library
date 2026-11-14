---
title: XOR 基底の内部演算
documentation_of: //components/xor_basis_core.hpp
library_category: components/data_structure
---

`math/xor_basis.hpp` が使う固定サイズの内部表現です。通常は公開側の [XorBasis](xor_basis.md) を使います。XOR 基底は、元の整数列と同じ部分集合 XOR を作れるように選んだ整数の組です。各基底要素は、他の基底要素の XOR では作れません。

| 操作・フィールド | 内容 | 計算量 |
| --- | --- | --- |
| `xor_basis_detail::Core<UInt,Bits>` | 符号なし整数の XOR 基底 | $$O(Bits)$$ |
| `row` / `dimension` | 先頭 `dimension` 個に詰めた基底／基底要素の個数 | $$O(1)$$ |
| `at` | 各最上位ビットの基底位置に 1 を加えた値。0 は未使用 | $$O(1)$$ |
| `digits` / `mask` | 要素型のビット数／下位 `Bits` bit のマスク | $$O(1)$$ |
| `valid(x)` | `x` が下位 `Bits` bit に収まるか | $$O(1)$$ |
| `pivot(x)` | 最上位の 1 の位置。`x` は非零とする | $$O(1)$$ |
| `insert(x)` | 基底を追加し、格納位置を返す。既存の基底の部分集合 XOR で作れる値なら -1 | $$O(r)$$ |
| `minimize(x)` / `maximize(x)` | 部分集合の XOR と `x` の XOR の最小値／最大値 | $$O(r)$$ |
| `xor_basis_detail::reduce_rows<UInt,maximum>(x,row,count)` | 消去順の基底に沿って最小化する。`maximum=true` なら最大化 | $$O(\mathrm{count})$$ |

基底は格納順に消去できる形で保持します。行の直接更新は公開ライブラリの内部からだけ行い、`at`・`dimension` も同時に更新します。

構築・初期化は $$O(\mathrm{Bits})$$ 時間、保持空間は $$O(\mathrm{Bits})$$ です。追加・最小最大は基底要素の個数を $$r$$ として $$O(r)$$ 時間、個数が `Bits` と等しい場合は $$O(1)$$ 時間です。`valid`・`pivot` は $$O(1)$$ 時間です。各操作の追加空間は $$O(1)$$ です。
