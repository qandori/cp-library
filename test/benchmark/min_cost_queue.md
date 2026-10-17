# 最小費用流の 0 費用キュー

`components/min_cost_circulation.hpp` の容量スケーリングで、reduced cost が 0 の辺をスタックへ入れ、128 bit の 4 分木ヒープを通さない。公開 API、容量・費用の型、残余グラフの構造、`slope` は変更しない。追加領域は頂点数に比例する整数列。

非負の reduced cost に対する Dijkstra なので、スタックの頂点は現在の最短距離を持つ。スタックを空にしてから次のヒープ要素を取り出す。等距離の処理順は変わるため、最適な辺ごとの流量は一致しなくてもよい。流量と目的関数値は一致させる。

## 参考

既存の調査は [flow.md](flow.md) を参照。[AC Library](https://github.com/atcoder/ac-library/blob/master/atcoder/mincostflow.hpp) と、このライブラリの逐次最短路法でも 0 費用の辺を別に処理している。容量スケーリングへ同じ方針を適用した。既存資料の最小費用 b-flow 上位提出は Network Simplex であり、今回の変更がそれを上回るとは主張しない。

## 比較方法

基準は `6af78c247f5c9beba109116966f3a3d4e9dc5104`。比較用プログラムは基準のヘッダーを別名で読み込み、同じプロセス内で変更前後を交互に実行する。通常の `flow` と内部の容量スケーリングを別々に測る。各条件 9 対、順序は `(対内の位置 + 試行番号 + 入力番号 + API 番号) % 2` で事前に決める。

各実装の測定時間を独立に積算し、通常は CPU 時間 200 ms 以上、2 頂点の並列辺は 500 ms 以上とする。CPU 時間と実時間を両方保存する。表の時間は 1 回あたりの中央値、SPEEDUP は各対の `変更前 / 変更後` の中央値であり、時間の中央値同士の比とは異なる。

seed は 36683669。通常は N=100、M=1000、容量 1..10^9。入力は正費用のランダムグラフ、約 90% が 0 費用、全辺 0 費用、費用 0..3、DAG、N=2・M=1000 の並列辺の 6 種類。各入力を通常の `flow` と内部スケーリングで測る。グラフのコピーと計算を測定し、入力生成は含めない。各測定の流量と 128 bit の目的関数値を完全一致で検査する。

```sh
python3 test/benchmark/min_cost_queue.py --baseline-ref 6af78c247f5c9beba109116966f3a3d4e9dc5104 --cpu 0 --trials 9 --min-cpu-ms 200 --small-cpu-ms 500 --output test/benchmark/results/min_cost_queue
```

コンパイラ、CPU、フラグ、ソースと全依存ヘッダーのハッシュ、全試行の出力を結果ディレクトリへ記録する。generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。生の結果ファイルはコミットしない。

最終測定はこの専用プログラムで行う。先行する最大流との共通プログラムによる試行と追加確認は探索・確認資料として区別する。

## 最終測定

2026-10-17、Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0、CPU 0 固定。他の測定・コンパイルと重ねずに実行した。24 条件を全て保持し、全測定が実時間ではなく CPU 時間の下限を満たした。最小の通常バッチは 200.043 ms。AtCoder 実機での速度は未測定。

| 条件 | API | 入力 | 変更前 ms | 変更後 ms | SPEEDUP |
| --- | --- | --- | ---: | ---: | ---: |
| generic | scaling | random_positive | 3.9112 | 2.2655 | 1.710 |
| generic | scaling | zero_90_percent | 4.0312 | 1.2990 | 3.063 |
| generic | scaling | all_zero | 3.9988 | 1.6883 | 2.370 |
| generic | scaling | cost_0_to_3 | 6.0847 | 3.0925 | 1.972 |
| generic | scaling | dag | 2.4715 | 1.1747 | 2.124 |
| generic | scaling | parallel_two_vertices | 1.1192 | 0.9100 | 1.172 |
| generic | public | random_positive | 1.0100 | 1.0107 | 0.994 |
| generic | public | zero_90_percent | 0.4434 | 0.4168 | 1.017 |
| generic | public | all_zero | 3.4271 | 2.2245 | 1.513 |
| generic | public | cost_0_to_3 | 15.1136 | 5.7652 | 2.566 |
| generic | public | dag | 0.2618 | 0.2480 | 1.041 |
| generic | public | parallel_two_vertices | 1.1306 | 1.1117 | 1.010 |
| native | scaling | random_positive | 3.7413 | 2.3169 | 1.567 |
| native | scaling | zero_90_percent | 3.2600 | 1.3531 | 2.427 |
| native | scaling | all_zero | 6.8698 | 3.1380 | 2.252 |
| native | scaling | cost_0_to_3 | 7.9182 | 3.8642 | 1.888 |
| native | scaling | dag | 2.8473 | 1.5320 | 1.905 |
| native | scaling | parallel_two_vertices | 0.9917 | 1.0567 | 0.979 |
| native | public | random_positive | 1.1462 | 1.1939 | 1.000 |
| native | public | zero_90_percent | 0.3253 | 0.3522 | 0.950 |
| native | public | all_zero | 2.7989 | 2.0887 | 1.309 |
| native | public | cost_0_to_3 | 11.9744 | 4.8230 | 2.485 |
| native | public | dag | 0.2525 | 0.2482 | 1.086 |
| native | public | parallel_two_vertices | 1.2304 | 1.2177 | 1.000 |

`public` は通常の `flow`、`scaling` は内部スケーリングを直接呼ぶ比較。通常 API の費用 0..3 は generic 2.566 倍、native 2.485 倍、全辺 0 費用は 1.513 倍と 1.309 倍だった。

不利な条件も残した。native の通常 API・約 90% が 0 費用の入力は SPEEDUP 0.950、native の内部スケーリング・2 頂点は 0.979 だった。通常 API・2 頂点は generic 1.010、native 1.000。先行する共通プログラムの長時間確認では 2 頂点で約 3–7% の悪化が出た。プログラム配置や実行時の変動に依存する小さい差もあるため、全入力での高速化や無劣化は主張しない。先行結果を最終測定へ混ぜたり、悪化した条件を除外したりしていない。

## 検証

既存の `min_cost_flow`、`flow_capacity` に加え、`min_cost_scaling` は 0 費用、多数の 0 費用、小さい費用、ランダム費用、ポテンシャル差を加えた負費用を含む 5000 ケースを逐次最短路法と比較する。流量・目的関数値・容量制約・フロー保存則を確認する。

GCC 14 の通常ビルドと ASan/UBSan で `min_cost_flow`、`flow_capacity`、`min_cost_scaling` が通過した。独立の小規模全探索 700 ケースと、全 12 入力・API 条件の流量と目的関数値の比較も通過した。実行環境の ptrace 制約により LeakSanitizer は無効化する。Clang と Mac の互換性は CI で別途確認する。ローカルには Clang がないため、ローカルでの通過は主張しない。
