# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

| サイト | 内容 |
| --- | --- |
| [全体（256 KiB向け）](https://qandori.github.io/cp-library/) | 高速実装と軽量実装を掲載 |
| [軽量版（64 KiB）](https://qandori.github.io/cp-library/lightweight/) | 同じ分類・項目順で64 KiB以内の実装を案内。同じ実装は共通ページを共有 |

[サイズ制限と提出方法](docs/source_size.md)

[提出コードの生成とサイズ上限](docs/bundle.md)

[利用者テンプレートとの併用](docs/template_compatibility.md)

## ライブラリ

### components

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/bitwise_zeta.hpp` | Bitwise Zeta / Möbius Transform | [使い方](docs/bitwise_zeta.md) |
| `components/walsh_hadamard.hpp` | Walsh–Hadamard Transform | [使い方](docs/walsh_hadamard.md) |
| `components/divisor_zeta.hpp` | Divisor Zeta / Möbius Transform | [使い方](docs/divisor_zeta.md) |
| `components/fft.hpp` | FFT | [使い方](docs/fft.md) |
| `components/ntt_mod.hpp` | NTT（法指定） | [使い方](docs/ntt_mod.md) |
| `components/ntt.hpp` | NTT（998244353） | [使い方](docs/ntt.md) |
| `components/fps_light_ntt.hpp` | 軽量 FPS の NTT・畳み込み | [使い方](docs/fps_light_ntt.md) |

### convolution

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `convolution/convolution.hpp` | 通常の畳み込み | [使い方](docs/convolution.md) |
| `convolution/convolution_light.hpp` | 軽量版の通常畳み込み | [使い方](docs/convolution_light.md) |
| `convolution/online_convolution.hpp` | オンライン畳み込み | [使い方](docs/online_convolution.md) |
| `convolution/online_convolution_light.hpp` | 軽量版のオンライン畳み込み | [使い方](docs/online_convolution_light.md) |
| `convolution/convolution_mod.hpp` | 任意 mod・整数の畳み込み | [使い方](docs/convolution_mod.md) |
| `convolution/convolution_fft.hpp` | 実数・複素数の畳み込み | [使い方](docs/convolution_fft.md) |
| `convolution/convolution_ring.hpp` | 加算・減算・乗算を使う型の畳み込み | [使い方](docs/convolution_ring.md) |
| `convolution/bitwise_convolution.hpp` | AND・OR・XOR 畳み込み | [使い方](docs/bitwise_convolution.md) |
| `convolution/gcd_lcm_convolution.hpp` | GCD-plus・LCM-plus 畳み込み | [使い方](docs/gcd_lcm_convolution.md) |
| `convolution/boolean_convolution.hpp` | AND-OR などの論理畳み込み | [使い方](docs/boolean_convolution.md) |
| `convolution/symmetric_group_convolution.hpp` | 順列の合成による畳み込み | [使い方](docs/symmetric_group_convolution.md) |
| `convolution/subset_convolution.hpp` | Subset 畳み込み | [使い方](docs/subset_convolution.md) |
| `convolution/min_plus_convolution.hpp` | 凸・凹 min-plus / max-plus 畳み込み | [使い方](docs/min_plus_convolution.md) |
| `convolution/max_min_convolution.hpp` | max-min・min-max 畳み込み | [使い方](docs/max_min_convolution.md) |
| `convolution/max_min_convolution_light.hpp` | 軽量版の max-min・min-max 畳み込み | [使い方](docs/max_min_convolution_light.md) |
| `convolution/cyclic_convolution.hpp` | 巡回・負巡回畳み込み | [使い方](docs/cyclic_convolution.md) |
| `convolution/cyclic_convolution_light.hpp` | 軽量版の巡回・負巡回畳み込み | [使い方](docs/cyclic_convolution_light.md) |
| `convolution/ordered_convolution.hpp` | i < j 制約付き畳み込み | [使い方](docs/ordered_convolution.md) |
| `convolution/ordered_convolution_light.hpp` | 軽量版の i < j 制約付き畳み込み | [使い方](docs/ordered_convolution_light.md) |
| `convolution/multiplicative_convolution.hpp` | 素数 mod 上の乗法畳み込み | [使い方](docs/multiplicative_convolution.md) |
| `convolution/multiplicative_convolution_light.hpp` | 軽量版の乗法畳み込み | [使い方](docs/multiplicative_convolution_light.md) |
| `convolution/multivariate_convolution.hpp` | 多変数の切り捨て畳み込み | [使い方](docs/multivariate_convolution.md) |
| `convolution/multivariate_convolution_light.hpp` | 軽量版の多変数畳み込み | [使い方](docs/multivariate_convolution_light.md) |

### data_structure

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/slope_trick.hpp` | Slope Trick | [使い方](docs/slope_trick.md) |
| `data_structure/dynamic_bitset.hpp` | Dynamic Bitset・論理演算／シフト・区間更新・ビット列挙 | [使い方](docs/dynamic_bitset.md) |
| `data_structure/double_ended_priority_queue.hpp` | 両端優先度付きキュー（最小値・最大値の取得と削除） | [使い方](docs/double_ended_priority_queue.md) |

### data_structure/associative

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/interval_set.hpp` | Interval Set（整数の区間集合） | [使い方](docs/interval_set.md) |
| `data_structure/interval_map.hpp` | Interval Map（区間への値の代入・削除・検索） | [使い方](docs/interval_map.md) |
| `data_structure/flat_hash_map.hpp` | Flat Hash Map（連想配列） | [使い方](docs/flat_hash_map.md) |
| `data_structure/xor_set.hpp` | XOR の最小値・最大値を求める整数集合 | [使い方](docs/xor_set.md) |
| `data_structure/top_k_sum.hpp` | Top K Sum（大小 k 個の和・k 番目の値・境界検索・重複個数） | [使い方](docs/top_k_sum.md) |

### data_structure/li_chao_tree

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/li_chao_tree.hpp` | Li Chao Tree (クエリ先読み) | [使い方](docs/li_chao_tree.md) |
| `data_structure/dynamic_li_chao_tree.hpp` | Dynamic Li Chao Tree | [使い方](docs/dynamic_li_chao_tree.md) |

### data_structure/persistent

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/persistent_link_cut_tree.hpp` | 永続 Link Cut Tree・過去版からの分岐 | [使い方](docs/persistent_link_cut_tree.md) |
| `data_structure/persistent_array.hpp` | 永続配列 | [使い方](docs/persistent_array.md) |
| `data_structure/persistent_union_find.hpp` | 永続 Union Find | [使い方](docs/persistent_union_find.md) |
| `data_structure/persistent_stack.hpp` | 永続スタック・祖先アクセス | [使い方](docs/persistent_stack.md) |
| `data_structure/persistent_queue.hpp` | 永続キュー | [使い方](docs/persistent_queue.md) |
| `data_structure/persistent_deque.hpp` | 永続 Deque・列の分割連結 | [使い方](docs/persistent_deque.md) |
| `data_structure/persistent_multiset.hpp` | 永続順序付き多重集合・順位取得 | [使い方](docs/persistent_multiset.md) |
| `data_structure/persistent_heap.hpp` | 永続併合可能ヒープ | [使い方](docs/persistent_heap.md) |
| `data_structure/persistent_segtree.hpp` | 永続セグメント木（過去版への更新・区間コピー） | [使い方](docs/persistent_segtree.md) |
| `data_structure/persistent_lazy_segtree.hpp` | 永続遅延セグメント木・区間コピー | [使い方](docs/persistent_lazy_segtree.md) |
| `string/data_dependent_tree.hpp` | Data Dependent Tree・永続文字列・LCP・辞書順比較 | [使い方](docs/data_dependent_tree.md) |

### data_structure/range_query

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/sliding_window_aggregation.hpp` | Sliding Window Aggregation（キューの集約） | [使い方](docs/sliding_window_aggregation.md) |
| `data_structure/fenwick_tree.hpp` | Fenwick Tree（一点加算・区間和） | [使い方](docs/fenwick_tree.md) |
| `data_structure/static_threshold_search.hpp` | 静的整数列の閾値検索・O(log n) | [使い方](docs/static_threshold_search.md) |
| `data_structure/disjoint_sparse_table.hpp` | Disjoint Sparse Table | [使い方](docs/disjoint_sparse_table.md) |
| `data_structure/sparse_table.hpp` | Sparse Table | [使い方](docs/sparse_table.md) |
| `data_structure/parallel_binary_search.hpp` | 並列二分探索・条件が初めて成立する更新回数 | [使い方](docs/parallel_binary_search.md) |
| `data_structure/mo_order.hpp` | Mo's Algorithm | [使い方](docs/mo_order.md) |
| `data_structure/mo_with_updates.hpp` | 更新付き Mo's Algorithm・一点更新と区間クエリ | [使い方](docs/mo_with_updates.md) |
| `data_structure/offline_range_frequency.hpp` | 区間頻度（一点更新・クエリ一括処理） | [使い方](docs/offline_range_frequency.md) |
| `data_structure/offline_range_count_distinct.hpp` | 区間の異なる値の個数（クエリ一括処理） | [使い方](docs/offline_range_count_distinct.md) |
| `data_structure/offline_range_sum.hpp` | 値の上限付き区間和（クエリ一括処理） | [使い方](docs/offline_range_sum.md) |
| `data_structure/offline_range_kth_smallest.hpp` | 区間の k 番目の値（クエリ一括処理） | [使い方](docs/offline_range_kth_smallest.md) |
| `data_structure/range_lis_query.hpp` | Range LIS Query（区間の最長増加部分列） | [使い方](docs/range_lis_query.md) |
| `data_structure/static_range_inversions.hpp` | 静的区間の転倒数 | [使い方](docs/static_range_inversions.md) |

### data_structure/rectangle_query

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/offline_rectangle_sum.hpp` | 静的矩形和（クエリ先読み） | [使い方](docs/offline_rectangle_sum.md) |
| `data_structure/rectangle_add_rectangle_sum.hpp` | 矩形加算・矩形和（更新座標を事前登録） | [使い方](docs/rectangle_add_rectangle_sum.md) |
| `data_structure/static_rectangle_add_rectangle_sum.hpp` | 静的な矩形加算・矩形和 | [使い方](docs/static_rectangle_add_rectangle_sum.md) |
| `data_structure/kd_tree.hpp` | Kd-tree・最近傍／k 近傍・矩形内の点の検索 | [使い方](docs/kd_tree.md) |
| `data_structure/priority_search_tree.hpp` | Priority Search Tree（三辺矩形内の点の列挙・点の更新） | [使い方](docs/priority_search_tree.md) |

### data_structure/segment_tree

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/range_affine_range_sum.hpp` | 区間アフィン変換・区間和 | [使い方](docs/range_affine_range_sum.md) |
| `data_structure/range_affine_point_get.hpp` | 区間アフィン変換・一点取得 | [使い方](docs/range_affine_point_get.md) |
| `data_structure/range_add_range_min.hpp` | 区間加算・区間最小値 | [使い方](docs/range_add_range_min.md) |
| `data_structure/segtree.hpp` | セグメント木・一点更新・区間集約・境界検索 | [使い方](docs/segtree.md) |
| `data_structure/lazy_segtree.hpp` | 遅延セグメント木・境界検索 | [使い方](docs/lazy_segtree.md) |
| `data_structure/segtree_beats.hpp` | Segtree Beats | [使い方](docs/segtree_beats.md) |
| `data_structure/range_chmin_chmax_add_sum.hpp` | Range Chmin Chmax Add Sum | [使い方](docs/range_chmin_chmax_add_sum.md) |
| `data_structure/dual_segtree.hpp` | 双対セグメント木（区間更新・一点取得） | [使い方](docs/dual_segtree.md) |
| `data_structure/dynamic_segtree.hpp` | Dynamic Segtree | [使い方](docs/dynamic_segtree.md) |
| `data_structure/dynamic_lazy_segtree.hpp` | Dynamic Lazy Segtree | [使い方](docs/dynamic_lazy_segtree.md) |
| `data_structure/sortable_segtree.hpp` | Sortable Segtree（区間ソート・区間集約） | [使い方](docs/sortable_segtree.md) |

### data_structure/sequence

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/lazy_reversible_treap.hpp` | 遅延伝搬・反転可能な乱択平衡二分木 | [使い方](docs/lazy_reversible_treap.md) |
| `data_structure/lazy_reversible_sequence.hpp` | 遅延伝搬・反転可能なブロック列 | [使い方](docs/lazy_reversible_sequence.md) |

### data_structure/union_find

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/union_find.hpp` | Union-Find・連結成分の列挙 | [使い方](docs/union_find.md) |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find | [使い方](docs/weighted_union_find.md) |
| `data_structure/rollback_union_find.hpp` | Rollback Union Find | [使い方](docs/rollback_union_find.md) |

### data_structure/wavelet_matrix

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/wavelet_matrix.hpp` | Wavelet Matrix | [使い方](docs/wavelet_matrix.md) |
| `data_structure/weighted_wavelet_matrix.hpp` | 重み更新 Wavelet Matrix | [使い方](docs/weighted_wavelet_matrix.md) |
| `data_structure/point_set_wavelet_matrix.hpp` | 一点代入 Wavelet Matrix (クエリ先読み) | [使い方](docs/point_set_wavelet_matrix.md) |
| `data_structure/dynamic_wavelet_matrix.hpp` | Dynamic Wavelet Matrix | [使い方](docs/dynamic_wavelet_matrix.md) |

### fps

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `fps/fps.hpp` | 形式的冪級数 | [使い方](docs/fps.md) |
| `fps/fps_light.hpp` | 軽量 FPS（64 KiB 制限向け） | [使い方](docs/fps_light.md) |
| `fps/fps_light_algorithms.hpp` | 軽量 FPS の Taylor shift・線形漸化式・多点評価・補間 | [使い方](docs/fps_light_algorithms.md) |
| `fps/polynomial_gcd.hpp` | 多項式 GCD・拡張 GCD・多項式の逆元 | [使い方](docs/polynomial_gcd.md) |
| `fps/polynomial_gcd_light.hpp` | 軽量 FPS の多項式 GCD・拡張 GCD・逆元 | [使い方](docs/polynomial_gcd_light.md) |
| `fps/bostan_mori.hpp` | Bostan–Mori | [使い方](docs/bostan_mori.md) |
| `fps/rational_fps_coefficients.hpp` | 有理 FPS の区間係数取得 | [使い方](docs/rational_fps_coefficients.md) |
| `fps/rational_fps_coefficients_light.hpp` | 軽量 FPS の有理 FPS 区間係数取得 | [使い方](docs/rational_fps_coefficients_light.md) |
| `fps/linear_recurrence.hpp` | 線形漸化式の第 k 項 | [使い方](docs/linear_recurrence.md) |
| `fps/berlekamp_massey.hpp` | 線形漸化式の復元（Berlekamp–Massey） | [使い方](docs/berlekamp_massey.md) |
| `fps/compose.hpp` | 形式的冪級数の合成 | [使い方](docs/compose.md) |
| `fps/compose_light.hpp` | 軽量 FPS の合成 | [使い方](docs/compose_light.md) |
| `fps/power_projection.hpp` | 冪ごとの係数・重み付き和（Power Projection） | [使い方](docs/power_projection.md) |
| `fps/power_projection_light.hpp` | 軽量 FPS の冪ごとの係数・重み付き和 | [使い方](docs/power_projection_light.md) |
| `fps/compositional_inverse.hpp` | 形式的冪級数の合成逆元 | [使い方](docs/compositional_inverse.md) |
| `fps/compositional_inverse_light.hpp` | 軽量 FPS の合成逆元 | [使い方](docs/compositional_inverse_light.md) |
| `fps/multipoint_eval.hpp` | 多点評価 | [使い方](docs/multipoint_eval.md) |
| `fps/interpolate.hpp` | 多項式補間 | [使い方](docs/interpolate.md) |
| `fps/taylor_shift.hpp` | Taylor Shift | [使い方](docs/taylor_shift.md) |

### geometry

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `geometry/geometry.hpp` | 平面幾何の一括 include | [使い方](docs/geometry.md) |
| `geometry/point.hpp` | 点・ベクトル・偏角順 | [使い方](docs/geometry_point.md) |
| `geometry/linear.hpp` | 直線・線分の交差・距離・直線上の最近点 | [使い方](docs/geometry_linear.md) |
| `geometry/polygon.hpp` | 多角形の面積・内外・切断 | [使い方](docs/geometry_polygon.md) |
| `geometry/convex.hpp` | 凸包・直径・最近点対・Minkowski和 | [使い方](docs/geometry_convex.md) |
| `geometry/dynamic_convex_hull.hpp` | 点追加型の動的凸包 | [使い方](docs/dynamic_convex_hull.md) |
| `geometry/circle.hpp` | 円・接線・共通面積・最小包含円 | [使い方](docs/geometry_circle.md) |

### geometry/exact

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `geometry/rational.hpp` | 幾何の任意精度整数・有理数座標 | [使い方](docs/geometry_rational.md) |
| `geometry/exact_linear.hpp` | 誤差のない交点・直線上の最近点・距離二乗 | [使い方](docs/geometry_exact_linear.md) |
| `geometry/exact_polygon.hpp` | 厳密な面積・重心・凸多角形の切断 | [使い方](docs/geometry_exact_polygon.md) |
| `geometry/exact_circle.hpp` | 厳密な円の判定・外接円・最小包含円 | [使い方](docs/geometry_exact_circle.md) |
| `geometry/exact.hpp` | 任意精度による厳密な平面幾何の一括 include | [使い方](docs/geometry_exact.md) |

### graph

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/range_edge_graph.hpp` | 区間辺グラフ・頂点／区間間の有向辺の圧縮 | [使い方](docs/range_edge_graph.md) |
| `graph/topological_sort.hpp` | トポロジカルソート・辞書順最小・閉路判定 | [使い方](docs/topological_sort.md) |
| `graph/functional_graph.hpp` | 閉路と木への分解・移動・到達距離 | [使い方](docs/functional_graph.md) |
| `graph/eulerian_trail.hpp` | 有向・無向 Euler 路（頂点列・辺 ID 列） | [使い方](docs/eulerian_trail.md) |
| `graph/enumerate_triangles.hpp` | 無向単純グラフの三角形列挙 | [使い方](docs/enumerate_triangles.md) |
| `graph/two_sat.hpp` | 2-SAT | [使い方](docs/two_sat.md) |

### graph/connectivity

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/dynamic_connectivity.hpp` | Online Dynamic Connectivity | [使い方](docs/dynamic_connectivity.md) |
| `graph/offline_dynamic_connectivity.hpp` | Offline Dynamic Connectivity（クエリ先読み） | [使い方](docs/offline_dynamic_connectivity.md) |
| `graph/scc.hpp` | 強連結成分分解（反復 SCC） | [使い方](docs/scc.md) |
| `graph/dominator_tree.hpp` | Dominator Tree・直接支配頂点・支配関係 | [使い方](docs/dominator_tree.md) |
| `graph/low_link.hpp` | Low-Link | [使い方](docs/low_link.md) |
| `graph/two_edge_connected_components.hpp` | 二辺連結成分分解・橋の森 | [使い方](docs/two_edge_connected_components.md) |
| `graph/block_cut_tree.hpp` | 二重頂点連結成分・Block Cut Tree | [使い方](docs/block_cut_tree.md) |

### graph/flow

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/max_flow.hpp` | 最大流・最小カット | [使い方](docs/max_flow.md) |
| `graph/min_cost_flow.hpp` | 最小費用流・費用曲線 | [使い方](docs/min_cost_flow.md) |
| `graph/min_cost_b_flow.hpp` | 下限付き最小費用流・頂点ごとの流量指定・流量範囲 | [使い方](docs/min_cost_b_flow.md) |
| `graph/project_selection.hpp` | Project Selection（燃やす埋める） | [使い方](docs/project_selection.md) |

### graph/matching

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/bipartite_graph.hpp` | 二部グラフの自動色分け・最大独立集合・最小頂点被覆 | [使い方](docs/bipartite_graph.md) |
| `graph/bipartite_matching.hpp` | 二部マッチング・最小頂点被覆・最大独立集合・辺の分類 | [使い方](docs/bipartite_matching.md) |
| `graph/general_matching.hpp` | General Matching（一般グラフ最大マッチング） | [使い方](docs/general_matching.md) |
| `graph/general_weighted_matching.hpp` | General Weighted Matching（一般グラフ最大重みマッチング） | [使い方](docs/general_weighted_matching.md) |
| `graph/hungarian.hpp` | Hungarian Algorithm（割当問題） | [使い方](docs/hungarian.md) |

### graph/shortest_path

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/dijkstra.hpp` | Dijkstra・最短経路の復元 | [使い方](docs/dijkstra.md) |
| `graph/k_shortest_walk.hpp` | K Shortest Walk・再訪を許す経路長の列挙 | [使い方](docs/k_shortest_walk.md) |
| `graph/bellman_ford.hpp` | 負辺の最短経路・負閉路の復元 | [使い方](docs/bellman_ford.md) |
| `graph/monge_d_edge_shortest_path.hpp` | Monge グラフの d 辺最短路長 | [使い方](docs/monge_d_edge_shortest_path.md) |
| `graph/warshall_floyd.hpp` | ワーシャルフロイド法・全点対最短距離 | [使い方](docs/warshall_floyd.md) |
| `graph/difference_constraints.hpp` | 牛ゲー（差分制約系）・解の復元・差の上下限 | [使い方](docs/difference_constraints.md) |

### graph/spanning_tree

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `graph/kruskal.hpp` | Kruskal・最小全域木／最小全域森 | [使い方](docs/kruskal.md) |
| `graph/incremental_msf.hpp` | 辺追加に対応する最小全域森・経路の最大辺 | [使い方](docs/incremental_msf.md) |
| `graph/directed_mst.hpp` | 有向最小全域木・採用辺と親の復元 | [使い方](docs/directed_mst.md) |
| `graph/steiner_tree.hpp` | 最小シュタイナー木・採用辺の復元 | [使い方](docs/steiner_tree.md) |

### integer_geometry

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `integer_geometry/geometry.hpp` | 固定幅整数による平面幾何の一括 include | [使い方](docs/integer_geometry.md) |
| `integer_geometry/point.hpp` | 整数の点・ベクトル・偏角順 | [使い方](docs/integer_geometry_point.md) |
| `integer_geometry/linear.hpp` | 直線・線分の判定・平行・直交・交差 | [使い方](docs/integer_geometry_linear.md) |
| `integer_geometry/polygon.hpp` | 多角形の面積2倍・内外・凸性 | [使い方](docs/integer_geometry_polygon.md) |
| `integer_geometry/convex.hpp` | 凸包・最近点対・最遠点対・Minkowski 和 | [使い方](docs/integer_geometry_convex.md) |
| `integer_geometry/dynamic_convex_hull.hpp` | 点追加型の動的凸包・支持点・面積 | [使い方](docs/integer_geometry_dynamic_convex_hull.md) |
| `integer_geometry/circle.hpp` | 整数中心・整数半径の円の判定 | [使い方](docs/integer_geometry_circle.md) |
| `integer_geometry/lattice.hpp` | 方向ベクトルの約分・線分と多角形の格子点数 | [使い方](docs/integer_geometry_lattice.md) |

### math

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/mint.hpp` | modint998244353 | [使い方](docs/mint.md) |
| `math/modint.hpp` | 任意の法の静的・動的 modint | [使い方](docs/modint.md) |
| `math/bigint.hpp` | 符号付き多倍長整数・四則演算・10 進入出力 | [使い方](docs/bigint.md) |
| `math/fraction.hpp` | 有理数の四則演算・正確な比較 | [使い方](docs/fraction.md) |

### math/combinatorics

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/binomial_prefix_sum.hpp` | 二項係数の先頭区間和 | [使い方](docs/binomial_prefix_sum.md) |
| `math/binomial_prefix_sum_light.hpp` | 軽量版の二項係数の先頭区間和 | [使い方](docs/binomial_prefix_sum_light.md) |
| `math/binomial_prefix_sum_incremental.hpp` | 二項係数の先頭区間和（差分更新） | [使い方](docs/binomial_prefix_sum_incremental.md) |
| `math/binomial_prefix_sum_incremental_light.hpp` | 軽量版の二項係数の先頭区間和 差分更新 | [使い方](docs/binomial_prefix_sum_incremental_light.md) |
| `math/binomial_mod.hpp` | 任意 mod の二項係数・素数冪・合成数法 | [使い方](docs/binomial_mod.md) |
| `math/factorial_mod.hpp` | 大きな N の階乗 mod 998244353 | [使い方](docs/factorial_mod.md) |
| `math/comb.hpp` | modint 型を指定する階乗・逆階乗・逆元と二項係数 | [使い方](docs/comb.md) |
| `math/comb_mod.hpp` | modint を指定する階乗・逆階乗と二項係数 | [使い方](docs/comb_mod.md) |
| `math/comb_light.hpp` | 軽量版の二項係数・階乗・逆元表 | [使い方](docs/comb_light.md) |
| `math/fixed_size_subset_sum.hpp` | 個数を指定して選ぶ部分和の集合 | [使い方](docs/fixed_size_subset_sum.md) |

### math/floor_sum

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/floor_sum.hpp` | Floor Sum（一次式の床関数の総和） | [使い方](docs/floor_sum.md) |
| `math/generalized_floor_sum.hpp` | 一般化 floor sum | [使い方](docs/generalized_floor_sum.md) |
| `math/monoid_floor_sum.hpp` | monoid floor sum | [使い方](docs/monoid_floor_sum.md) |
| `math/min_of_mod_of_linear.hpp` | Mod of Linear の最小値 | [使い方](docs/min_of_mod_of_linear.md) |
| `math/prefix_min_of_mod_of_linear.hpp` | Mod of Linear の prefix minimum の等差数列圧縮 | [使い方](docs/prefix_min_of_mod_of_linear.md) |
| `math/partition_mod_of_linear.hpp` | Mod of Linear の列の等差数列分割 | [使い方](docs/partition_mod_of_linear.md) |

### math/linear_algebra

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/matrix.hpp` | 行列積・行列式・逆行列・連立一次方程式など | [使い方](docs/matrix.md) |
| `math/matrix_mod2.hpp` | mod 2 専用の行列積・階数・逆行列・連立方程式 | [使い方](docs/matrix_mod2.md) |
| `math/matrix_light.hpp` | 軽量な行列演算 | [使い方](docs/matrix_light.md) |
| `math/xor_basis.hpp` | XOR 基底・順序統計・共通部分・復元・ロールバック・区間クエリ | [使い方](docs/xor_basis.md) |

### math/number_theory

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/linear_congruence.hpp` | 一次合同式 ax ≡ b (mod m) の全解 | [使い方](docs/linear_congruence.md) |
| `math/crt.hpp` | 一般化 CRT・連立一次合同式 | [使い方](docs/crt.md) |
| `math/modular_arithmetic.hpp` | 一般の法の剰余累乗・逆元 | [使い方](docs/modular_arithmetic.md) |
| `math/linear_diophantine.hpp` | 区間制約付き一次不定方程式 | [使い方](docs/linear_diophantine.md) |
| `math/mod_inv.hpp` | 定数時間 mod inv | [使い方](docs/mod_inv.md) |
| `math/mod_pow.hpp` | 定数時間 mod pow・離散対数 | [使い方](docs/mod_pow.md) |
| `math/mod_sqrt.hpp` | 任意の素数 mod の平方根 | [使い方](docs/mod_sqrt.md) |
| `math/kth_root.hpp` | 64 bit 整数の厳密な k 乗根 | [使い方](docs/kth_root.md) |
| `math/discrete_log.hpp` | 合成数 mod にも対応する離散対数 | [使い方](docs/discrete_log.md) |
| `math/stern_brocot_tree.hpp` | Stern–Brocot Tree・圧縮経路・祖先・LCA・有理数二分探索 | [使い方](docs/stern_brocot_tree.md) |
| `math/factorize.hpp` | 素因数分解・素数判定・約数列挙 | [使い方](docs/factorize.md) |
| `math/multiplicative_divisors.hpp` | 約数ごとの乗法的関数の値 | [使い方](docs/multiplicative_divisors.md) |
| `math/is_prime.hpp` | 素数判定 | [使い方](docs/is_prime.md) |
| `math/is_prime_light.hpp` | 軽量版の素数判定 | [使い方](docs/is_prime_light.md) |
| `math/prime_sieve.hpp` | 素数列挙・間引き列挙 | [使い方](docs/prime_sieve.md) |
| `math/prime_count.hpp` | 上限以下の素数の個数 | [使い方](docs/prime_count.md) |
| `math/totient_sum.hpp` | Euler φ関数の先頭区間和 | [使い方](docs/totient_sum.md) |
| `math/multiplicative_prefix_sum.hpp` | 乗法的関数の先頭区間和 | [使い方](docs/multiplicative_prefix_sum.md) |

### math/util

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `math/integer_division.hpp` | 整数除算の切り下げ・切り上げ | [使い方](docs/integer_division.md) |
| `math/geometric_sum.hpp` | 等比数列の和 | [使い方](docs/geometric_sum.md) |

### sequence

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/cartesian_tree.hpp` | Cartesian Tree（根・親・左右の子） | [使い方](docs/cartesian_tree.md) |
| `tree/permutation_tree.hpp` | Permutation Tree（共通区間分解木） | [使い方](docs/permutation_tree.md) |
| `sequence/lis.hpp` | 最長増加部分列（長さ・復元） | [使い方](docs/lis.md) |
| `sequence/permutation_rank.hpp` | 順列の辞書順順位・復元 | [使い方](docs/permutation_rank.md) |
| `sequence/inversion_count.hpp` | 転倒数 | [使い方](docs/inversion_count.md) |
| `sequence/run_length_encoding.hpp` | 連長圧縮（RLE） | [使い方](docs/run_length_encoding.md) |

### string

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/concatenation_order.hpp` | 文字列結合の辞書順比較・並べ替え | [使い方](docs/concatenation_order.md) |
| `string/trie.hpp` | Trie・重複付き辞書・接頭辞検索・辞書順検索・木 DP | [使い方](docs/trie.md) |

### string/distance

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/lcs.hpp` | 最長共通部分列 | [使い方](docs/lcs.md) |
| `string/edit_distance.hpp` | 編集距離 | [使い方](docs/edit_distance.md) |

### string/palindrome

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/manacher.hpp` | 回文半径・全中心の最大回文長・区間の回文判定 | [使い方](docs/manacher.md) |
| `string/eertree.hpp` | Eertree（回文木）・回文の出現回数 | [使い方](docs/eertree.md) |

### string/pattern_matching

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/rolling_hash.hpp` | Rolling Hash | [使い方](docs/rolling_hash.md) |
| `string/z_algorithm.hpp` | Z algorithm | [使い方](docs/z_algorithm.md) |
| `string/wildcard_pattern_matching.hpp` | ワイルドカード付きパターンマッチング | [使い方](docs/wildcard_pattern_matching.md) |
| `string/aho_corasick.hpp` | Aho–Corasick・複数パターン検索 | [使い方](docs/aho_corasick.md) |
| `string/dynamic_pattern_matching.hpp` | Dynamic Pattern Matching（固定本文） | [使い方](docs/dynamic_pattern_matching.md) |

### string/substring

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `string/longest_common_substring.hpp` | 最長共通部分文字列 | [使い方](docs/longest_common_substring.md) |
| `string/suffix_array.hpp` | Suffix Array・整数列対応 | [使い方](docs/suffix_array_public.md) |
| `string/lcp_array.hpp` | LCP Array | [使い方](docs/lcp_array.md) |
| `string/run_enumerate.hpp` | Run 列挙 | [使い方](docs/run_enumerate.md) |
| `string/suffix_automaton.hpp` | Suffix Automaton・文字追加・部分文字列検索・出現回数 | [使い方](docs/suffix_automaton.md) |
| `string/suffix_tree.hpp` | Suffix Tree（静的文字列） | [使い方](docs/suffix_tree.md) |
| `string/basic_substring_structure.hpp` | Basic Substring Structure・部分文字列の階段状ブロック分解 | [使い方](docs/basic_substring_structure.md) |

### tree

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/prufer_code.hpp` | Prufer Code（木の符号化・復号） | [使い方](docs/prufer_code.md) |
| `tree/rooted_tree_hash.hpp` | 根付き木・各部分木のハッシュ | [使い方](docs/rooted_tree_hash.md) |
| `tree/tree_hash.hpp` | 根を持たない木のハッシュ | [使い方](docs/tree_hash.md) |
| `tree/auxiliary_tree.hpp` | Auxiliary Tree・圧縮木の隣接リスト | [使い方](docs/auxiliary_tree.md) |
| `tree/tree_diameter.hpp` | 木の直径・最長パスの復元 | [使い方](docs/tree_diameter.md) |
| `tree/tree_center.hpp` | 木の中心・最大距離を最小にする頂点 | [使い方](docs/tree_center.md) |
| `tree/rerooting.hpp` | 全方位木 DP | [使い方](docs/rerooting.md) |
| `tree/dsu_on_tree.hpp` | DSU on Tree | [使い方](docs/dsu_on_tree.md) |

### tree/dynamic

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/link_cut_tree.hpp` | Link Cut Tree | [使い方](docs/link_cut_tree.md) |
| `tree/edge_link_cut_tree.hpp` | 辺重み・方向付きの Link Cut Tree | [使い方](docs/edge_link_cut_tree.md) |
| `tree/top_tree.hpp` | Top Tree・辺の追加削除と木 DP | [使い方](docs/top_tree.md) |

### tree/query

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/lca.hpp` | LCA | [使い方](docs/lca.md) |
| `tree/path_product.hpp` | 頂点値更新・順序付き経路積 | [使い方](docs/path_product.md) |
| `tree/contour_sum.hpp` | 頂点値更新・距離区間の和 | [使い方](docs/contour_sum.md) |
| `tree/subtree_index.hpp` | 親配列から部分木の区間番号を構築 | [使い方](docs/subtree_index.md) |
| `tree/tree_mo.hpp` | 木上の Mo's Algorithm・頂点値／辺値の経路クエリ | [使い方](docs/tree_mo.md) |

### data_structure/advanced

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `tree/hld.hpp` | Heavy-Light Decomposition | [使い方](docs/hld.md) |
| `tree/static_top_tree.hpp` | Static Top Tree・動的な木 DP | [使い方](docs/static_top_tree.md) |
| `tree/centroid_decomposition.hpp` | 重心分解 | [使い方](docs/centroid_decomposition.md) |
| `data_structure/kinetic_tournament.hpp` | Kinetic Tournament (固定係数昇順・更新係数非負) | [使い方](docs/kinetic_tournament.md) |
| `data_structure/partially_retroactive_priority_queue.hpp` | Partially Retroactive Priority Queue | [使い方](docs/partially_retroactive_priority_queue.md) |
| `data_structure/retroactive_priority_queue.hpp` | Fully Retroactive Priority Queue | [使い方](docs/retroactive_priority_queue.md) |

## 補助部品

### components/convolution

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/bitwise_convolution_modular.hpp` | ビット演算畳み込みの modint 用共通実装 | [使い方](docs/bitwise_convolution_modular.md) |
| `components/ntt_mod_core.hpp` | 法指定 NTT の共通演算 | [使い方](docs/ntt_mod_core.md) |
| `components/convolution_mod_core.hpp` | 任意 mod・整数畳み込みの共通処理 | [使い方](docs/convolution_mod_core.md) |
| `components/min_plus_convolution_integral.hpp` | min-plus / max-plus 畳み込みの整数用共通実装 | [使い方](docs/min_plus_convolution_integral.md) |
| `components/multivariate_convolution_core.hpp` | 多変数畳み込みの共通実装 | [使い方](docs/multivariate_convolution_core.md) |
| `components/multivariate_convolution_product.hpp` | 多変数畳み込みの積 | [使い方](docs/multivariate_convolution_product.md) |
| `components/multivariate_ranked.hpp` | 小さい次元の多変数畳み込み | [使い方](docs/multivariate_ranked.md) |
| `components/min_plus_convolution_envelope.hpp` | 凹列と任意列の畳み込み | [使い方](docs/min_plus_convolution_envelope.md) |
| `components/subset_convolution_modular.hpp` | Subset 畳み込みの modint 用処理 | [使い方](docs/subset_convolution_modular.md) |
| `components/ntt_mod_blocked.hpp` | 任意法畳み込みのブロック変換 | [使い方](docs/ntt_mod_blocked.md) |
| `components/convolution_dispatch.hpp` | 畳み込みの型別振り分け | [使い方](docs/convolution_dispatch.md) |
| `components/gcd_convolution_mint.hpp` | modint998244353 の GCD 畳み込み | [使い方](docs/gcd_convolution_mint.md) |
| `components/lcm_convolution_mint.hpp` | modint998244353 の LCM 畳み込み | [使い方](docs/lcm_convolution_mint.md) |
| `components/symmetric_group_fft.hpp` | 順列の合成用の Fourier 変換 | [使い方](docs/symmetric_group_fft.md) |
| `components/mint_convolution.hpp` | modint998244353 の共有畳み込み | [使い方](docs/mint_convolution.md) |

### components/data_structure

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/xor_select.hpp` | 64 bit 内の XOR 順位選択 | [使い方](docs/xor_select.md) |
| `components/range_add_min_node.hpp` | 区間加算・最小値のノード演算 | [使い方](docs/range_add_min_node.md) |
| `components/range_affine_sum_simd.hpp` | 区間アフィン変換・区間和の共通実装 | [使い方](docs/range_affine_sum_simd.md) |
| `components/range_affine_point_simd.hpp` | 区間アフィン変換・一点取得の共通実装 | [使い方](docs/range_affine_point_simd.md) |
| `components/predecessor_set.hpp` | Predecessor Set（64 分木） | [使い方](docs/predecessor_set.md) |
| `components/small_vector.hpp` | Small Vector | [使い方](docs/small_vector.md) |
| `components/dynamic_bit_vector.hpp` | Dynamic Bit Vector | [使い方](docs/dynamic_bit_vector.md) |
| `components/dynamic_quad_vector.hpp` | Dynamic Quad Vector | [使い方](docs/dynamic_quad_vector.md) |
| `components/quad_bit_vector.hpp` | Quad Bit Vector | [使い方](docs/quad_bit_vector.md) |
| `components/range_lis.hpp` | Range LIS の前計算 | [使い方](docs/range_lis_component.md) |
| `components/oct_bit_vector.hpp` | Oct Bit Vector | [使い方](docs/oct_bit_vector.md) |
| `components/fenwick_sum.hpp` | Fenwick Sum | [使い方](docs/fenwick_sum.md) |
| `components/rectangle_sum_polynomial.hpp` | 矩形和の四つの係数 | [使い方](docs/rectangle_sum_polynomial.md) |
| `components/bitset_ops.hpp` | Bitset のワード演算・ビット数集約 | [使い方](docs/bitset_ops.md) |
| `components/fenwick_bitset.hpp` | 更新可能ビット列の区間個数 | [使い方](docs/fenwick_bitset.md) |
| `components/incremental_prefix_count.hpp` | 一点加算・接頭辞個数 | [使い方](docs/incremental_prefix_count.md) |
| `components/offline_range_kth.hpp` | 区間順位の一括処理 | [使い方](docs/offline_range_kth.md) |
| `components/insert_only_prefix_count_sum.hpp` | 挿入のみの区間和・個数 | [使い方](docs/insert_only_prefix_count_sum.md) |
| `components/static_range_min.hpp` | ブロック分割による静的 RMQ | [使い方](docs/static_range_min.md) |
| `components/offline_time_segments.hpp` | オフラインの時間区間処理 | [使い方](docs/offline_time_segments.md) |
| `components/d_ary_heap.hpp` | D-ary Heap | [使い方](docs/d_ary_heap.md) |
| `components/smawk.hpp` | SMAWK | [使い方](docs/smawk.md) |
| `components/monotone_minima.hpp` | Monotone Minima | [使い方](docs/monotone_minima.md) |
| `components/online_monge_minima.hpp` | Online Monge Minima | [使い方](docs/online_monge_minima.md) |
| `components/xor_basis_core.hpp` | XOR 基底の内部演算 | [使い方](docs/xor_basis_core.md) |
| `components/RadixHeap.hpp` | Radix Heap | [使い方](docs/radix_heap.md) |

### components/fps

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/rational_fps_coefficients.hpp` | 有理 FPS の区間係数取得の共通処理 | [使い方](docs/rational_fps_coefficients_core.md) |
| `components/polynomial_half_gcd.hpp` | 多項式 Half-GCD | [使い方](docs/polynomial_half_gcd.md) |
| `components/polynomial_half_gcd_light.hpp` | 軽量 FPS の Half-GCD | [使い方](docs/polynomial_half_gcd_light.md) |
| `components/fps_light_product_tree.hpp` | 軽量 FPS の多項式の積をまとめる木 | [使い方](docs/fps_light_product_tree.md) |
| `components/fps_operations.hpp` | FPS の内部演算 | [使い方](docs/fps_operations.md) |
| `components/fps_sparse.hpp` | 疎 FPS の共通演算 | [使い方](docs/fps_sparse.md) |
| `components/fps_sparse_core.hpp` | 疎 FPS の次数圧縮・前処理 | [使い方](docs/fps_sparse_core.md) |
| `components/fps_sparse_scalar.hpp` | 軽量 FPS の疎演算 | [使い方](docs/fps_sparse_scalar.md) |
| `components/fps_factorials.hpp` | 階乗・逆階乗生成 | [使い方](docs/fps_factorials.md) |
| `components/fps_workspace.hpp` | FPS の作業領域 | [使い方](docs/fps_workspace.md) |
| `components/fps_log.hpp` | FPS の対数の内部変換 | [使い方](docs/fps_log.md) |
| `components/fps_blocked.hpp` | FPS のブロック変換 | [使い方](docs/fps_blocked.md) |
| `components/fps_bostan.hpp` | Bostan–Mori の倍長化 | [使い方](docs/fps_bostan.md) |
| `components/fps_horner.hpp` | FPS の一点評価 | [使い方](docs/fps_horner.md) |
| `components/polynomial_gcd_blocked.hpp` | 多項式 Half-GCD の専用変換 | [使い方](docs/polynomial_gcd_blocked.md) |
| `components/fps_composition.hpp` | FPS の合成・冪ごとの係数計算の内部処理 | [使い方](docs/fps_composition.md) |
| `components/fps_composition_common.hpp` | FPS の合成・冪ごとの係数計算の共通変換 | [使い方](docs/fps_composition_common.md) |
| `components/fps_compose_blocked.hpp` | FPS の合成の専用変換 | [使い方](docs/fps_compose_blocked.md) |
| `components/fps_projection_blocked.hpp` | FPS の冪ごとの係数計算の専用変換 | [使い方](docs/fps_projection_blocked.md) |
| `components/fps_composition_light.hpp` | 軽量 FPS の合成・冪ごとの係数計算の共通部品 | [使い方](docs/fps_composition_light.md) |
| `components/fps_composition_generic.hpp` | FPS の合成・冪ごとの係数計算の汎用実装 | [使い方](docs/fps_composition_generic.md) |
| `components/fps_composition_stages.hpp` | FPS の合成・冪ごとの係数計算の共通段 | [使い方](docs/fps_composition_stages.md) |
| `components/fps_compose_generic.hpp` | FPS の合成の汎用実装 | [使い方](docs/fps_compose_generic.md) |
| `components/fps_projection_generic.hpp` | FPS の冪ごとの係数計算の汎用実装 | [使い方](docs/fps_projection_generic.md) |
| `components/fps_reversion_generic.hpp` | FPS の合成逆元の汎用実装 | [使い方](docs/fps_reversion_generic.md) |
| `components/fps_convolution.hpp` | FPS の融合畳み込み | [使い方](docs/fps_convolution.md) |
| `components/fps_convolution_unbalanced.hpp` | FPS の非対称畳み込み | [使い方](docs/fps_convolution_unbalanced.md) |
| `components/fps_convolution_constant.hpp` | FPS の定数係数畳み込み | [使い方](docs/fps_convolution_constant.md) |
| `components/fps_direct_eval.hpp` | 多点評価の内部直接計算 | [使い方](docs/fps_direct_eval.md) |
| `components/fps_division_blocked.hpp` | FPS と多項式除算の内部変換 | [使い方](docs/fps_division_blocked.md) |
| `components/fps_evaluation.hpp` | 多点評価・補間の SIMD 木 | [使い方](docs/fps_evaluation.md) |
| `components/fps_evaluation_common.hpp` | 多点評価・補間が共有する積の木 | [使い方](docs/fps_evaluation_common.md) |
| `components/fps_multipoint.hpp` | 多点評価の SIMD 処理 | [使い方](docs/fps_multipoint.md) |
| `components/fps_interpolation.hpp` | 多項式補間の SIMD 処理 | [使い方](docs/fps_interpolation.md) |
| `components/fps_sparse_terms.hpp` | 疎 FPS の項収集・演算種別 | [使い方](docs/fps_sparse_terms.md) |
| `components/polynomial_multiply.hpp` | 多項式の積・打ち切り積の共通処理 | [使い方](docs/polynomial_multiply.md) |
| `components/fps_exp_blocked.hpp` | FPS の指数・冪の内部変換 | [使い方](docs/fps_exp_blocked.md) |
| `components/fps_inverse_blocked.hpp` | FPS 逆数のブロック変換 | [使い方](docs/fps_inverse_blocked.md) |
| `components/fps_output.hpp` | FPS の結果配列への一括変換 | [使い方](docs/fps_output.md) |
| `components/fps_reversion.hpp` | FPS の合成逆元の内部処理 | [使い方](docs/fps_reversion.md) |
| `components/fps_sqrt_blocked.hpp` | FPS 平方根のブロック変換 | [使い方](docs/fps_sqrt_blocked.md) |
| `components/fps_taylor_blocked.hpp` | Taylor Shift の中間積 | [使い方](docs/fps_taylor_blocked.md) |
| `components/fps_io.hpp` | FPS の一括入出力 | [使い方](docs/fps_io.md) |
| `components/fps_write.hpp` | FPS の高速出力 | [使い方](docs/fps_write.md) |
| `components/fps_product_tree.hpp` | 多点評価・補間の内部木 | [使い方](docs/fps_product_tree.md) |
| `components/polynomial_product_tree.hpp` | 多項式の積をまとめる共通の木 | [使い方](docs/polynomial_product_tree.md) |

### components/geometry

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/packed_convex_hull.hpp` | 整数点の凸包 | [使い方](docs/packed_convex_hull.md) |
| `components/integer_geometry_bridge.hpp` | 整数幾何と通常幾何の厳密な座標変換 | [使い方](docs/integer_geometry_bridge.md) |
| `components/closest_pair.hpp` | 最近点対の整数座標用処理 | [使い方](docs/closest_pair_core.md) |

### components/graph

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/flow_graph.hpp` | フローの残余グラフ | [使い方](docs/flow_graph.md) |
| `components/undirected_csr.hpp` | 無向グラフの CSR | [使い方](docs/undirected_csr.md) |
| `components/push_relabel.hpp` | Push-Relabel の内部演算 | [使い方](docs/push_relabel.md) |
| `components/min_cost_circulation.hpp` | 最小費用流の容量スケーリング | [使い方](docs/min_cost_circulation.md) |
| `components/min_cost_simplex.hpp` | 最小費用流の Network Simplex | [使い方](docs/min_cost_simplex.md) |
| `components/weighted_blossom.hpp` | 最大重みマッチングの内部部品 | [使い方](docs/weighted_blossom.md) |
| `components/bipartite_matching_core.hpp` | 二部マッチングの内部演算 | [使い方](docs/bipartite_matching_core.md) |
| `components/dijkstra_queue.hpp` | Dijkstra の内部キュー | [使い方](docs/dijkstra_queue.md) |
| `components/dijkstra_topological.hpp` | Dijkstra のトポロジカル前処理 | [使い方](docs/dijkstra_topological.md) |
| `components/dijkstra_unique.hpp` | Dijkstra の一意経路の探索 | [使い方](docs/dijkstra_unique.md) |
| `components/scc_core.hpp` | 反復 SCC の共通部品 | [使い方](docs/scc_core.md) |
| `components/two_sat_core.hpp` | 2-SAT の含意伝播・強連結成分判定の共通部品 | [使い方](docs/two_sat_core.md) |
| `components/shortest_path_scc.hpp` | 最短路で使う到達可能部分の強連結成分分解 | [使い方](docs/shortest_path_scc.md) |
| `components/directed_csr.hpp` | 重み付き有向グラフの CSR | [使い方](docs/directed_csr.md) |

### components/math

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/barrett32.hpp` | 32 bit Barrett 演算 | [使い方](docs/barrett32.md) |
| `components/modular_power_sequence.hpp` | 剰余を取った冪の列 | [使い方](docs/modular_power_sequence.md) |
| `components/binomial_prefix_core.hpp` | 二項係数の先頭区間和の内部演算 | [使い方](docs/binomial_prefix_core.md) |
| `components/binomial_prefix_core_light.hpp` | 軽量版の二項係数の先頭区間和 の共通部品 | [使い方](docs/binomial_prefix_core_light.md) |
| `components/limb_arithmetic.hpp` | 整数を分割した桁配列の演算 | [使い方](docs/limb_arithmetic.md) |
| `components/bigint_magnitude.hpp` | 多倍長整数の絶対値演算・NTT 乗算・逆数除算 | [使い方](docs/bigint_magnitude.md) |
| `components/integer_arithmetic.hpp` | 整数型の判定・オーバーフローを検出する加算と乗算 | [使い方](docs/integer_arithmetic.md) |
| `components/field_arithmetic.hpp` | 体の演算・内積・配列の定数倍と加算 | [使い方](docs/field_arithmetic.md) |
| `components/field_base.hpp` | 汎用型の体の演算 | [使い方](docs/field_base.md) |
| `components/is_prime_core.hpp` | 素数判定の共通処理 | [使い方](docs/is_prime_core.md) |
| `components/is_prime_bases.hpp` | 素数判定の基数表 | [使い方](docs/is_prime_bases.md) |
| `components/matrix_core.hpp` | 行列の共通クラス | [使い方](docs/matrix_core.md) |
| `components/matrix_base_arithmetic.hpp` | 行列の基本演算 | [使い方](docs/matrix_base_arithmetic.md) |
| `components/matrix_power.hpp` | 行列累乗・特性多項式の内部演算 | [使い方](docs/matrix_power.md) |
| `components/factorial_table.hpp` | 型を指定する階乗・逆階乗テーブル | [使い方](docs/factorial_table.md) |
| `components/factorial_table_light.hpp` | 軽量版の階乗・逆階乗生成 | [使い方](docs/factorial_table_light.md) |
| `components/factorial_table_generic.hpp` | 汎用 modint の階乗・逆階乗生成 | [使い方](docs/factorial_table_generic.md) |
| `components/factorial_mod_table.hpp` | 998244353 専用の階乗定数表 | [使い方](docs/factorial_mod_table.md) |
| `components/stern_brocot_path.hpp` | Stern–Brocot Tree の経路演算 | [使い方](docs/stern_brocot_path.md) |
| `components/montgomery.hpp` | Montgomery 64 | [使い方](docs/montgomery.md) |
| `components/montgomery32.hpp` | Montgomery 32 | [使い方](docs/montgomery32.md) |
| `components/mod_fast_core.hpp` | 定数時間 mod 演算の内部部品 | [使い方](docs/mod_fast_core.md) |
| `components/multiplicative_sum_backend.hpp` | 乗法的関数の和の内部処理 | [使い方](docs/multiplicative_sum_backend.md) |
| `components/mod_linear_prefix_min.hpp` | Mod of Linear の prefix minimum の内部走査 | [使い方](docs/mod_linear_prefix_min.md) |
| `components/matrix_arithmetic.hpp` | 行列の内部演算 | [使い方](docs/matrix_arithmetic.md) |
| `components/matrix_block_inverse.hpp` | 行列のブロック逆行列・余因子行列 | [使い方](docs/matrix_block_inverse.md) |
| `components/matrix_elimination_modular.hpp` | 固定法の行列消去 | [使い方](docs/matrix_elimination_modular.md) |
| `components/fixed_size_subset_sum.hpp` | 個数指定の部分和集合の NTT・厳密判定 | [使い方](docs/fixed_size_subset_sum_core.md) |
| `components/ordered_midpoint.hpp` | 順序付き整数区間の中点 | [使い方](docs/ordered_midpoint.md) |

### components/string

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/sequence_match.hpp` | 前方・後方の一致比較 | [使い方](docs/sequence_match.md) |
| `components/z_algorithm.hpp` | Z algorithm・接頭辞一致の列挙 | [使い方](docs/z_algorithm_core.md) |
| `components/wildcard_pattern_matching.hpp` | ワイルドカード照合の内部部品 | [使い方](docs/wildcard_pattern_matching_core.md) |
| `components/sequence_compare.hpp` | 共通接頭辞・接尾辞の比較 | [使い方](docs/sequence_compare.md) |
| `components/substring_seed.hpp` | 共通部分文字列の候補走査 | [使い方](docs/substring_seed.md) |
| `components/suffix_array.hpp` | SA-IS による接尾辞配列の構築 | [使い方](docs/suffix_array.md) |
| `components/suffix_locus.hpp` | 本文の区間から Suffix Tree の位置を検索 | [使い方](docs/suffix_locus.md) |

### components/tree

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/link_cut_tree_core.hpp` | Link Cut Tree の共通処理 | [使い方](docs/link_cut_tree_core.md) |
| `components/static_top_tree_structure.hpp` | Static Top Tree の構造 | [使い方](docs/static_top_tree_structure.md) |
| `components/tree_hash_core.hpp` | 木ハッシュの共通部品 | [使い方](docs/tree_hash_core.md) |

### components/util

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `components/compiler_features.hpp` | コンパイラとアーキテクチャの共通条件 | [使い方](docs/compiler_features.md) |
| `components/binary_search.hpp` | 条件が切り替わる境界の二分探索 | [使い方](docs/binary_search.md) |
| `components/fast_io.hpp` | Fast I/O | [使い方](docs/fast_io.md) |
| `components/fast_io_bulk.hpp` | 整数の一括入出力 | [使い方](docs/fast_io_bulk.md) |
| `components/fast_io_output.hpp` | 非負整数の一括出力 | [使い方](docs/fast_io_output.md) |
| `components/fast_io_output_values.hpp` | 非負整数列の一括出力 | [使い方](docs/fast_io_output_values.md) |
| `components/dynamic_quad_wavelet.hpp` | Dynamic Quad Wavelet Matrix | [使い方](docs/dynamic_quad_wavelet.md) |
| `components/default_init_allocator.hpp` | Default Init Allocator | [使い方](docs/default_init_allocator.md) |
| `components/radix_sort.hpp` | 64 bit キーの安定基数ソート | [使い方](docs/radix_sort.md) |
| `components/splitmix64.hpp` | SplitMix64 | [使い方](docs/splitmix64.md) |
| `components/warshall_floyd_kernel.hpp` | ワーシャルフロイド法のブロック・SIMD 演算 | [使い方](docs/warshall_floyd_kernel.md) |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。各ライブラリは単独で利用できます。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

FPS も通常の `oj-bundle` だけで展開できます。専用の圧縮ツールは不要です。

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## ライセンス

[CC0 1.0 Universal](https://github.com/qandori/cp-library/blob/main/LICENSE) を採用しています。著作権表示なしで利用・変更・再配布・商用利用できます。
