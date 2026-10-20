---
title: Dynamic Quad Vector
documentation_of: //components/dynamic_quad_vector.hpp
---

[Dynamic Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp.html) の内部部品です。`dynamic_wavelet_detail::QuadVector<Block,Branch>` は 0・1・2・3 の列を保持し、任意位置への挿入・削除・代入と順位検索を行います。通常は公開クラスの `DynamicWaveletMatrix` を使います。

各葉では下位・上位 bit を別々の 64 bit word 列に詰め、word ごとの4種類の累積個数を保持します。4種類の累積個数は同じ word のものを隣接させ、64 bit の加減算で更新します。葉の既定容量は1024個、内部木は既定で32分木です。Dynamic Wavelet Matrix 内では葉の容量を8192個にしています。子の長さと桁値ごとの累積個数を持ち、順位の検索と更新を行います。均一な配列の構築は word をまとめて埋めます。空いた葉・内部ノードの領域は再利用し、全要素が同じ桁値の場合は木の検索を省きます。

構築時は葉に約4分の1の空きを残します。ただし、それによって木が一段深くなる場合は葉数を減らします。挿入で葉が満杯になると半分に分割し、削除で容量の4分の1未満になると隣の葉と結合・再配分します。これにより更新のたびに結合と分割を繰り返すことを避けます。

`Block` は一つの葉に保持する桁値の個数で、128 以上 32768 以下の 64 の倍数とします。`Branch` は 4 以上の 2 の冪です。`digit` は 0〜3、位置と `select` の順位は 0 始まりです。

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

`ranks(l,r)` の各配列はそれぞれ $$[0,l)$$・$$[0,r)$$ の個数です。区間内の個数は、後者から前者を引いて求めます。同じ子に含まれる両端の検索は経路を共有します。`move_rank` は二つの位置の検索と共通する祖先の更新を共有し、葉の分割・結合が必要な場合は通常の削除・挿入を使います。更新時の順位は必要な桁値だけを集計します。大きな葉の `select` は累積個数を二分探索して対象 word を求めます。

GCC・Clang の x86 では、ノード処理と葉の bit 移動・累積個数の更新に AVX-512F + POPCNT／AVX2 + POPCNT、word 内の `select` に BMI2 の PDEP を実行時の判定で使います。POPCNT を使う処理にも対応状況の判定があり、その他の CPU では通常の整数演算を使います。GCC では内部関数を O3 で最適化し、ヘッダー読み込み後は呼び出し側の最適化設定へ戻します。

include 前の `DYNAMIC_QUAD_DISABLE_SIMD` で実行時に選ぶ SIMD・POPCNT・BMI2 経路を、`DYNAMIC_QUAD_DISABLE_AVX512` で AVX-512 経路を無効にできます。CPU 固有の命令を全体へ有効にするコンパイルオプションは不要です。

`Block`・`Branch` を固定し、要素数を $$n$$ として、構築は $$O(n)$$、挿入・移動は償却 $$O(\log n)$$、削除・代入・クエリは $$O(\log n)$$ です。空間は $$O(N)$$ で、$$N$$ は過去の最大要素数と予約容量の最大値です。
