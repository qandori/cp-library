# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

## ライブラリ

| ファイル | 内容 | 使い方 |
| --- | --- | --- |
| `data_structure/disjoint_sparse_table.hpp` | Disjoint Sparse Table | [使い方](docs/disjoint_sparse_table.md) |
| `data_structure/dynamic_connectivity.hpp` | Online Dynamic Connectivity | [使い方](docs/dynamic_connectivity.md) |
| `data_structure/li_chao_tree.hpp` | Li Chao Tree | [使い方](docs/li_chao_tree.md) |
| `data_structure/link_cut_tree.hpp` | Link Cut Tree | [使い方](docs/link_cut_tree.md) |
| `data_structure/wavelet_matrix.hpp` | Wavelet Matrix | [使い方](docs/wavelet_matrix.md) |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find | [使い方](docs/weighted_union_find.md) |
| `graph/dijkstra.hpp` | Dijkstra | [使い方](docs/dijkstra.md) |
| `graph/lca.hpp` | LCA | [使い方](docs/lca.md) |
| `graph/low_link.hpp` | Low-Link | [使い方](docs/low_link.md) |
| `math/mint.hpp` | modint998244353 | [使い方](docs/mint.md) |
| `math/comb.hpp` | 階乗・逆階乗と二項係数 | [使い方](docs/comb.md) |
| `math/factorize.hpp` | 素因数分解・素数判定 | [使い方](docs/factorize.md) |
| `math/monoid_floor_sum.hpp` | monoid floor sum | [使い方](docs/monoid_floor_sum.md) |
| `math/fps.hpp` | 形式的冪級数 | [使い方](docs/fps.md) |
| `math/sqrt_floor.hpp` | Mod of Linear の列の等差数列分割 | [使い方](docs/sqrt_floor.md) |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。各ライブラリは単独で利用できます。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## ライセンス

[MIT License](LICENSE) を採用しています。誰でも利用・変更・再配布・商用利用できます。コピーや再配布の際は、著作権表示とライセンス全文を残してください。
