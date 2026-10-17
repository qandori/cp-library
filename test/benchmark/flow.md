# 最大流・最小費用流

以下の BigInt の結果は削除前の記録です。現在の No.3669 ドライバは Boost の `cpp_int` を使います。[比較記録](bigint.md)を参照してください。再測定には Boost のヘッダーが必要で、`flow.py --boost-include` で検索パスを指定できます。

## 参考提出・資料

2026-10-17 に上位 AC を取得した。最大流に直接対応する Library Checker 問題は見つからなかったため、指定された yukicoder No.3668 の実行時間順の提出を確認した。

| 問題 | 提出 | 取得時の時間 | 手法 |
| --- | --- | ---: | --- |
| [yukicoder 3668](https://yukicoder.me/problems/no/3668) | [1174960](https://yukicoder.me/submissions/1174960) | 24 ms | Highest Label Push-Relabel、Gap、Global Relabel |
| 同上 | [1174927](https://yukicoder.me/submissions/1174927) | 31 ms | 同系統、辺の走査順の調整 |
| 同上 | [1182754](https://yukicoder.me/submissions/1182754) | 33 ms | Highest Label Push-Relabel |
| [Minimum Cost b-flow](https://judge.yosupo.jp/problem/min_cost_b_flow) | [385580](https://judge.yosupo.jp/submission/385580) | 2 ms | Network Simplex |
| 同上 | [364560](https://judge.yosupo.jp/submission/364560) | 2 ms | Network Simplex |
| 同上 | [313861](https://judge.yosupo.jp/submission/313861) | 2 ms | Network Simplex |

[AC Library の最大流](https://github.com/atcoder/ac-library/blob/master/atcoder/maxflow.hpp) の逆向き DFS、[最小費用流](https://github.com/atcoder/ac-library/blob/master/atcoder/mincostflow.hpp) のポテンシャルと reduced cost が 0 の辺の処理も確認した。今回の残余グラフは正逆辺を連続配列に置き、隣接する辺の番号を CSR 形式に構築する。辺を後から追加しても現在の流量は保つ。

最大流の上限なしの API は Highest Label Push-Relabel、上限付きは反復版 Dinic。Push-Relabel の余剰流量には 128 bit を使い、始点から出る容量の総和が容量型を超える場合もフロー保存則を保つ。Dinic も含めて再帰は使わない。

最小費用流は逐次最短路法を使い、Radix Heap と 0 費用の辺を処理するスタックを併用する。増加回数が `max(64,min(N,512))` に達した場合、残りは容量スケーリングに切り替える。容量を上位 bit から復元し、負の reduced cost の辺を飽和させ、余剰流量を解消する。スケーリングの内部費用は 128 bit、ヒープは 4 分木。`slope` は折れ点を全て求めるため逐次最短路法を継続する。

最小費用 b-flow の検証では、負費用辺を上限まで流した状態から非負費用の逆辺へ変換し、頂点の需給を調整して最小費用流を解く。この変換により、負閉路・負の下限・自己辺を含む公式ケースも扱う。双対ポテンシャルと全辺のフローも公式 checker で検証した。

取得結果・ソースのハッシュは `results/max_flow_reference.json` と `results/flow_reference.json`。

```sh
python3 test/benchmark/max_flow_reference.py --cache .verify-helper/cache/max-flow-references --output test/benchmark/results/max_flow_reference.json
python3 test/benchmark/flow_reference.py --cache .verify-helper/cache/flow-references --output test/benchmark/results/flow_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。seed 36683669、各条件 7 回、実行順はシャッフル、各実装のウォームアップは 1 回。測定中は他のテストを実行していない。子プロセスの起動・入力・グラフの構築・計算・出力・終了を含む実時間を測定した。コンパイル・入力生成・出力検査は時間外。

| 入力 | 内容 |
| --- | --- |
| max_random | N=2000、M=10000、ランダムな有向辺、容量 1..10^9 |
| max_ladder | N=2000、始点から各中間頂点へ容量 1、中間頂点を容量 10^9 の鎖でつなぐ。Dinic の距離が少しずつ増える |
| max_layered | 幅 20、98 層。次の層へ各頂点から 4 辺 |
| min_random | N=100、M=1000、需給 0、容量 1..10^9、費用 ±10^9 の最小費用 b-flow |
| min_anti_ssp | Library Checker の公式 `anti_ssp_00` |
| dijkstra_bigint | yukicoder No.3669 の制約内で N=30000、M=50000、連結な無向グラフ。分子・分母が 300 以下の既約分数 |

最大流は今回の二つの手法と提出 1174960、最小費用流は今回の b-flow 検証プログラムと提出 385580 を比較した。最大流では答え全体、最小費用流では目的関数値が一致した。各実装の出力ハッシュも全試行で一致した。最小費用流のフローとポテンシャルは解が一意でないため、比較実装間での一致は要求しない。

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。以下は中央値、単位は ms。AtCoder 実機での速度は未測定。公開ジャッジの時間とローカル時間は直接比較しない。

| 条件 | 入力 | Push-Relabel | Dinic | 提出 1174960 |
| --- | --- | ---: | ---: | ---: |
| generic | max_random | 3.469 | 3.501 | 3.794 |
| generic | max_ladder | 3.070 | 42.156 | 3.447 |
| generic | max_layered | 6.257 | 5.005 | 6.036 |
| native | max_random | 3.353 | 3.499 | 3.825 |
| native | max_ladder | 3.309 | 54.618 | 3.483 |
| native | max_layered | 6.151 | 5.788 | 5.935 |

| 条件 | 入力 | 今回の最小費用流 | 提出 385580 |
| --- | --- | ---: | ---: |
| generic | min_random | 12.874 | 3.847 |
| generic | min_anti_ssp | 3.409 | 3.083 |
| native | min_random | 12.540 | 3.825 |
| native | min_anti_ssp | 3.545 | 3.363 |

最大流は入力によって有利な手法が変わる。最小費用 b-flow の測定では、今回の汎用 API より上位 AC の Network Simplex が速かった。最速とは主張しない。`dijkstra_bigint` の中央値は generic 223.208 ms、native 224.129 ms だった。

```sh
python3 test/benchmark/flow.py --max-reference .verify-helper/cache/max-flow-references/1174960.cpp --min-reference .verify-helper/cache/flow-references/385580.cpp --anti-ssp .verify-helper/cache/fde4f1d25a7bfd29d312403ba399d3f7/test/anti_ssp_00.in --output test/benchmark/results/flow_generic
python3 test/benchmark/flow.py --max-reference .verify-helper/cache/max-flow-references/1174960.cpp --min-reference .verify-helper/cache/flow-references/385580.cpp --anti-ssp .verify-helper/cache/fde4f1d25a7bfd29d312403ba399d3f7/test/anti_ssp_00.in --output test/benchmark/results/flow_native --native
```

## 検証

最大流は小規模グラフの全カット、フロー保存則、最小カット、上限付きの継続、辺追加、容量変更、リセット、30 万頂点の鎖、符号付き・符号なし容量、始点の容量総和が 64 bit を超える場合を確認した。

最小費用流は小規模グラフの全フロー割り当て、負費用辺、自己辺、費用曲線、双対条件、1 単位ずつの継続、辺追加、リセット、通常法と容量スケーリングの比較、UINT64_MAX の容量、128 bit の費用を確認した。GCC・Clang と ASan/UBSan を使った。

AOJ GRL_6_A は 40 ケース、GRL_6_B は 32 ケース、Library Checker Minimum Cost b-flow は 54 ケースを GCC・Clang の両方で通過した。

指定された [No.3668](https://yukicoder.me/problems/no/3668) は最大流、[No.3669](https://yukicoder.me/problems/no/3669) は `dijkstra<BigInt>` の検証に登録した。後者は分母の最小公倍数を BigInt で作り、通分した整数で最短路を求め、最後に多倍長の GCD と除算で既約分数へ戻す。公開サンプルに加え、Python の Fraction と比較するランダムな 60 ケースを GCC・Clang で確認した。ローカルの YUKICODER_TOKEN は未設定のため、公式全ケースは既存の GitHub CI の secret で検証する。

FastOutput は `__int128_t`・`__uint128_t` の出力を追加し、最小値・最大値を確認した。標準整数型と BigInt の既存テストも通過した。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/flow_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/flow_selection.json
oj-verify run test/max_flow.test.cpp test/min_cost_flow.test.cpp test/min_cost_b_flow.test.cpp
oj download -d .verify-helper/cache/flow-samples/3668 https://yukicoder.me/problems/no/3668
oj download -d .verify-helper/cache/flow-samples/3669 https://yukicoder.me/problems/no/3669
python3 test/benchmark/flow_check.py --samples .verify-helper/cache/flow-samples
```
