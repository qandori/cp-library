# cp-library

[![verify](https://github.com/qandori/cp-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/cp-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

全部爆速で動作するようにしています。

## ライブラリ

| ファイル | 内容 |
| --- | --- |
| `template.hpp` | `rep`・`per`・`chmin`・`chmax` などの共通テンプレート |
| `data_structure/disjoint_sparse_table.hpp` | Disjoint Sparse Table |
| `data_structure/wavelet_matrix.hpp` | Wavelet Matrix |
| `data_structure/weighted_union_find.hpp` | 重み付き Union-Find |
| `graph/dijkstra.hpp` | dijkstra（到達不能は `-1`） |
| `math/mint.hpp` | modint998244353 |
| `math/comb.hpp` | 階乗・逆階乗と二項係数 |
| `math/fps.hpp` | 形式的冪級数 `fps` |
| `math/sqrt_floor.hpp` | Mod of Linear の列を $O\left(\sqrt{N}\right)$ 個の等差数列に分割 |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## ライセンス

[MIT License](LICENSE)。誰でも利用・変更・再配布・商用利用できます。コピーや再配布の際は、著作権表示とライセンス全文を残してください。
