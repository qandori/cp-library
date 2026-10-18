# FPS の逆元・疎な演算と入出力

比較元は `82fe5f034fca2805aeb59aee1be56216493ea5de`。2026-10-18 UTC に調査・測定した。公開 API と mod 998244353 を維持する。

## 上位提出

Library Checker API の時間順 AC 上位 3 提出を取得した。取得時刻・順位・実行時間・ソース SHA-256 は [参照一覧](results/fps_speed_reference.json) に残す。ローカルのプロセス全体の比較には各問題の上位 2 提出を変更せず使う。

| 操作 | 上位 3 提出と掲載時間 |
| --- | --- |
| 逆元 | [409370](https://judge.yosupo.jp/submission/409370) 11 ms, [409242](https://judge.yosupo.jp/submission/409242) 13 ms, [409369](https://judge.yosupo.jp/submission/409369) 18 ms |
| 対数 | [409452](https://judge.yosupo.jp/submission/409452) 13 ms, [409363](https://judge.yosupo.jp/submission/409363) 14 ms, [409264](https://judge.yosupo.jp/submission/409264) 15 ms |
| 指数 | [409662](https://judge.yosupo.jp/submission/409662) 16 ms, [409661](https://judge.yosupo.jp/submission/409661) 16 ms, [409402](https://judge.yosupo.jp/submission/409402) 16 ms |
| 平方根 | [409728](https://judge.yosupo.jp/submission/409728) 12 ms, [409316](https://judge.yosupo.jp/submission/409316) 16 ms, [409361](https://judge.yosupo.jp/submission/409361) 20 ms |
| 冪 | [409718](https://judge.yosupo.jp/submission/409718) 23 ms, [409358](https://judge.yosupo.jp/submission/409358) 26 ms, [409715](https://judge.yosupo.jp/submission/409715) 28 ms |

逆元の参照実装は Newton 法で変換を再利用し、必要な半分の逆変換と積を融合する。今回も変換を再利用する Newton 更新を採用した。指数・平方根の上位実装では最終段を短い変換に分ける。これらの密な演算には既存の区間法を使う。参照には専用の入出力・固定幅出力・早期実行・終了処理の省略を含むものがある。

## 変更

- 密な逆元は `m` 項から `2m` 項への Newton 更新を行う。`g` の変換を 2 回の積で共有し、従来のねじり・復元を伴う更新を省く。8 係数の環で積を計算する。
- 疎な逆元と指数は積を 7 項ずつまとめて還元する。係数の次数の最大公約数を `d` として、結果が零になる次数を飛ばす。指数では `x^d` を変数とする漸化式を使い、逆数表も必要な長さに縮める。
- 256 項を超える場合、逆元は `K <= 12d`、指数は `K <= 24d` を疎な経路の目安とする。これを超える入力を既存の密な経路へ渡す。検出する非零項数の上限はそれぞれ 32・64 のままにする。
- 一次式の逆元は等比数列、一次式の指数は逆階乗と等比数列で計算する。既存の SIMD 部品を使う。
- FPS のドライバーが使う `FastInput::read(unsigned&)` と `FastOutput::write(unsigned)` を改善した。little endian の入力は 11 byte 以上残る場合に 8 桁をまとめて解析し、桁・範囲・区切りを確認する。出力は 4 桁の表を使う。バッファ境界やほかの形式は従来の解析を使う。

### 値域

`p = 998244353`、`R = 2^32` とする。疎な積和では一方を `p` 未満、もう一方を `2p` 未満に保つ。`7(p-1)(2p-1) + (R-1)p < 2^64` を `static_assert` でも確認する。還元後に値域を整えてから加算する。

Newton 更新の再変換後は `4p` 未満の値を取り得る。2 回目の積でも入力を正規化する。8 項の積では両方を `p` 未満にするため、還元の加算を含めて 64 bit に収まる。`4p-1` までの内部値を使う単体テストを追加した。

入出力は読み込み可能な範囲内で `memcpy` し、出力の確保済み範囲内にだけ書く。バッファ末尾、10 桁の最大値、先頭の零、パイプの分割位置をテストする。

## 測定条件

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0。CPU 2 に固定する。フラグは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。ヘッダーの既存 O3 指定を含み、assert は有効。AVX2 までの命令を対象とする。

1 回の予備実行後、実行順を seed 20261018 でシャッフルして 9 回測る。係数生成は `mt19937(731)`。各試行は別プロセスで、NTT の初期化・確保も含む。計算部分では入力生成・チェックサムを除き、全試行で結果のチェックサムを照合する。疎な入力の `operation:K:d` は次数 `d, 2d, ..., Kd` の係数を生成する。

プロセス全体は公式入力を通常のファイルから読み、通常のファイルに出力する。起動・入出力・終了を含み、公式 checker の実行とハッシュ計算は除く。`current_old_io` は変更後の FPS と変更前の入出力を組み合わせたもの。全版の出力を checker で検証する。

測定中はほかの検証・コンパイルを実行しない。各版のヘッダーとドライバーのハッシュ、フラグ、生の試行値を [測定結果](results/fps_speed.json) に保存する。AtCoder・Library Checker の実機での測定ではない。

## 計算部分

| 操作・524,288 項 | 変更前 ms | 変更後 ms |
| --- | ---: | ---: |
| 逆元 | 11.348 | 10.269 |
| 対数 | 20.256 | 20.079 |
| 指数 | 22.471 | 22.263 |
| 平方根 | 13.228 | 13.139 |
| 冪 | 43.419 | 43.146 |
| 級数除算 | 16.884 | 16.927 |
| 多項式除算 | 17.502 | 17.412 |
| 平方 | 10.433 | 10.156 |
| 畳み込み | 11.849 | 11.604 |
| inv:1:1 | 5.138 | 2.031 |
| inv:12:1 | 28.282 | 9.280 |
| inv:32:1 | 61.258 | 11.099 |
| exp:1:1 | 6.614 | 4.191 |
| exp:24:1 | 51.153 | 18.989 |
| exp:64:1 | 128.844 | 23.308 |
| inv:32:1024 | 55.162 | 1.730 |
| exp:64:1024 | 121.934 | 1.849 |

畳み込みなど実装を変えていない演算の差も掲載する。小さい差を高速化の効果とは扱わない。

## プロセス全体

| 操作・公式入力 | 変更前 ms | FPS 変更後・旧 I/O ms | 全変更後 ms | 上位提出 ms |
| --- | ---: | ---: | ---: | --- |
| 逆元 / `max_random_00.in` | 24.235 | 23.926 | 19.295 | [409370](https://judge.yosupo.jp/submission/409370) 10.579, [409242](https://judge.yosupo.jp/submission/409242) 11.767 |
| 指数 / `max_random_00.in` | 35.932 | 35.376 | 31.571 | [409662](https://judge.yosupo.jp/submission/409662) 15.523, [409661](https://judge.yosupo.jp/submission/409661) 15.631 |
| 対数 / `max_random_00.in` | 33.056 | 32.826 | 28.084 | [409452](https://judge.yosupo.jp/submission/409452) 12.846, [409363](https://judge.yosupo.jp/submission/409363) 13.488 |
| 平方根 / `max_random_01.in` | 27.304 | 27.560 | 22.719 | [409728](https://judge.yosupo.jp/submission/409728) 10.908, [409316](https://judge.yosupo.jp/submission/409316) 11.700 |
| 冪 / `binary_exp_max_00.in` | 54.683 | 54.444 | 50.575 | [409718](https://judge.yosupo.jp/submission/409718) 22.848, [409358](https://judge.yosupo.jp/submission/409358) 24.983 |

上位提出の掲載時間とローカル時間は別の測定である。ローカルの上位提出との比較は同じ入力・環境で行う。最速の獲得は確認していない。

## 候補

Newton 更新の積をまとめる個数を 1・2・4・8 組で比較する。[生データ](results/fps_speed_candidates.json) に全試行とソースのハッシュを残す。採用版は 8 組である。

| 係数数 | 1 組 ms | 2 組 ms | 4 組 ms | 8 組 ms |
| ---: | ---: | ---: | ---: | ---: |
| 32768 | 0.682 | 0.613 | 0.552 | 0.535 |
| 131073 | 2.917 | 2.571 | 2.414 | 2.329 |
| 524288 | 12.693 | 11.313 | 10.722 | 10.239 |

小区画で変換と積を逐次融合する方法、汎用 NTT のキャッシュ分割、逆変換の半分だけを計算する候補も試した。安定した改善を確認できず、採用していない。

## 検証

GCC 14.2 で 24 ヘッダー・17 単体テストを確認した。段階ごとに変更された依存先を再検証し、同一ソースで成功済みの確認を再利用する。最終版の FPS は [GCC 検証](results/fps_speed_final_check.log)、入出力は [I/O 検証](results/fps_speed_io_check.log)、ASan・UBSan は [Sanitizer](results/fps_speed_final_sanitize.log) にある。スカラー経路を強制した版と、`NDEBUG` の新規テストも成功した。24 個の結合ヘッダーもコンパイルした。[検証の記録](results/fps_speed_validation.json) に対象とハッシュを保存する。

公式ケースは [FPS の 22 ドライバー・596 ケース](results/fps_speed_judge.json) と [オンライン畳み込みの 3 ドライバー・104 ケース](results/fps_speed_online_judge.json) を最終版で照合した。NTT ドライバーの 53 ケースは依存ソースが変わっていない先行確認を再利用する。I/O に依存する追加 43 ドライバーのコンパイルは [追加確認](results/fps_speed_io_drivers.json) に残す。追加ドライバーの公式ケースはこのローカル確認に含まない。

この環境の LeakSanitizer は ptrace 下で起動に失敗したため、`ASAN_OPTIONS=detect_leaks=0` と `UBSAN_OPTIONS=halt_on_error=1` で AddressSanitizer・UBSan を実行した。リーク検査は含まない。

Clang と Mac の実機検証は [Linux CI](https://github.com/qandori/cp-library/actions/workflows/verify.yml) と [Mac CI](https://github.com/qandori/cp-library/actions/workflows/macos-portability.yml) で確認する。ローカルでは Boost がない追加ドライバー 1 本と、トークンがない yukicoder の公式ケースを実行していない。

main のスカラー演算更新を保持して統合後、`fps.cpp` と `fps_speed.cpp` を通常・Sanitizer の両方で再確認し、公式 596 ケースと性能比較を再実行した。[統合後の Sanitizer](results/fps_speed_merge_sanitize.log) に結果を残す。

## 再現

```sh
python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference
python3 test/benchmark/fps_speed.py --output speed.json --official-root /tmp/library-checker-problems --references /tmp/fps-reference
python3 test/benchmark/fps_speed.py --output candidates.json --operations inv --sizes 32768 131073 524288 --candidates
python3 test/benchmark/fps_speed_verify.py --official-root /tmp/library-checker-problems --selection test/benchmark/results/fps_speed_selection.json --output verification.json --generate
```

参照取得スクリプトは保存済みの提出 ID を使い、ソースのハッシュを確認する。最新順位の調査には `fps_tuning_reference.py` を使える。

公式入力・解答・checker は Library Checker Problems の `generate.py` で作る。Python の依存は `test/requirements.txt` にある。計算部分だけの比較では `--official-root` と `--references` を省略できる。
