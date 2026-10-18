---
title: "Wavelet Matrix"
documentation_of: //data_structure/wavelet_matrix.hpp
---

```cpp
#include "lib/data_structure/wavelet_matrix.hpp"

WaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
auto value = wm.kth_smallest(0, 5, 2);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の $$k$$ は 0 始まりです。

| 操作 | 内容・返り値 |
| --- | --- |
| `WaveletMatrix()` / `WaveletMatrix(a)` | 空／配列から構築 |
| `WaveletMatrix(a,true)` | 和の前処理も済ませて構築 |
| `build(a)` | 配列で再構築。和の前処理は必要時に行う |
| `size()` / `empty()` | 要素数／空判定 |
| `access(i)` / `operator[](i)` | 元の配列の要素を取得 |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ にある `x` の個数 |
| `prepare_count()` | 整数型の `count`・`rank`・`select` 用に値検索を前処理。完了済みなら何もしない |
| `select(x,k)` | 0 始まり `k` 番目の `x` の位置。なければ -1 |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の 0 始まり `k` 番目 |
| `range_freq(l,r,upper)` | `upper` 未満の個数 |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の個数 |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。なければ `nullopt` |
| `prepare_sum()` | 和の前処理。完了済みなら何もしない |
| `range_sum(l,r)` | 区間和。未前処理なら初回に前処理 |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和。未前処理なら初回に前処理 |

和の前処理は必要時に行います。構築時に済ませる場合は `WaveletMatrix<T> wm(a, true)` とします。整数の和は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。`WaveletMatrix<T, Sum>` で和の型を指定できます。

通常の問い合わせは 8 分岐の索引を使います。AVX2・POPCNT に対応する x86 CPU では実行時に高速な経路を選び、それ以外ではスカラー処理を使います。和の前処理後は 4 分岐の索引を共用します。和を使うことが決まっている場合は構築時に `true` を指定すると、索引の作り直しを省けます。

`count`・`rank`・`select` を繰り返す場合は、`wm.prepare_count()` で値検索用のハッシュ表を準備できます。64 bit 以下の整数型に対応し、値の種類が少ない場合や連続している場合は追加の表を作りません。その他の型では何もしません。`build` で前処理は破棄されます。

要素数を $$n$$、異なる値の数を $$\sigma$$ として、整数型の構築は $$O(n \log \sigma)$$、その他の型は $$O(n \log n)$$ 時間です。`access` は $$O(1)$$、`select` は $$O(\log \sigma)$$、その他の問い合わせは $$O(\log \sigma)$$ です。

和の前処理は時間・追加空間ともに $$O(n \log \sigma)$$ で、前処理後の `range_sum` は $$O(1)$$、上下位の和は $$O(\log \sigma)$$ です。和を使う最初の問い合わせには前処理時間が加わります。全体の空間は和を含めて $$O(n \log \sigma)$$ です。比較・算術演算は $$O(1)$$ とします。

`prepare_count` は期待 $$O(\sigma)$$・最悪 $$O(\sigma^2)$$ 時間、$$O(\sigma)$$ 空間です。前処理後の値検索は期待 $$O(1)$$・最悪 $$O(\sigma)$$、出現位置の検索を含む `count`・`rank` は期待 $$O(\log \sigma)$$、`select` は期待 $$O(1)$$ です。
