# kyopro-library

[![verify](https://github.com/qandori/kyopro-library/actions/workflows/verify.yml/badge.svg)](https://github.com/qandori/kyopro-library/actions/workflows/verify.yml)

競技プログラミング用の C++ ライブラリです。

## ライブラリ

| ファイル | 内容 |
| --- | --- |
| `template.hpp` | `rep`・`per`・`chmin`・`chmax` などの共通テンプレート |
| `graph/dijkstra.hpp` | 非負の辺の重みに対する最短路（到達不能は `-1`） |
| `math/mint.hpp` | mod 998244353 の整数型 `mint` |
| `math/comb.hpp` | 階乗・逆階乗と二項係数 |
| `math/fps.hpp` | 形式的冪級数 `fps` |
| `math/sqrt_floor.hpp` | Mod of Linear の列を $O\left(\sqrt{N}\right)$ 個の等差数列の区間に分割 |

## 使い方

リポジトリを `lib/` に配置し、必要なヘッダーを include します。

提出には [oj-bundle](https://github.com/online-judge-tools/verification-helper) の利用を想定しています。

例えば

```sh
oj-bundle a.cpp | pbcopy
```

を実行することで展開先のコードを自動でコピーすることができます。

## 検証

push・Pull Request 時に GitHub Actions で GCC / Clang の自作テストと Library Checker の問題を実行します。

ローカルでは次のコマンドで同じ検証を実行できます。

```sh
uv tool install --with 'setuptools<82' online-judge-verify-helper
python3 test/verify.py
```

自作テストだけを実行する場合：

```sh
python3 test/verify.py --unit-only
python3 test/verify.py --unit-only --cxx clang++ --sanitize
```

新しいライブラリを追加したら、`test/unit/` に自作テスト、または `test/` に `PROBLEM` を指定した `.test.cpp` を追加してください。

## ライセンス

[MIT License](LICENSE)。誰でも利用・変更・再配布・商用利用できます。コピーや再配布の際は、著作権表示とライセンス全文を残してください。
