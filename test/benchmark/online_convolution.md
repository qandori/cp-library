# Online Convolution

## 参考実装

2026-10-17 に Library Checker の AC を実行時間順に調べた。オンライン畳み込み専用の問題ではなく、畳み込みと FPS exp の上位提出を確認した。

| 問題・実装 | 参照先 | 取得時の時間 | 手法 |
| --- | --- | ---: | --- |
| Convolution | [409465](https://judge.yosupo.jp/submission/409465) | 12 ms | 全係数を先に受け取る NTT。サイズに応じた積と専用の入出力 |
| 同上 | [409433](https://judge.yosupo.jp/submission/409433) / [409432](https://judge.yosupo.jp/submission/409432) | 13 / 13 ms | 同系統の上位 AC |
| Exp of Formal Power Series | [409662](https://judge.yosupo.jp/submission/409662) | 16 ms | 全係数を先に受け取る Newton 法。NTT と作業領域を再利用 |
| 同上 | [409661](https://judge.yosupo.jp/submission/409661) / [409402](https://judge.yosupo.jp/submission/409402) | 16 / 16 ms | 同系統の上位 AC |
| cplib-cpp | [relaxed_multiplication.hpp](https://github.com/hitonanode/cplib-cpp/blob/d67be71282f9f59d357f94d020c217f746613eae/convolution/relaxed_multiplication.hpp) | 下記で測定 | lowbit ごとの区間分割、先頭区間の NTT の保存、巡回畳み込みの折り返し除去 |
| Nyaan's Library | [relaxed-convolution.hpp](https://github.com/NyaanNyaan/library/blob/b3981adc80a800b2584980b01821324ea6c77183/ntt/relaxed-convolution.hpp) | 未測定 | 係数対の追加後に将来の区間を計算。先頭区間の NTT を保存 |

今回の実装は両方の数列をオンラインで受け取る。追加済みの結果と先頭区間の NTT を保存し、lowbit に対応する区間だけを更新する。2 の冪の境界では同じ長さの巡回畳み込みを使い、確定済みの下位係数を引いて上位係数を取り出す。それ以外では末尾区間と保存済みの先頭区間を掛け、必要な中央部分だけを取り出す。入力数の上限を指定した場合は不要な先読みを省く。

NTT は既存の `components/ntt.hpp` を使い、Montgomery 表現のまま計算する。変換の並べ替えは不要で、作業配列も再利用する。小さい区間は直接計算する。追加前に `pending()` を参照できるように、次の係数の既知部分を保つ。

取得した提出・公開実装の commit と SHA-256 は `results/online_convolution_reference.json` に保存した。参照ソース自体はローカルキャッシュに置く。

```sh
python3 test/benchmark/online_convolution_reference.py --cache .verify-helper/cache/online_convolution_reference --output test/benchmark/results/online_convolution_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、各条件を 7 回、順番をシャッフルして実行した。各実装は計測前に 1 回実行し、出力全体を照合した。seed は 682341。測定中に他のテストは実行していない。

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。今回の実装は generic でも実行時の AVX2 判定を行う。提出 409662 は generic では追加で `-march=haswell` を指定し、ソースは変更していない。cplib-cpp は取得した 3 ヘッダーを変更せず、同じドライバの `push()` を `add()` に接続した。

子プロセスの起動・入力・構築・計算・全係数の出力・終了を含む実時間。コンパイル・入力生成・出力検査は計測外。入力はパイプで渡す。

| 入力 | 内容 |
| --- | --- |
| pairs | 乱数の係数対を追加し、積の先頭 N 係数を求める |
| feedback | 直前の返り値を `last` として `a[i]+last`・`b[i]+last*last` を追加する。将来の入力が前の結果に依存する |
| exp | 定数項 0 の乱数 FPS の exp。導関数と計算済みの exp の係数をオンラインで畳み込む |

以下は中央値、単位は ms。

| 条件 | N | 入力 | 今回 | cplib-cpp | 提出 409662 |
| --- | ---: | --- | ---: | ---: | ---: |
| generic | 8192 | pairs | 4.295 | 5.160 | — |
| generic | 8192 | feedback | 4.163 | 5.028 | — |
| generic | 8192 | exp | 4.011 | 4.836 | 3.521 |
| generic | 131072 | pairs | 22.007 | 50.801 | — |
| generic | 131072 | feedback | 22.551 | 50.331 | — |
| generic | 131072 | exp | 21.382 | 43.152 | 8.580 |
| generic | 500000 | pairs | 80.520 | 189.601 | — |
| generic | 500000 | feedback | 83.357 | 191.770 | — |
| generic | 500000 | exp | 80.788 | 194.186 | 25.976 |
| native | 8192 | pairs | 3.989 | 5.095 | — |
| native | 8192 | feedback | 4.021 | 5.046 | — |
| native | 8192 | exp | 3.907 | 4.720 | 3.884 |
| native | 131072 | pairs | 21.459 | 51.495 | — |
| native | 131072 | feedback | 22.670 | 51.752 | — |
| native | 131072 | exp | 23.255 | 44.946 | 10.350 |
| native | 500000 | pairs | 78.238 | 192.064 | — |
| native | 500000 | feedback | 82.071 | 192.705 | — |
| native | 500000 | exp | 75.844 | 185.991 | 24.535 |

今回の環境では、大きい入力で比較したオンライン実装より速かった。全係数が既知の FPS exp では Newton 法の上位提出の方が速い。通常の FPS 演算は既存の一括処理を使い、入力が前の結果に依存するときにオンライン畳み込みを使う。AtCoder 実機での速度は未測定で、公開ジャッジの時間とは直接比較しない。

直接計算の閾値は N=131072 の 3 種類の入力で 2・4・8・16・32・64 を比較した。4 と 8 は近く、16 以上より速かったため 8 を採用した。全試行・入力ハッシュ・出力ハッシュ・ソースハッシュは `results/online_convolution_tuning/results.json`、`results/online_convolution_generic/results.json`、`results/online_convolution_native/results.json` に保存した。

```sh
python3 test/benchmark/online_convolution.py --tune --output test/benchmark/results/online_convolution_tuning
python3 test/benchmark/online_convolution.py --output test/benchmark/results/online_convolution_generic
python3 test/benchmark/online_convolution.py --native --output test/benchmark/results/online_convolution_native
```

## 検証

GCC・Clang の単体テスト、Clang の ASan/UBSan、GCC 13 の単体テストを通過した。小さい入力は既知部分を直接計算し、大きい入力は既存の一括畳み込みと照合した。空入力・0・最大剰余・2 の冪の前後・途中でのコピーと分岐・移動・clear・別の modint 型を検証した。`pending()` の値から次の入力を決める乱択テストも含む。複数ヘッダーを併用する standalone テストも通過した。

Library Checker の公式ケースは、Convolution 53 件、Exp of Formal Power Series 26 件、Inv of Formal Power Series 25 件を GCC・Clang の両方で通過した。これらは公開ジャッジへの提出ではなく、公式入力・出力・チェッカーを使うローカル検証である。

最大対応長の 2^23 項は、両方の列を 1 にしたときの第 i 係数が i+1、追加前の既知部分が max(0,i-1) であることを全項検査した。GCC 15.2・`-O2 -DNDEBUG` で成功した。リリース条件では既存 NTT の assert 専用変数に未使用警告が出るため、このチェックに `-Werror` は指定していない。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/online_convolution_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/online_convolution_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/online_convolution.test.cpp test/online_convolution_exp.test.cpp test/online_convolution_inv.test.cpp
g++ -std=c++2b -O2 -DNDEBUG -I . test/benchmark/online_convolution_limit.cpp -o /tmp/online-convolution-limit
/tmp/online-convolution-limit
```
