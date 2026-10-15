# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

[ライブラリ検索・Bundle](https://qandori.github.io/cp-library/)

## ライブラリ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `convolution/convolution.hpp` | 通常の畳み込み | [使い方](docs/convolution.md) |
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
| `data_structure/d_ary_heap.hpp` | D-ary Heap | [使い方](docs/d_ary_heap.md) |
| `data_structure/slope_trick.hpp` | Slope Trick | [使い方](docs/slope_trick.md) |
| `data_structure/disjoint_sparse_table.hpp` | Disjoint Sparse Table | [使い方](docs/disjoint_sparse_table.md) |
| `data_structure/dynamic_segtree.hpp` | Dynamic Segtree | [使い方](docs/dynamic_segtree.md) |
| `data_structure/dynamic_connectivity.hpp` | Online Dynamic Connectivity | [使い方](docs/dynamic_connectivity.md) |
| `data_structure/li_chao_tree.hpp` | Li Chao Tree | [使い方](docs/li_chao_tree.md) |
| `data_structure/link_cut_tree.hpp` | Link Cut Tree | [使い方](docs/link_cut_tree.md) |
| `data_structure/wavelet_matrix.hpp` | Wavelet Matrix | [使い方](docs/wavelet_matrix.md) |
| `data_structure/dynamic_wavelet_matrix.hpp` | Dynamic Wavelet Matrix | [使い方](docs/dynamic_wavelet_matrix.md) |
| `data_structure/dynamic_bit_vector.hpp` | Dynamic Bit Vector | [使い方](docs/dynamic_bit_vector.md) |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find | [使い方](docs/weighted_union_find.md) |
| `graph/dijkstra.hpp` | Dijkstra | [使い方](docs/dijkstra.md) |
| `graph/lca.hpp` | LCA | [使い方](docs/lca.md) |
| `graph/low_link.hpp` | Low-Link | [使い方](docs/low_link.md) |
| `string/lcs.hpp` | 最長共通部分列 | [使い方](docs/lcs.md) |
| `string/edit_distance.hpp` | 編集距離 | [使い方](docs/edit_distance.md) |
| `math/mint.hpp` | modint998244353 | [使い方](docs/mint.md) |
| `math/comb.hpp` | 階乗・逆階乗と二項係数 | [使い方](docs/comb.md) |
| `math/factorize.hpp` | 素因数分解・素数判定 | [使い方](docs/factorize.md) |
| `math/monoid_floor_sum.hpp` | monoid floor sum | [使い方](docs/monoid_floor_sum.md) |
| `math/min_of_mod_of_linear.hpp` | Mod of Linear の最小値 | [使い方](docs/min_of_mod_of_linear.md) |
| `math/fps.hpp` | 形式的冪級数 | [使い方](docs/fps.md) |
| `math/sqrt_floor.hpp` | Mod of Linear の列の等差数列分割 | [使い方](docs/sqrt_floor.md) |
| `data_structure/RadixHeap.hpp` | Radix Heap | [使い方](docs/radix_heap.md) |
| `data_structure/quad_bit_vector.hpp` | Quad Bit Vector | [使い方](docs/quad_bit_vector.md) |
| `data_structure/default_init_allocator.hpp` | Default Init Allocator | [使い方](docs/default_init_allocator.md) |
| `data_structure/sparse_table.hpp` | Sparse Table | [使い方](docs/sparse_table.md) |
| `math/montgomery.hpp` | Montgomery 64 | [使い方](docs/montgomery.md) |
| `math/is_prime.hpp` | 素数判定 | [使い方](docs/is_prime.md) |
| `math/splitmix64.hpp` | SplitMix64 | [使い方](docs/splitmix64.md) |
| `math/prime_sieve.hpp` | 素数篩 | [使い方](docs/prime_sieve.md) |
| `math/smawk.hpp` | SMAWK | [使い方](docs/smawk.md) |
| `convolution/bitwise_zeta.hpp` | Bitwise Zeta / Möbius Transform | [使い方](docs/bitwise_zeta.md) |
| `convolution/walsh_hadamard.hpp` | Walsh–Hadamard Transform | [使い方](docs/walsh_hadamard.md) |
| `convolution/divisor_zeta.hpp` | Divisor Zeta / Möbius Transform | [使い方](docs/divisor_zeta.md) |
| `convolution/fft.hpp` | FFT | [使い方](docs/fft.md) |
| `convolution/ntt_mod.hpp` | NTT（法指定） | [使い方](docs/ntt_mod.md) |
| `convolution/ntt.hpp` | NTT（998244353） | [使い方](docs/ntt.md) |

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
