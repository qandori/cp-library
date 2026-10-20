# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

[ライブラリ検索・Bundle](https://qandori.github.io/cp-library/)

## ライブラリ

[フォルダの分類](docs/library_layout.md)

### 変換・基本部品

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/bitwise_zeta.hpp` | Bitwise Zeta / Möbius Transform | [使い方](docs/bitwise_zeta.md) |
| `components/walsh_hadamard.hpp` | Walsh–Hadamard Transform | [使い方](docs/walsh_hadamard.md) |
| `components/divisor_zeta.hpp` | Divisor Zeta / Möbius Transform | [使い方](docs/divisor_zeta.md) |
| `components/fft.hpp` | FFT | [使い方](docs/fft.md) |
| `components/ntt_mod.hpp` | NTT（法指定） | [使い方](docs/ntt_mod.md) |
| `components/ntt.hpp` | NTT（998244353） | [使い方](docs/ntt.md) |
| `components/predecessor_set.hpp` | Predecessor Set（64 分木） | [使い方](docs/predecessor_set.md) |

### 畳み込み

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `convolution/convolution.hpp` | 通常の畳み込み | [使い方](docs/convolution.md) |
| `convolution/online_convolution.hpp` | オンライン畳み込み | [使い方](docs/online_convolution.md) |
| `convolution/convolution_mod.hpp` | 任意 mod・整数の畳み込み | [使い方](docs/convolution_mod.md) |
| `convolution/convolution_fft.hpp` | 実数・複素数の畳み込み | [使い方](docs/convolution_fft.md) |
| `convolution/convolution_ring.hpp` | 一般の環の畳み込み | [使い方](docs/convolution_ring.md) |
| `convolution/bitwise_convolution.hpp` | AND・OR・XOR 畳み込み | [使い方](docs/bitwise_convolution.md) |
| `convolution/gcd_lcm_convolution.hpp` | GCD-plus・LCM-plus 畳み込み | [使い方](docs/gcd_lcm_convolution.md) |
| `convolution/boolean_convolution.hpp` | AND-OR などの論理畳み込み | [使い方](docs/boolean_convolution.md) |
| `convolution/subset_convolution.hpp` | Subset 畳み込み | [使い方](docs/subset_convolution.md) |
| `convolution/min_plus_convolution.hpp` | 凸・凹 min-plus / max-plus 畳み込み | [使い方](docs/min_plus_convolution.md) |
| `convolution/max_min_convolution.hpp` | max-min・min-max 畳み込み | [使い方](docs/max_min_convolution.md) |
| `convolution/cyclic_convolution.hpp` | 巡回・負巡回畳み込み | [使い方](docs/cyclic_convolution.md) |
| `convolution/ordered_convolution.hpp` | i < j 制約付き畳み込み | [使い方](docs/ordered_convolution.md) |
| `convolution/multiplicative_convolution.hpp` | 素数 mod 上の乗法畳み込み | [使い方](docs/multiplicative_convolution.md) |
| `convolution/multivariate_convolution.hpp` | 多変数の切り捨て畳み込み | [使い方](docs/multivariate_convolution.md) |

### 形式的冪級数

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `fps/fps.hpp` | 形式的冪級数 | [使い方](docs/fps.md) |
| `fps/polynomial_gcd.hpp` | 多項式 GCD・拡張 GCD・多項式の逆元 | [使い方](docs/polynomial_gcd.md) |
| `fps/bostan_mori.hpp` | Bostan–Mori | [使い方](docs/bostan_mori.md) |
| `fps/linear_recurrence.hpp` | 線形漸化式の第 k 項 | [使い方](docs/linear_recurrence.md) |
| `fps/compose.hpp` | 形式的冪級数の合成 | [使い方](docs/compose.md) |
| `fps/power_projection.hpp` | Power Projection | [使い方](docs/power_projection.md) |
| `fps/compositional_inverse.hpp` | 形式的冪級数の合成逆元 | [使い方](docs/compositional_inverse.md) |
| `fps/multipoint_eval.hpp` | 多点評価 | [使い方](docs/multipoint_eval.md) |
| `fps/interpolate.hpp` | 多項式補間 | [使い方](docs/interpolate.md) |
| `fps/taylor_shift.hpp` | Taylor Shift | [使い方](docs/taylor_shift.md) |

### 木

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/permutation_tree.hpp` | Permutation Tree（共通区間分解木） | [使い方](docs/permutation_tree.md) |
| `tree/link_cut_tree.hpp` | Link Cut Tree | [使い方](docs/link_cut_tree.md) |
| `tree/lca.hpp` | LCA | [使い方](docs/lca.md) |
| `tree/auxiliary_tree.hpp` | Auxiliary Tree | [使い方](docs/auxiliary_tree.md) |
| `tree/rerooting.hpp` | 全方位木 DP | [使い方](docs/rerooting.md) |
| `tree/dsu_on_tree.hpp` | DSU on Tree | [使い方](docs/dsu_on_tree.md) |

### 最短経路

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/dijkstra.hpp` | Dijkstra・最短経路の復元 | [使い方](docs/dijkstra.md) |
| `graph/bellman_ford.hpp` | 負辺の最短経路・負閉路の復元 | [使い方](docs/bellman_ford.md) |
| `graph/warshall_floyd.hpp` | ワーシャルフロイド法・全点対最短距離 | [使い方](docs/warshall_floyd.md) |

### フロー

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/flow/max_flow.hpp` | 最大流・最小カット | [使い方](docs/max_flow.md) |
| `graph/flow/min_cost_flow.hpp` | 最小費用流・費用曲線 | [使い方](docs/min_cost_flow.md) |

### マッチング

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/matching/bipartite_matching.hpp` | 二部マッチング・最小頂点被覆・最大独立集合・辺の分類 | [使い方](docs/bipartite_matching.md) |
| `graph/matching/general_matching.hpp` | General Matching（一般グラフ最大マッチング） | [使い方](docs/general_matching.md) |
| `graph/matching/general_weighted_matching.hpp` | General Weighted Matching（一般グラフ最大重みマッチング） | [使い方](docs/general_weighted_matching.md) |
| `graph/matching/hungarian.hpp` | Hungarian Algorithm（割当問題） | [使い方](docs/hungarian.md) |

### 連結性

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/connectivity/dynamic_connectivity.hpp` | Online Dynamic Connectivity | [使い方](docs/dynamic_connectivity.md) |
| `graph/connectivity/offline_dynamic_connectivity.hpp` | Offline Dynamic Connectivity（クエリ先読み） | [使い方](docs/offline_dynamic_connectivity.md) |
| `graph/connectivity/low_link.hpp` | Low-Link | [使い方](docs/low_link.md) |
| `graph/connectivity/two_edge_connected_components.hpp` | 二辺連結成分分解・橋の森 | [使い方](docs/two_edge_connected_components.md) |

### データ構造

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/slope_trick.hpp` | Slope Trick | [使い方](docs/slope_trick.md) |
| `data_structure/lazy_reversible_treap.hpp` | 遅延伝搬・反転可能な乱択平衡二分木 | [使い方](docs/lazy_reversible_treap.md) |
| `data_structure/li_chao_tree.hpp` | Li Chao Tree (クエリ先読み) | [使い方](docs/li_chao_tree.md) |
| `data_structure/dynamic_li_chao_tree.hpp` | Dynamic Li Chao Tree | [使い方](docs/dynamic_li_chao_tree.md) |
| `data_structure/interval_set.hpp` | Interval Set（整数の区間集合） | [使い方](docs/interval_set.md) |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find | [使い方](docs/weighted_union_find.md) |
| `data_structure/rollback_union_find.hpp` | Rollback Union Find | [使い方](docs/rollback_union_find.md) |

### セグメント木

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/segment_tree/dual_segtree.hpp` | 双対セグメント木（区間更新・一点取得） | [使い方](docs/dual_segtree.md) |
| `data_structure/segment_tree/dynamic_segtree.hpp` | Dynamic Segtree | [使い方](docs/dynamic_segtree.md) |
| `data_structure/segment_tree/dynamic_lazy_segtree.hpp` | Dynamic Lazy Segtree | [使い方](docs/dynamic_lazy_segtree.md) |
| `data_structure/segment_tree/sortable_segtree.hpp` | Sortable Segtree（区間ソート・区間集約） | [使い方](docs/sortable_segtree.md) |

### Wavelet Matrix

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/wavelet_matrix/wavelet_matrix.hpp` | Wavelet Matrix | [使い方](docs/wavelet_matrix.md) |
| `data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp` | 重み更新 Wavelet Matrix | [使い方](docs/weighted_wavelet_matrix.md) |
| `data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp` | 一点代入 Wavelet Matrix (クエリ先読み) | [使い方](docs/point_set_wavelet_matrix.md) |
| `data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp` | Dynamic Wavelet Matrix | [使い方](docs/dynamic_wavelet_matrix.md) |

### 静的な区間クエリ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/range_query/disjoint_sparse_table.hpp` | Disjoint Sparse Table | [使い方](docs/disjoint_sparse_table.md) |
| `data_structure/range_query/sparse_table.hpp` | Sparse Table | [使い方](docs/sparse_table.md) |

### 列アルゴリズム

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `sequence/lis.hpp` | 最長増加部分列（長さ・復元） | [使い方](docs/lis.md) |

### 列・文字列

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/lcs.hpp` | 最長共通部分列 | [使い方](docs/lcs.md) |
| `string/edit_distance.hpp` | 編集距離 | [使い方](docs/edit_distance.md) |
| `string/rolling_hash.hpp` | Rolling Hash | [使い方](docs/rolling_hash.md) |
| `string/run_enumerate.hpp` | Run 列挙 | [使い方](docs/run_enumerate.md) |
| `string/eertree.hpp` | Eertree（回文木）・回文の出現回数 | [使い方](docs/eertree.md) |
| `string/aho_corasick.hpp` | Aho–Corasick・複数パターン検索 | [使い方](docs/aho_corasick.md) |
| `string/suffix_tree.hpp` | Suffix Tree（静的文字列） | [使い方](docs/suffix_tree.md) |
| `string/dynamic_pattern_matching.hpp` | Dynamic Pattern Matching（固定本文） | [使い方](docs/dynamic_pattern_matching.md) |

### 数値型

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/mint.hpp` | modint998244353 | [使い方](docs/mint.md) |
| `math/fraction.hpp` | 有理数の四則演算・正確な比較 | [使い方](docs/fraction.md) |

### 整数論

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/number_theory/mod_inv.hpp` | 定数時間 mod inv | [使い方](docs/mod_inv.md) |
| `math/number_theory/mod_pow.hpp` | 定数時間 mod pow・離散対数 | [使い方](docs/mod_pow.md) |
| `math/number_theory/mod_sqrt.hpp` | 任意の素数 mod の平方根 | [使い方](docs/mod_sqrt.md) |
| `math/number_theory/kth_root.hpp` | 64 bit 整数の厳密な k 乗根 | [使い方](docs/kth_root.md) |
| `math/number_theory/discrete_log.hpp` | 合成数 mod にも対応する離散対数 | [使い方](docs/discrete_log.md) |
| `math/number_theory/stern_brocot_tree.hpp` | Stern–Brocot Tree・圧縮経路・祖先・LCA | [使い方](docs/stern_brocot_tree.md) |
| `math/number_theory/factorize.hpp` | 素因数分解・素数判定・約数列挙 | [使い方](docs/factorize.md) |
| `math/number_theory/is_prime.hpp` | 素数判定 | [使い方](docs/is_prime.md) |
| `math/number_theory/prime_sieve.hpp` | 素数篩 | [使い方](docs/prime_sieve.md) |

### 組合せ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/combinatorics/binomial_prefix_sum.hpp` | 二項係数の prefix sum | [使い方](docs/binomial_prefix_sum.md) |
| `math/combinatorics/binomial_prefix_sum_incremental.hpp` | 二項係数の prefix sum（差分更新） | [使い方](docs/binomial_prefix_sum_incremental.md) |
| `math/combinatorics/comb.hpp` | 階乗・逆階乗と二項係数 | [使い方](docs/comb.md) |

### Floor Sum

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/floor_sum/generalized_floor_sum.hpp` | 一般化 floor sum | [使い方](docs/generalized_floor_sum.md) |
| `math/floor_sum/monoid_floor_sum.hpp` | monoid floor sum | [使い方](docs/monoid_floor_sum.md) |
| `math/floor_sum/min_of_mod_of_linear.hpp` | Mod of Linear の最小値 | [使い方](docs/min_of_mod_of_linear.md) |
| `math/floor_sum/prefix_min_of_mod_of_linear.hpp` | Mod of Linear の prefix minimum の等差数列圧縮 | [使い方](docs/prefix_min_of_mod_of_linear.md) |
| `math/floor_sum/partition_mod_of_linear.hpp` | Mod of Linear の列の等差数列分割 | [使い方](docs/partition_mod_of_linear.md) |

### 線形代数

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/linear_algebra/matrix.hpp` | 行列積・行列式・逆行列・連立一次方程式など | [使い方](docs/matrix.md) |
| `math/linear_algebra/xor_basis.hpp` | XOR 基底・順序統計・共通部分・復元・ロールバック・区間クエリ | [使い方](docs/xor_basis.md) |

## 補助部品

[補助部品の検索](https://qandori.github.io/cp-library/components/)

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/barrett32.hpp` | 32 bit Barrett 演算 | [使い方](docs/barrett32.md) |
| `components/binary_search.hpp` | 単調な述語の境界探索 | [使い方](docs/binary_search.md) |
| `components/binomial_prefix_core.hpp` | 二項係数の prefix sum の内部演算 | [使い方](docs/binomial_prefix_core.md) |
| `components/binomial_prefix_polynomial.hpp` | 二項係数の prefix sum の多項式評価 | [使い方](docs/binomial_prefix_polynomial.md) |
| `components/flow_graph.hpp` | フローの残余グラフ | [使い方](docs/flow_graph.md) |
| `components/undirected_csr.hpp` | 無向グラフの CSR | [使い方](docs/undirected_csr.md) |
| `components/push_relabel.hpp` | Push-Relabel の内部演算 | [使い方](docs/push_relabel.md) |
| `components/min_cost_circulation.hpp` | 最小費用流の容量スケーリング | [使い方](docs/min_cost_circulation.md) |
| `components/mo_order.hpp` | Mo 法の処理順 (クエリ先読み) | [使い方](docs/mo_order.md) |
| `components/small_vector.hpp` | Small Vector | [使い方](docs/small_vector.md) |
| `components/limb_arithmetic.hpp` | Limb Arithmetic | [使い方](docs/limb_arithmetic.md) |
| `components/fast_io.hpp` | Fast I/O | [使い方](docs/fast_io.md) |
| `components/fast_io_bulk.hpp` | 非負整数の一括入出力 | [使い方](docs/fast_io_bulk.md) |
| `components/dynamic_bit_vector.hpp` | Dynamic Bit Vector | [使い方](docs/dynamic_bit_vector.md) |
| `components/dynamic_quad_vector.hpp` | Dynamic Quad Vector | [使い方](docs/dynamic_quad_vector.md) |
| `components/dynamic_quad_wavelet.hpp` | Dynamic Quad Wavelet Matrix | [使い方](docs/dynamic_quad_wavelet.md) |
| `components/quad_bit_vector.hpp` | Quad Bit Vector | [使い方](docs/quad_bit_vector.md) |
| `components/oct_bit_vector.hpp` | Oct Bit Vector | [使い方](docs/oct_bit_vector.md) |
| `components/fenwick_sum.hpp` | Fenwick Sum | [使い方](docs/fenwick_sum.md) |
| `components/default_init_allocator.hpp` | Default Init Allocator | [使い方](docs/default_init_allocator.md) |
| `components/sequence_match.hpp` | 前方・後方の一致比較と Z algorithm | [使い方](docs/sequence_match.md) |
| `components/sequence_compare.hpp` | 共通接頭辞・接尾辞の比較 | [使い方](docs/sequence_compare.md) |
| `components/suffix_array.hpp` | SA-IS による接尾辞配列の構築 | [使い方](docs/suffix_array.md) |
| `components/static_range_min.hpp` | ブロック分割による静的 RMQ | [使い方](docs/static_range_min.md) |
| `components/static_top_tree_structure.hpp` | Static Top Tree の構造 | [使い方](docs/static_top_tree_structure.md) |
| `components/stern_brocot_path.hpp` | Stern–Brocot Tree の経路演算 | [使い方](docs/stern_brocot_path.md) |
| `components/polynomial_half_gcd.hpp` | 多項式 Half-GCD | [使い方](docs/polynomial_half_gcd.md) |
| `components/weighted_blossom.hpp` | 最大重みマッチングの内部部品 | [使い方](docs/weighted_blossom.md) |
| `components/bipartite_matching_core.hpp` | 二部マッチングの内部演算 | [使い方](docs/bipartite_matching_core.md) |
| `components/offline_time_segments.hpp` | オフラインの時間区間処理 | [使い方](docs/offline_time_segments.md) |
| `components/radix_sort.hpp` | 64 bit キーの安定基数ソート | [使い方](docs/radix_sort.md) |
| `components/montgomery.hpp` | Montgomery 64 | [使い方](docs/montgomery.md) |
| `components/montgomery32.hpp` | Montgomery 32 | [使い方](docs/montgomery32.md) |
| `components/mod_fast_core.hpp` | 定数時間 mod 演算の内部部品 | [使い方](docs/mod_fast_core.md) |
| `components/splitmix64.hpp` | SplitMix64 | [使い方](docs/splitmix64.md) |
| `components/d_ary_heap.hpp` | D-ary Heap | [使い方](docs/d_ary_heap.md) |
| `components/RadixHeap.hpp` | Radix Heap | [使い方](docs/radix_heap.md) |
| `components/dijkstra_queue.hpp` | Dijkstra の内部キュー | [使い方](docs/dijkstra_queue.md) |
| `components/dijkstra_topological.hpp` | Dijkstra のトポロジカル前処理 | [使い方](docs/dijkstra_topological.md) |
| `components/dijkstra_unique.hpp` | Dijkstra の一意経路の探索 | [使い方](docs/dijkstra_unique.md) |
| `components/warshall_floyd_kernel.hpp` | ワーシャルフロイド法のブロック・SIMD 演算 | [使い方](docs/warshall_floyd_kernel.md) |
| `components/shortest_path_scc.hpp` | 最短路で使う到達可能部分の強連結成分分解 | [使い方](docs/shortest_path_scc.md) |
| `components/directed_csr.hpp` | 重み付き有向グラフの CSR | [使い方](docs/directed_csr.md) |
| `components/smawk.hpp` | SMAWK | [使い方](docs/smawk.md) |
| `components/monotone_minima.hpp` | Monotone Minima | [使い方](docs/monotone_minima.md) |
| `components/mod_linear_prefix_min.hpp` | Mod of Linear の prefix minimum の内部走査 | [使い方](docs/mod_linear_prefix_min.md) |
| `components/matrix_arithmetic.hpp` | 行列の内部演算 | [使い方](docs/matrix_arithmetic.md) |
| `components/fps_operations.hpp` | FPS の内部演算 | [使い方](docs/fps_operations.md) |
| `components/fps_factorials.hpp` | 階乗・逆階乗生成 | [使い方](docs/fps_factorials.md) |
| `components/fps_workspace.hpp` | FPS の作業領域 | [使い方](docs/fps_workspace.md) |
| `components/fps_log.hpp` | FPS の対数の内部変換 | [使い方](docs/fps_log.md) |
| `components/fps_asm.hpp` | FPS の命令文字列 | [使い方](docs/fps_asm.md) |
| `components/fps_blocked.hpp` | FPS のブロック変換 | [使い方](docs/fps_blocked.md) |
| `components/fps_bostan.hpp` | Bostan–Mori の倍長化 | [使い方](docs/fps_bostan.md) |
| `components/fps_horner.hpp` | FPS の一点評価 | [使い方](docs/fps_horner.md) |
| `components/polynomial_gcd_blocked.hpp` | 多項式 Half-GCD の専用変換 | [使い方](docs/polynomial_gcd_blocked.md) |
| `components/fps_composition.hpp` | FPS の合成・射影の内部処理 | [使い方](docs/fps_composition.md) |
| `components/fps_convolution.hpp` | FPS の融合畳み込み | [使い方](docs/fps_convolution.md) |
| `components/fps_convolution_unbalanced.hpp` | FPS の非対称畳み込み | [使い方](docs/fps_convolution_unbalanced.md) |
| `components/fps_convolution_constant.hpp` | FPS の定数係数畳み込み | [使い方](docs/fps_convolution_constant.md) |
| `components/fps_direct_eval.hpp` | 多点評価の内部直接計算 | [使い方](docs/fps_direct_eval.md) |
| `components/fps_division_blocked.hpp` | FPS と多項式除算の内部変換 | [使い方](docs/fps_division_blocked.md) |
| `components/fps_evaluation.hpp` | 多点評価・補間の SIMD 木 | [使い方](docs/fps_evaluation.md) |
| `components/fps_exp_blocked.hpp` | FPS の指数・冪の内部変換 | [使い方](docs/fps_exp_blocked.md) |
| `components/fps_inverse_blocked.hpp` | FPS 逆数のブロック変換 | [使い方](docs/fps_inverse_blocked.md) |
| `components/fps_output.hpp` | FPS の結果配列への一括変換 | [使い方](docs/fps_output.md) |
| `components/fps_reversion.hpp` | FPS の合成逆元の内部処理 | [使い方](docs/fps_reversion.md) |
| `components/fps_sqrt_blocked.hpp` | FPS 平方根のブロック変換 | [使い方](docs/fps_sqrt_blocked.md) |
| `components/fps_taylor_blocked.hpp` | Taylor Shift の中間積 | [使い方](docs/fps_taylor_blocked.md) |
| `components/fps_io.hpp` | FPS の一括入出力 | [使い方](docs/fps_io.md) |
| `components/fps_product_tree.hpp` | 多点評価・補間の内部木 | [使い方](docs/fps_product_tree.md) |
| `components/xor_basis_core.hpp` | XOR 基底の内部演算 | [使い方](docs/xor_basis_core.md) |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。各ライブラリは単独で利用できます。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

| 提出用ツール | 説明 |
| --- | --- |
| [tools/compact_fps.py](tools/compact_fps.py) | FPS の結合ソースの短縮（[使い方](docs/fps.md)） |

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## ライセンス

[CC0 1.0 Universal](LICENSE) を採用しています。著作権表示なしで利用・変更・再配布・商用利用できます。

### 応用ライブラリ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/advanced/hld.hpp` | Heavy-Light Decomposition | [使い方](docs/hld.md) |
| `data_structure/advanced/static_top_tree.hpp` | Static Top Tree・動的な木 DP | [使い方](docs/static_top_tree.md) |
| `data_structure/advanced/centroid_decomposition.hpp` | 重心分解 | [使い方](docs/centroid_decomposition.md) |
| `data_structure/advanced/segtree_beats.hpp` | Segtree Beats | [使い方](docs/segtree_beats.md) |
| `data_structure/advanced/kinetic_tournament.hpp` | Kinetic Tournament (固定係数昇順・更新係数非負) | [使い方](docs/kinetic_tournament.md) |
| `data_structure/advanced/generic_segtree_beats.hpp` | 汎用 Segtree Beats | [使い方](docs/generic_segtree_beats.md) |
| `data_structure/advanced/partially_retroactive_priority_queue.hpp` | Partially Retroactive Priority Queue | [使い方](docs/partially_retroactive_priority_queue.md) |
| `data_structure/advanced/retroactive_priority_queue.hpp` | Fully Retroactive Priority Queue | [使い方](docs/retroactive_priority_queue.md) |
