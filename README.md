# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

[ライブラリ検索・Bundle](https://qandori.github.io/cp-library/)

## ライブラリ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/bitwise_zeta.hpp` | Bitwise Zeta / Möbius Transform | [使い方](docs/bitwise_zeta.md) |
| `components/walsh_hadamard.hpp` | Walsh–Hadamard Transform | [使い方](docs/walsh_hadamard.md) |
| `components/divisor_zeta.hpp` | Divisor Zeta / Möbius Transform | [使い方](docs/divisor_zeta.md) |
| `components/fft.hpp` | FFT | [使い方](docs/fft.md) |
| `components/ntt_mod.hpp` | NTT（法指定） | [使い方](docs/ntt_mod.md) |
| `components/ntt.hpp` | NTT（998244353） | [使い方](docs/ntt.md) |
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
| `data_structure/slope_trick.hpp` | Slope Trick | [使い方](docs/slope_trick.md) |
| `data_structure/disjoint_sparse_table.hpp` | Disjoint Sparse Table | [使い方](docs/disjoint_sparse_table.md) |
| `data_structure/segtree_beats.hpp` | Segtree Beats | [使い方](docs/segtree_beats.md) |
| `data_structure/partially_retroactive_priority_queue.hpp` | Partially Retroactive Priority Queue | [使い方](docs/partially_retroactive_priority_queue.md) |
| `data_structure/retroactive_priority_queue.hpp` | Fully Retroactive Priority Queue | [使い方](docs/retroactive_priority_queue.md) |
| `data_structure/lazy_reversible_treap.hpp` | 遅延伝搬・反転可能な乱択平衡二分木 | [使い方](docs/lazy_reversible_treap.md) |
| `data_structure/dynamic_segtree.hpp` | Dynamic Segtree | [使い方](docs/dynamic_segtree.md) |
| `data_structure/dynamic_connectivity.hpp` | Online Dynamic Connectivity | [使い方](docs/dynamic_connectivity.md) |
| `data_structure/li_chao_tree.hpp` | Li Chao Tree | [使い方](docs/li_chao_tree.md) |
| `data_structure/dynamic_li_chao_tree.hpp` | Dynamic Li Chao Tree | [使い方](docs/dynamic_li_chao_tree.md) |
| `data_structure/interval_set.hpp` | Interval Set（整数の区間集合） | [使い方](docs/interval_set.md) |
| `data_structure/link_cut_tree.hpp` | Link Cut Tree | [使い方](docs/link_cut_tree.md) |
| `data_structure/wavelet_matrix.hpp` | Wavelet Matrix | [使い方](docs/wavelet_matrix.md) |
| `data_structure/dynamic_wavelet_matrix.hpp` | Dynamic Wavelet Matrix | [使い方](docs/dynamic_wavelet_matrix.md) |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find | [使い方](docs/weighted_union_find.md) |
| `graph/general_matching.hpp` | General Matching（一般グラフ最大マッチング） | [使い方](docs/general_matching.md) |
| `graph/hungarian.hpp` | Hungarian Algorithm（割当問題） | [使い方](docs/hungarian.md) |
| `graph/max_flow.hpp` | 最大流・最小カット | [使い方](docs/max_flow.md) |
| `graph/min_cost_flow.hpp` | 最小費用流・費用曲線 | [使い方](docs/min_cost_flow.md) |
| `graph/dijkstra.hpp` | Dijkstra | [使い方](docs/dijkstra.md) |
| `graph/lca.hpp` | LCA | [使い方](docs/lca.md) |
| `graph/hld.hpp` | Heavy-Light Decomposition | [使い方](docs/hld.md) |
| `graph/static_top_tree.hpp` | Static Top Tree・動的な木 DP | [使い方](docs/static_top_tree.md) |
| `graph/centroid_decomposition.hpp` | 重心分解 | [使い方](docs/centroid_decomposition.md) |
| `graph/low_link.hpp` | Low-Link | [使い方](docs/low_link.md) |
| `string/lcs.hpp` | 最長共通部分列 | [使い方](docs/lcs.md) |
| `string/edit_distance.hpp` | 編集距離 | [使い方](docs/edit_distance.md) |
| `string/rolling_hash.hpp` | Rolling Hash | [使い方](docs/rolling_hash.md) |
| `string/run_enumerate.hpp` | Run 列挙 | [使い方](docs/run_enumerate.md) |
| `math/mint.hpp` | modint998244353 | [使い方](docs/mint.md) |
| `math/mod_inv.hpp` | 定数時間 mod inv | [使い方](docs/mod_inv.md) |
| `math/mod_pow.hpp` | 定数時間 mod pow・離散対数 | [使い方](docs/mod_pow.md) |
| `math/mod_sqrt.hpp` | 任意の素数 mod の平方根 | [使い方](docs/mod_sqrt.md) |
| `math/fraction.hpp` | 有理数の四則演算・正確な比較 | [使い方](docs/fraction.md) |
| `math/stern_brocot_tree.hpp` | Stern–Brocot Tree・圧縮経路・祖先・LCA | [使い方](docs/stern_brocot_tree.md) |
| `math/matrix.hpp` | 行列積・行列式・逆行列・連立一次方程式など | [使い方](docs/matrix.md) |
| `math/xor_basis.hpp` | XOR 基底・順序統計・共通部分・復元・ロールバック・区間クエリ | [使い方](docs/xor_basis.md) |
| `math/binomial_prefix_sum.hpp` | 二項係数の prefix sum | [使い方](docs/binomial_prefix_sum.md) |
| `math/comb.hpp` | 階乗・逆階乗と二項係数 | [使い方](docs/comb.md) |
| `math/factorize.hpp` | 素因数分解・素数判定 | [使い方](docs/factorize.md) |
| `math/generalized_floor_sum.hpp` | 一般化 floor sum | [使い方](docs/generalized_floor_sum.md) |
| `math/monoid_floor_sum.hpp` | monoid floor sum | [使い方](docs/monoid_floor_sum.md) |
| `math/min_of_mod_of_linear.hpp` | Mod of Linear の最小値 | [使い方](docs/min_of_mod_of_linear.md) |
| `math/prefix_min_of_mod_of_linear.hpp` | Mod of Linear の prefix minimum の等差数列圧縮 | [使い方](docs/prefix_min_of_mod_of_linear.md) |
| `fps/fps.hpp` | 形式的冪級数 | [使い方](docs/fps.md) |
| `fps/polynomial_gcd.hpp` | 多項式 GCD・拡張 GCD・多項式の逆元 | [使い方](docs/polynomial_gcd.md) |
| `fps/bostan_mori.hpp` | Bostan–Mori | [使い方](docs/bostan_mori.md) |
| `fps/linear_recurrence.hpp` | 線形漸化式の第 k 項 | [使い方](docs/linear_recurrence.md) |
| `fps/compose.hpp` | 形式的冪級数の合成 | [使い方](docs/compose.md) |
| `fps/compositional_inverse.hpp` | 形式的冪級数の合成逆元 | [使い方](docs/compositional_inverse.md) |
| `fps/multipoint_eval.hpp` | 多点評価 | [使い方](docs/multipoint_eval.md) |
| `fps/interpolate.hpp` | 多項式補間 | [使い方](docs/interpolate.md) |
| `fps/taylor_shift.hpp` | Taylor Shift | [使い方](docs/taylor_shift.md) |
| `math/partition_mod_of_linear.hpp` | Mod of Linear の列の等差数列分割 | [使い方](docs/partition_mod_of_linear.md) |
| `data_structure/sparse_table.hpp` | Sparse Table | [使い方](docs/sparse_table.md) |
| `math/is_prime.hpp` | 素数判定 | [使い方](docs/is_prime.md) |
| `math/prime_sieve.hpp` | 素数篩 | [使い方](docs/prime_sieve.md) |

## 補助部品

[補助部品の検索](https://qandori.github.io/cp-library/components/)

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/binomial_prefix_core.hpp` | 二項係数の prefix sum の内部演算 | [使い方](docs/binomial_prefix_core.md) |
| `components/flow_graph.hpp` | フローの残余グラフ | [使い方](docs/flow_graph.md) |
| `components/push_relabel.hpp` | Push-Relabel の内部演算 | [使い方](docs/push_relabel.md) |
| `components/min_cost_circulation.hpp` | 最小費用流の容量スケーリング | [使い方](docs/min_cost_circulation.md) |
| `components/mo_order.hpp` | Mo 法の処理順 | [使い方](docs/mo_order.md) |
| `components/small_vector.hpp` | Small Vector | [使い方](docs/small_vector.md) |
| `components/limb_arithmetic.hpp` | Limb Arithmetic | [使い方](docs/limb_arithmetic.md) |
| `components/fast_io.hpp` | Fast I/O | [使い方](docs/fast_io.md) |
| `components/dynamic_bit_vector.hpp` | Dynamic Bit Vector | [使い方](docs/dynamic_bit_vector.md) |
| `components/quad_bit_vector.hpp` | Quad Bit Vector | [使い方](docs/quad_bit_vector.md) |
| `components/default_init_allocator.hpp` | Default Init Allocator | [使い方](docs/default_init_allocator.md) |
| `components/sequence_match.hpp` | 前方・後方の一致比較と Z algorithm | [使い方](docs/sequence_match.md) |
| `components/montgomery.hpp` | Montgomery 64 | [使い方](docs/montgomery.md) |
| `components/montgomery32.hpp` | Montgomery 32 | [使い方](docs/montgomery32.md) |
| `components/mod_fast_core.hpp` | 定数時間 mod 演算の内部部品 | [使い方](docs/mod_fast_core.md) |
| `components/splitmix64.hpp` | SplitMix64 | [使い方](docs/splitmix64.md) |
| `components/d_ary_heap.hpp` | D-ary Heap | [使い方](docs/d_ary_heap.md) |
| `components/RadixHeap.hpp` | Radix Heap | [使い方](docs/radix_heap.md) |
| `components/smawk.hpp` | SMAWK | [使い方](docs/smawk.md) |
| `components/mod_linear_prefix_min.hpp` | Mod of Linear の prefix minimum の内部走査 | [使い方](docs/mod_linear_prefix_min.md) |
| `components/matrix_arithmetic.hpp` | 行列の内部演算 | [使い方](docs/matrix_arithmetic.md) |
| `components/fps_operations.hpp` | FPS の内部演算 | [使い方](docs/fps_operations.md) |
| `components/fps_product_tree.hpp` | 多点評価・補間の内部木 | [使い方](docs/fps_product_tree.md) |
| `components/xor_basis_core.hpp` | XOR 基底の内部演算 | [使い方](docs/xor_basis_core.md) |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。各ライブラリは単独で利用できます。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## ライセンス

[CC0 1.0 Universal](LICENSE) を採用しています。著作権表示なしで利用・変更・再配布・商用利用できます。
