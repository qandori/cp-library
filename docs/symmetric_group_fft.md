---
title: 対称群 Fourier 変換
documentation_of: //components/symmetric_group_fft.hpp
library_category: components/convolution
---

$$n$$ 要素の全ての順列を集めたものを対称群と呼びます。各順列に対応する係数を並べた配列を、いくつかの正方行列へ変換します。変換後は行列ごとに積を取り、逆変換すると順列上の畳み込みを求められます。合成の規約と入力順は [対称群上の畳み込み](symmetric_group_convolution.md) と共通です。通常は畳み込み関数から使いますが、変換後の行列を保存して繰り返し使うこともできます。

```cpp
#include "lib/components/symmetric_group_fft.hpp"
#include <cassert>

SymmetricGroupFFT<> fft(3);
std::vector<mint> a{1, 2, 3, 4, 5, 6};
auto transformed = fft.forward(a);
assert(fft.inverse(transformed) == a);
for (const auto &block : fft.blocks()) {
    assert(block.dimension > 0);
    assert(block.offset + block.dimension * block.dimension <= fft.size());
}
```

| 操作 | 引数・返り値 |
| --- | --- |
| `SymmetricGroupFFT<Mint = mint>(n = 0)` | 次数 `n` の変換を準備。`0 <= n <= 10`、法は `n` より大きい素数 |
| `degree()` | 次数を `int` で返す |
| `size()` | 配列長 `n!` を `int` で返す |
| `blocks()` | `const std::vector<Block>&` を返す。各 `Block` は `partition: vector<int>`・`dimension: int`・`offset: int` |
| `forward(a)` | 辞書順の係数を Fourier 変換し、`std::vector<Mint>` を返す |
| `inverse(a)` | 変換後の配列から辞書順の係数を復元し、`std::vector<Mint>` を返す |

`forward`・`inverse` は `std::span<const Mint>` に変換できる配列を受け取ります。入力は変更しません。長さが `size()` と異なる場合は `std::invalid_argument` を送出します。構築時の次数が範囲外、または法が `n` 以下の場合も `std::invalid_argument` です。法の素数判定は行いません。`Mint` に必要な演算と動的 modint の条件は [畳み込み](symmetric_group_convolution.md) と同じです。

`partition` は、合計が `n` になる正整数を降順に並べた列（`n` の分割）で、各行列を区別するための情報です。`n=0` では空列です。各行列は 1 行ずつ順に格納し、`a[block.offset + i * block.dimension + j]` が行 `i`・列 `j` の要素です。行・列はともに `0 <= i, j < dimension` です。このように変換後の配列内にまとめて格納した行列をブロックと呼び、その要素数の合計は `size()` です。`blocks()` の参照はオブジェクトが破棄・代入されるまで有効です。

分割 $$\lambda$$ に対応する `forward(a)` の行列を $$A_\lambda$$、`forward(b)` の行列を $$B_\lambda$$ とします。行列積 $$A_\lambda B_\lambda$$ を同じブロック順に並べて `inverse` に渡すと、順列 $$q,r$$ を $$q[r[i]]$$ で合成する規約での畳み込みになります。任意の長さ `n!` の配列について、`forward(inverse(a))`・`inverse(forward(a))` はともに元の配列に戻ります。`n = 0, 1` では変換しても配列は変わりません。

各変換は $$O(n^2n!)$$ 時間・返り値を含めて $$O(n!)$$ 追加空間です。計算手順の保持空間と構築中の空間は $$O(nn!)$$、構築時間は $$O(n^2n!\log(n!))$$ です。`degree()`・`size()`・`blocks()` は $$O(1)$$ 時間・追加空間です。`Mint` の四則演算を $$O(1)$$ とし、割り算に使う逆数の計算は構築時に $$O(n)$$ 回、逆変換時に 1 回行います。`n = 0, 1` は $$O(1)$$ 時間・空間です。
