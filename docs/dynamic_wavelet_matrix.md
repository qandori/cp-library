---
title: Dynamic Wavelet Matrix
documentation_of: //data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp
redirect_from:
- /data_structure/dynamic_wavelet_matrix.hpp.html
---

```cpp
#include "lib/data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"

DynamicWaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
wm.insert(2, -7);
wm.erase(0);
wm.set(1, 6);
auto value = wm.kth_smallest(0, wm.size(), 2);
auto frequency = wm.count(0, wm.size(), 1);
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の順位は 0 始まりです。`kth_*` の `k` は区間の長さ未満とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `DynamicWaveletMatrix()` / `DynamicWaveletMatrix(a)` | 空／配列から構築 | 空: $$O(B)$$、配列: $$O(Bn)$$ |
| `build(a)` | 配列で再構築 | $$O(Bn)$$ |
| `insert(i,x)` / `erase(i)` / `set(i,x)` | 挿入／削除／代入。`erase` は削除した値を返す | 挿入・代入: 償却 $$O(B\log n)$$、削除: $$O(B\log n)$$ |
| `push_back(x)` / `pop_back()` | 末尾への挿入／削除。`pop_back` は削除した値を返す | 追加: 償却 $$O(B\log n)$$、削除: $$O(B\log n)$$ |
| `access(i)` / `operator[](i)` | 要素の取得 | $$O(B\log n)$$ |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ にある `x` の個数。値域外なら 0 | $$O(B\log n)$$ |
| `select(x,k)` | 0 始まり `k` 番目の `x` の位置。なければ -1 | $$O(B\log n)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の 0 始まり `k` 番目 | $$O(B\log n)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の個数 | $$O(B\log n)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の個数 | $$O(B\log n)$$ |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。なければ `nullopt` | $$O(B\log n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `reserve(n)` / `clear()` | 領域予約／容量を残して空にする | reserve: $$O(BN)$$、clear: $$O(B)$$ |
| `memory_usage()` | 確保済みノード領域のバイト数 | $$O(B)$$ |
| `swap(other)` | 内容を交換 | $$O(B)$$ |

型 `T` は `bool` を除く 64 bit 以下の整数型です。`DynamicWaveletMatrix<T,B>` で値の bit 数を指定でき、既定は `T` の bit 数です。$$1\le B\le\mathrm{sizeof}(T)\cdot8$$ とし、符号付き型の値域は $$[-2^{B-1},2^{B-1})$$、符号なし型は $$[0,2^B)$$ です。格納する値はこの範囲に収めてください。

`build(a)` で再構築できます。`reserve(n)` は要素数の予約、`memory_usage()` は確保済みノード領域のバイト数です。`clear()` は容量を保持して空にします。copy・move・`swap` に対応し、move 元は空の列として再利用できます。

特定の値の個数には `count(l,r,x)` を使います。`x` の経路だけをたどり、`x+1` を作らないので整数型の最大値も渡せます。`range_freq(l,r,lower,upper)` は両端の共通する上位の経路を共有し、`lower >= upper` なら 0 を返します。既定実装では境界の差が1の場合も `count` と同じ経路を使います。

`prev_value(l,r,upper)` は境界を含まず、`next_value(l,r,lower)` は境界を含みます。空区間や条件を満たす値がない場合は `nullopt` です。`select(x,k)` は `x` が値域外の場合や `k` が負の場合も `-1` を返します。

既定の `DynamicBitVector<>` を使う場合、内部は [4値の動的 Wavelet Matrix](https://qandori.github.io/cp-library/components/dynamic_quad_wavelet.hpp.html) です。一段で 2 bit を処理し、段数は $$\lceil B/2\rceil$$ です。奇数 bit 幅では最上位段の有効な値が 0・1 になります。テンプレート引数と公開 API は従来と同じです。

各段は [Dynamic Quad Vector](https://qandori.github.io/cp-library/components/dynamic_quad_vector.hpp.html) で保持します。4値の順位をまとめて取得し、代入では削除と挿入の木の更新を共有します。全要素が同じ桁値の段や、値を変えない代入は専用の経路を使います。`prev_value`・`next_value` は境界に沿って下降し、必要な場合に候補の枝から最大値・最小値を求めます。候補が一要素になった `count`・`select` などでは残りの桁を直接照合します。

各段の葉は最大8192個の桁値を保持します。葉の bit 移動と累積個数の更新をまとめ、クエリ時にたどるノード数を減らします。初期構築では更新用の空きを残し、大きな葉の `select` は累積個数から対象 word を二分探索します。

既定の内部処理は GCC・Clang の x86 で CPU の対応状況を実行時に調べ、AVX-512F／AVX2 と POPCNT を使うノード処理、BMI2 の PDEP を使う word 内の `select` を選びます。対応しない CPU や macOS ARM では通常の整数演算を使います。`-march=native` や呼び出し側への target 指定は不要です。include 前に `DYNAMIC_QUAD_DISABLE_SIMD` を定義すると実行時に選ぶ SIMD・POPCNT・BMI2 経路を、`DYNAMIC_QUAD_DISABLE_AVX512` を定義すると AVX-512 経路を無効にできます。

第3テンプレート引数に既定と異なる `BitVector` を指定した場合は、従来の二分 Wavelet Matrix を使います。独自の `BitVector` のインターフェース要件は変わりません。`move_rank` がない場合は `erase_rank` と `insert` を組み合わせ、`select` で `access_rank` を使えない場合は順位検索の経路を使います。この二分実装の BMI2・AVX2 最適化はコンパイル時の指定に従うので、その場合は実行する CPU に合ったオプションを使ってください。

[比較結果と再現手順](https://github.com/qandori/cp-library/blob/main/test/benchmark/dynamic_wavelet_nim_round2.md)を別に記録しています。

要素数を $$n$$、bit 数を $$B$$ として、構築は $$O(Bn)$$、挿入・代入は償却 $$O(B\log n)$$、削除・クエリは $$O(B\log n)$$ です。サイズ取得は $$O(1)$$、`clear`・`memory_usage`・`swap` は $$O(B)$$、`reserve(N)` は $$O(BN)$$、空間は $$O(BN)$$ です。$$N$$ は過去の最大要素数と予約容量の最大値です。
