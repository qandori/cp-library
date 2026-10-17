# Linux x86_64 / GCC の点検

2026-10-16 に `d6ef542f834dbf09b335af241503a5a9a5ae7c71` を基準として、公開ヘッダー 65 個の構成、依存先、命令セットの条件分岐、既存の測定資料を点検した。全 API の性能を実測したという意味ではない。変更しなかった実装が最速であるとも主張しない。

AtCoder の [practice の言語設定](https://atcoder.jp/contests/practice/rules) は、確認時点で C++23 / GCC 15.2.0、`-O2 -march=native -ftrivial-auto-var-init=zero` を使用している。以下は同じ GCC 世代と主要な最適化フラグでのローカル測定であり、AtCoder の実機での測定ではない。

## 採用した変更

* `ntt_mod`: 各段の回転因子と Shoup 商を準備し、バタフライ内の `% mod` と逐次的な回転因子更新を除去した。AVX2 では 8 個の異なる回転因子を並列に処理する。法は従来どおり `2 < mod < 2^31` に対応し、CRT に使う `2013265921` も含む。長さ 16 以下は従来の小さいループを使い、作業配列を確保しない。自然順の入出力と逆変換の正規化は維持する。追加領域は O(n)、大きい変換で約 `4n` byte。
* `prime_sieve`: 偶数を除外し、奇数の合成数フラグを byte 配列で管理する。`vector<bool>` の read-modify-write を避ける。フラグの領域は従来の約 `n/8` byte から `n/2` byte に増える。返り値の領域は別途必要。
* `quad_bit_vector`: AVX2 のシフトと movemask で 8 要素ずつ 2 本の bit mask に詰める。末尾の端数と 64 要素未満はスカラー処理にする。rank 用の累積個数とデータ形式は維持し、Wavelet Matrix の構築にも反映される。

AVX2 は関数単位の `target("avx2")` と実行時の CPU 判定で使う。ヘッダー利用者に `#pragma GCC target` や `-mavx2` を要求せず、ARM64 ではスカラー経路を使う。既存の 998244353 専用 NTT の NEON 経路も維持している。

## 参照した AC 実装

API の `problem=...&status=AC&order=%2Btime&limit=1` で確認した先頭の提出。提出時のジャッジ時間と、この資料のローカル時間は直接比較しない。ソースをそのまま取り込んではいない。

| 対象 | 提出 | 比較した手法と採否 |
| --- | --- | --- |
| 任意 mod 畳み込み | [409643 / Aiyiyi](https://judge.yosupo.jp/submission/409643) | AVX2、Shoup 商、radix-4、再帰的な積の融合、遅延剰余。法を小さく制限するカーネルがあり、そのままでは本ライブラリの `mod < 2^31` 契約を満たさない。Shoup 商と AVX2 を採用し、各バタフライの出力を法未満に戻す。 |
| 素数列挙 | [336754 / adamant](https://judge.yosupo.jp/submission/336754) | 2・3・5・7 の wheel、bit 配列、区間処理。大規模列挙に向くが固定表や処理が増える。既存 API の簡潔さを維持し、まず奇数だけの byte 篩を採用した。wheel 版とのローカル速度比較は未実施。 |
| 区間 k 番目 | [403008 / nandhagk](https://judge.yosupo.jp/submission/403008) | radix による値圧縮、AVX2、クエリを先読みするオフラインの階層処理。オンライン問い合わせ用の Wavelet Matrix を保持しない構成なので API 全体の置換には使わない。ビット抽出の並列化を比較対象とした。 |

提出の AC 状態、ソースの SHA-256 は [gcc_audit_reference.json](results/gcc_audit_reference.json) に保存した。再取得用スクリプトは `gcc_audit_reference.py`。

## 測定条件

* WSL2 / Ubuntu / Linux x86_64、Intel Core i7-13700F。AVX2 対応、AVX-512 非対応。
* conda-forge GCC 15.2.0。単一 CPU に affinity を固定し、変更前・変更後を交互に 7 回測る。試行ごとに実行順を反転する。
* 基本フラグは `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -Wno-unused-parameter`。native 版のみ `-march=native -ftrivial-auto-var-init=zero` を追加する。`unused-parameter` の抑制は変更前 Wavelet Matrix の assert 専用引数を含むためで、両版に同じ条件で指定する。
* 配列の生成は計測外。割り当て、変換表の準備、構築、出力の checksum 集計は計測に含む。NTT はコピー・順変換・逆変換、CRT は 1000000007 による畳み込み、Wavelet Matrix は構築と中央要素 1 個の問い合わせを測る。
* RNG seed は `20261016`。すべての試行で変更前後の checksum 一致を検査する。速さの数値は該当するサイズと型に限定する。

中央値の単位は ms。表の時間は反復回数分の合計。小さい NTT は高速化を主張しない。

| 処理 | n | 反復 | native 変更前 | native 変更後 | 倍率 | 汎用 x86 の倍率 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| NTT 順逆変換 | 16 | 10000 | 11.795 | 11.998 | 0.98 | 1.00 |
| NTT 順逆変換 | 1024 | 100 | 3.430 | 1.649 | 2.08 | 2.10 |
| NTT 順逆変換 | 262144 | 4 | 65.731 | 21.565 | 3.05 | 3.09 |
| CRT 畳み込み | 32 | 100 | 1.105 | 0.987 | 1.12 | 1.11 |
| CRT 畳み込み | 4096 | 4 | 6.566 | 2.538 | 2.59 | 2.58 |
| CRT 畳み込み | 131072 | 1 | 77.073 | 26.651 | 2.89 | 2.96 |
| 素数篩 | 1000 | 10000 | 16.141 | 5.967 | 2.70 | 2.89 |
| 素数篩 | 1000000 | 10 | 25.669 | 17.060 | 1.50 | 1.56 |
| 素数篩 | 10000000 | 2 | 53.735 | 41.670 | 1.29 | 1.37 |
| QuadBitVector 構築 | 200000 | 100 | 15.113 | 2.633 | 5.74 | 5.26 |
| Wavelet Matrix 構築 | 200000 | 5 | 29.057 | 23.305 | 1.25 | 1.33 |

```sh
taskset -c 0 python3 test/benchmark/gcc_audit.py --cxx g++ --native \
  --output test/benchmark/results/gcc_audit_native
taskset -c 0 python3 test/benchmark/gcc_audit.py --cxx g++ \
  --output test/benchmark/results/gcc_audit_generic
```

`--base` で比較元を指定できる。`--cases ntt crt` のように指定すると、それ以外の既存の測定行を保存したまま指定した処理だけを再測定する。

* [native の測定環境](results/gcc_audit_native/environment.json) / [全試行](results/gcc_audit_native/samples.csv)
* [汎用 x86 の測定環境](results/gcc_audit_generic/environment.json) / [全試行](results/gcc_audit_generic/samples.csv)

## ヘッダーごとの点検結果

「維持」は今回の点検で採用した変更がないことを表す。依存部品の変更による効果と、その実装自身の追加最適化は区別している。

| ヘッダー | 判断 |
| --- | --- |
| components/bitwise_zeta.hpp | 維持。連続区間の加減算。一般の要素型を維持する。 |
| components/d_ary_heap.hpp | 維持。連続配列と穴埋め式の移動。分岐次数の変更は用途別の測定が必要。 |
| components/default_init_allocator.hpp | 維持。不要な値初期化を避ける部品。 |
| components/divisor_zeta.hpp | 素数篩の変更が既定の素数生成経路に反映。変換ループは維持。 |
| components/dynamic_bit_vector.hpp | 維持。大きい葉、累積個数、複数分岐の構成。 |
| components/fast_io.hpp | 維持。mmap / fread の切り替えと SWAR スキャン。Mac 用の POSIX 分岐あり。 |
| components/fft.hpp | 維持。浮動小数点演算の順序と精度保証を変える最適化は未採用。 |
| components/limb_arithmetic.hpp | 維持。AVX2 の桁上がり処理とスカラー経路が既存。 |
| components/montgomery.hpp | 維持。128 bit 積による任意の奇数法の Montgomery 演算。 |
| components/ntt.hpp | 維持。998244353 専用の AVX2 / NEON / スカラー実装が既存。 |
| components/ntt_mod.hpp | Shoup 商と AVX2 を追加。 |
| components/quad_bit_vector.hpp | AVX2 パッキングを追加。 |
| components/RadixHeap.hpp | 維持。単調キー用の bucket 構造。 |
| components/small_vector.hpp | 維持。小さい配列の埋め込み、trivial 型の一括移動。 |
| components/smawk.hpp | 維持。比較回数が線形の列削減。 |
| components/splitmix64.hpp | 維持。短い整数演算。 |
| components/walsh_hadamard.hpp | 維持。連続区間のバタフライ。一般の要素型を維持する。 |
| convolution/bitwise_convolution.hpp | 維持。zeta / Walsh-Hadamard の呼び出し。 |
| convolution/boolean_convolution.hpp | 素数篩の変更が gcd / lcm 系の内部呼び出しに反映。 |
| convolution/convolution.hpp | 汎用 NTT の変更が整数型・任意 mod 型の選択先に反映。 |
| convolution/convolution_fft.hpp | 維持。同一入力の変換再利用が既存。 |
| convolution/convolution_mod.hpp | 汎用 NTT の変更が CRT の 3 法と 998244353 の経路に反映。 |
| convolution/convolution_ring.hpp | 維持。小さい入力は直接積、大きい入力は Karatsuba。 |
| convolution/cyclic_convolution.hpp | 内部で選ぶ通常畳み込みに応じて NTT の変更が反映。 |
| convolution/gcd_lcm_convolution.hpp | 素数篩の変更が反映。素数列を 3 回の変換で共有済み。 |
| convolution/max_min_convolution.hpp | 維持。専用 FPS の畳み込みを使用し、今回の汎用 NTT は使用しない。 |
| convolution/min_plus_convolution.hpp | 維持。SMAWK と入力の凸性を使う構成。 |
| convolution/multiplicative_convolution.hpp | 内部で選ぶ通常畳み込みに応じて NTT の変更が反映。 |
| convolution/multivariate_convolution.hpp | 998244353 専用経路は維持。他の型で選ぶ通常畳み込みに変更が反映。 |
| convolution/ordered_convolution.hpp | 内部で選ぶ通常畳み込みに応じて NTT の変更が反映。 |
| convolution/subset_convolution.hpp | 維持。rank 方向の範囲を限定した zeta / Mobius。 |
| data_structure/disjoint_sparse_table.hpp | 維持。未使用の末尾を省略し、値初期化を避ける構成。 |
| data_structure/dynamic_connectivity.hpp | 維持。配列ノード、辺のハッシュ、レベルごとの森林。SIMD で直接置換しにくい探索。 |
| data_structure/dynamic_segtree.hpp | 維持。圧縮した分岐と小さい葉。既存の Linux 測定資料あり。 |
| data_structure/dynamic_wavelet_matrix.hpp | 維持。動的 bit vector を使い、QuadBitVector の変更は反映しない。 |
| data_structure/lazy_reversible_treap.hpp | 維持。配列ノード、再利用、可換演算用の省略が既存。 |
| data_structure/li_chao_tree.hpp | 維持。連続座標と 64 bit で十分な場合の経路が既存。 |
| data_structure/link_cut_tree.hpp | 維持。可換演算の逆順集計を省略する構成。 |
| data_structure/partially_retroactive_priority_queue.hpp | 維持。優先度と時刻の配列 treap。既存の測定資料あり。 |
| data_structure/retroactive_priority_queue.hpp | 維持。ブロック要約と遅延した再構築。既存の測定資料あり。 |
| data_structure/segtree_beats.hpp | 維持。連続ノード配置と一様区間の短絡。既存の測定資料あり。 |
| data_structure/slope_trick.hpp | 維持。heap の頂点置換と同時処理が既存。 |
| data_structure/sparse_table.hpp | 維持。必要な長さの各段と未初期化 allocator。 |
| data_structure/wavelet_matrix.hpp | 内部の QuadBitVector の構築を高速化。圧縮・問い合わせ API は維持。 |
| data_structure/weighted_union_find.hpp | 維持。サイズ併合と重み付き経路圧縮。 |
| graph/dijkstra.hpp | 維持。整数重みの RadixHeap と他の重みの priority_queue。 |
| graph/general_matching.hpp | 維持。CSR、短い増加道、成分の上界。既存の Linux 測定資料あり。 |
| graph/hld.hpp | 維持。葉の除去による構築、パス・星の専用経路。既存の Linux 測定資料あり。 |
| graph/hungarian.hpp | 維持。列削減と Monge の判定。既存の Linux 測定資料あり。 |
| graph/lca.hpp | 維持。反復 DFS、前順序の RMQ。 |
| graph/low_link.hpp | 維持。CSR と反復 DFS。 |
| math/bigint.hpp | この監査後に削除。[Boost との比較](bigint.md)。 |
| math/comb.hpp | 維持。前計算後は定数回の積。 |
| math/factorize.hpp | 維持。Montgomery、積をまとめた gcd、平方数判定。 |
| math/fps.hpp | 維持。専用 NTT の AVX2 とスカラー / NEON 経路が既存。 |
| math/generalized_floor_sum.hpp | 維持。低次数の専用式と 64 / 128 bit の除算切り替え。 |
| math/is_prime.hpp | 維持。小さい素数の除去、32 / 64 bit の基底選択。 |
| math/min_of_mod_of_linear.hpp | 維持。小さい法の専用経路とユークリッド的な縮約。 |
| math/mint.hpp | 維持。遅延 Montgomery 表現と固定法用の逆元計算。 |
| math/monoid_floor_sum.hpp | 維持。一般のモノイドに対応する縮約。 |
| math/prime_sieve.hpp | 奇数の byte 篩に変更。 |
| math/sqrt_floor.hpp | 維持。剰余列を等差区間へ分解するアルゴリズム。 |
| string/edit_distance.hpp | 維持。既存の AVX2 / AVX-512 / スカラー切り替え。 |
| string/lcs.hpp | 維持。既存の AVX2 / AVX-512 / スカラー切り替えと文字種の絞り込み。 |
| string/rolling_hash.hpp | 維持。2^61-1 法の積と累乗の共有。 |

## 検証

| 検証 | 結果 |
| --- | --- |
| GCC 15.2 / Clang 21.1.8 | 65 ヘッダーの単体コンパイル、48 単体テストが通過 |
| ASan + UBSan / Clang | 変更箇所と推移的依存先の 9 単体テストが通過 |
| ARM64 / GCC 15.2 + QEMU 6.2 | 同じ 9 単体テストが通過。新しい NTT とパッキングはスカラー経路、既存の専用 NTT は NEON 経路を含む |
| Library Checker / GCC + Clang | 9 問題、2 コンパイラ合計 522 ケースが通過 |
| yukicoder 1307 | 循環畳み込み・順序付き畳み込みの 2 検証ファイルはログイン必須のためケース取得不可。対応する単体テストは通過 |

結果と対象ファイルは [検証記録](results/gcc_audit_validation.json)、各 selection JSON に保存した。NTT 用にスカラーと AVX2 の比較、自然順の DFT 比較を含む既存テスト、大小の法・長さ・境界値の回帰テストを使用した。NTT の追加テストは `test/unit/ntt_mod.cpp`。

変更範囲の単体・sanitizer 検証の再現例:

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ \
  --selection test/benchmark/results/gcc_audit_sanitize_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --sanitize --cxx clang++ \
  --selection test/benchmark/results/gcc_audit_sanitize_selection.json
```

macOS 実機ではこの作業中に実行できていない。`.github/workflows/macos-portability.yml` で Apple Clang のヘッダー単体・単体テストを追加した。既存の依存関係付き fingerprint を使い、成功済みで内容が同じ対象は再実行しない。CI は次回の push / pull request、または手動実行で動作する。今回 commit / push は行っていない。

WSL の自動更新により途中で一度再起動したため、中断前に成功を確認した GCC の対象は再実行せず、残りとその後の変更の依存先を検証した。測定環境は再起動後のものを保存している。
