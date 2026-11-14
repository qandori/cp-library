---
title: Dynamic Quad Vector
documentation_of: //components/dynamic_quad_vector.hpp
library_category: components/data_structure
---

[Dynamic Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/dynamic_wavelet_matrix.hpp.html) の内部部品です。`dynamic_wavelet_detail::QuadVector<Block,Branch>` は 0・1・2・3 の列を保持し、任意位置への挿入・削除・代入と、指定位置より前にある各値の個数の取得を行います。通常は公開クラスの `DynamicWaveletMatrix` を使います。

`Block` は一つの葉に保持する値の個数で、128 以上 32768 以下の 64 の倍数とします。`Branch` は各節点が持つ子の最大数で、4 以上の 2 の冪です。既定は `Block=1024`・`Branch=32` です。`digit` は 0〜3、位置と `select` の順位は 0 始まりです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `QuadVector()` / `QuadVector(a)` / `build(a)` | 空列／`vector<uint8_t>` から構築・再構築 | 空: $$O(1)$$、配列: $$O(n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `uniform_digit()` | 全要素が同じ桁値ならその値、空列・混在なら -1 | $$O(1)$$ |
| `rank(r,digit)` | $$[0,r)$$ にある `digit` の個数 | $$O(\log n)$$ |
| `rank(l,r,digit)` | 両端の `rank` の組 | $$O(\log n)$$ |
| `ranks(r)` / `ranks(l,r)` | 4種類の累積個数の `array<int,4>`／両端の配列の組 | $$O(\log n)$$ |
| `access_rank(i)` | `(rank(i,a[i]),a[i])` を返す | $$O(\log n)$$ |
| `select(digit,k)` | `k` 番目の `digit` の位置。なければ -1 | $$O(\log n)$$ |
| `insert(i,digit)` | 挿入し、挿入前の `rank(i,digit)` を返す | 償却 $$O(\log n)$$ |
| `erase_rank(i)` | 削除し、削除前の順位と桁値の組を返す | $$O(\log n)$$ |
| `set_rank(i,digit)` | 代入し、操作前の旧値の順位・旧値・新値の順位を tuple で返す | $$O(\log n)$$ |
| `move_rank(from,to,digit)` | `from` を削除し、削除後の `to` に挿入。削除前の順位・旧値・挿入直前の新値の順位を返す | 償却 $$O(\log n)$$ |
| `reserve(N)` / `clear()` | 領域予約／容量を残して空にする | reserve: $$O(N)$$、clear: $$O(1)$$ |
| `memory_usage()` / `swap(other)` | 確保済みノード領域のバイト数／内容の交換 | $$O(1)$$ |

`ranks(l,r)` の各配列はそれぞれ $$[0,l)$$・$$[0,r)$$ の個数です。区間内の個数は、後者から前者を引いて求めます。表の更新操作が返す「順位」は、指定位置より前にある同じ値の個数を表します。

include 前の `DYNAMIC_QUAD_DISABLE_SIMD` で CPU 固有の命令を使う高速化を、`DYNAMIC_QUAD_DISABLE_AVX512` で AVX-512 命令による高速化だけを無効にできます。通常はどちらも定義する必要がなく、CPU 固有の命令を全体へ有効にするコンパイルオプションも不要です。

`Block`・`Branch` を固定し、要素数を $$n$$ として、構築は $$O(n)$$、挿入・移動は償却 $$O(\log n)$$、削除・代入・クエリは $$O(\log n)$$ です。空間は $$O(N)$$ で、$$N$$ は過去の最大要素数と予約容量の最大値です。

償却計算量は、1 回ごとの最悪時間ではなく、一連の操作にかかる合計時間を操作回数で割ったときの上界です。
