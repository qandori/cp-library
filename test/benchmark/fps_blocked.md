# FPS 対数の専用バックエンド

比較元は `2be6469cbc4f7864e7920c98ac1a38450f0770eb`。2026-10-19（Asia/Tokyo）に測定した。[前回の診断](fps_log_gap.md) を基に実装した。公開 FPS API は維持する。

## 参照と変更

参照は [409452](https://judge.yosupo.jp/submission/409452)。ソース SHA-256 は `86ef3c3b4a4707ef5e7515e5229b2564f2a5f46bd830bb6cbe64b506a95675b6`。今回も順位を取得し、上位 3 提出の掲載時間は 13・14・15 ms だった。取得結果は [参照記録](results/fps_blocked_reference.json) にある。

4 ブロックの計算手順に加え、専用 Transform、変換表の配置、butterfly と葉の積の assembly、再帰タイル、部分逆変換、初期逆数の Newton 更新をまとめて `components/fps_blocked.hpp` へ取り込んだ。使わない通常の畳み込みクラスなどは含めていない。Linux x86_64 / GCC と実行時 AVX2 判定を組み合わせ、その他は既存の実装へ戻す。

入力は変更しない。内部ワードを $[0,p)$ に正規化して扱う。これは元の係数の一様な $R=2^{32}$ 倍であり、対数微分 $(Rf)'/(Rf)=f'/f$ では因子が消えるため、入力の Montgomery 解除を省ける。積分の逆数へ $R$ を掛け、結果を直接 Montgomery 表現にする。冪で使う積分しない対数微分は、出力時に Montgomery 表現へ戻す。

結果配列は容量を確保し、係数が求まった順に `emplace_back` する。最初に結果全体を 0 で埋めてから上書きする処理を省いた。API の入力配列を破壊したり、vector の要素数を超えた領域へ書き込んだりはしない。

参照側の mmap 専用 Arena は、既存の RAII 作業領域へ置き換えた。mmap が失敗した場合は整列した配列へ戻り、確保できなければ例外となる。未初期化の領域へ依存しないよう変換表の未使用部分と Window の最後のワードを初期化する。利用者の入力・出力へ余分な padding やアラインメントは要求しない。

FPS ドライバーでは `components/fps_io.hpp` の一括入出力を使う。入力は 2 トークンずつ SIMD で変換し、32 bit の上限・数字・区切り・バッファの残りを確認する。不規則な空白やバッファ末尾では既存の読み込みへ戻す。係数をまとめて Montgomery 表現にし、結果は 16 値ずつ固定幅に整形する。一般の `FastInput::read` と `FastOutput::write` の動作・形式は変更しない。

AVX2 の指定は各 SIMD ラムダにも明示する。引数・返り値に SIMD 値を持つ captureless lambda の暗黙の関数ポインタ変換も避ける。変換表生成のループは、根の添字の上限を明示した。

## 測定条件

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 に固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効。予備実行 1 回後に seed 20261018 で順番をシャッフルし、9 回測定する。中央値、単位 ms。

計算部分は入力生成とチェックサムを除き、確保・初期化を含む。全チェックサムを比較元と照合する。プロセス全体は通常ファイルから読み、通常ファイルへ出力する。全試行の出力を公式 checker で照合する。それぞれの比較は全版をビルドしてから計時し、測定中に他のコンパイル・検証を並行実行しない。

`current_old_io` は採用バックエンドに比較元の `FastInput`・FPS 入出力ヘルパーを組み合わせた版。比較元のヘルパーも git から取り出し、変更後の一括入出力を比較元へ混ぜないようベンチマークを修正した。ソース・ドライバー・入力・バイナリのハッシュと全試行を [最終測定](results/fps_blocked.json) に残す。

## 結果

計算部分。

| 項数 | 対数・比較元 | 対数・採用版 | 冪・比較元 | 冪・採用版 |
| ---: | ---: | ---: | ---: | ---: |
| 32,768 | 0.699 | 0.588 | 1.915 | 1.802 |
| 131,073 | 2.962 | 2.204 | 8.147 | 7.357 |
| 500,000 | 12.248 | 9.740 | 32.630 | 30.148 |
| 524,288 | 12.060 | 9.541 | 32.566 | 30.994 |

公式最大入力のプロセス全体。

| 操作 | 比較元 | 採用計算 + 旧入出力 | 採用版 | 上位提出 |
| --- | ---: | ---: | ---: | --- |
| log | 22.374 | 19.667 | 16.368 | 409452: 13.101, 409363: 13.543 |
| pow | 42.660 | 40.928 | 37.431 | 409718: 22.959, 409358: 25.158 |

50 万項の対数の計算部分は約 20.5%、公式最大入力の対数全体は約 26.8% 短縮した。52 万項の計算部分は約 20.9% 短縮した。最速コードとの差は、対数全体で約 9.27 ms から約 3.27 ms へ縮まった。比較した上位提出のソースは変更していない。

同じ採用計算での旧入出力 19.667 ms と一括入出力 16.368 ms を比較し、入出力側にも改善を確認した。ただし個々の差を別の測定セッションと足し合わせない。

最速版は入力・出力・表・作業領域を同じ Arena に置くインプレースなプログラムであり、通常の fps 配列を入出力に持つ本実装とはまだ異なる。最速の獲得は未確認で、Library Checker 実機での速度はローカル結果だけでは断定しない。

## 検証

GCC の通常設定で影響する 21 ヘッダーと 18 単体テストを確認し、同じ 18 単体テストを ASan・UBSan で確認した。既存の対数テストは、3・4 ブロックの境界、入力長と出力次数の違い、内部剰余値の上限付近、入力の保存を検証する。汎用係数型と冪も既存テストの対象である。

一括入出力の新しいテストは、32 bit の上限、短い入力、1024・4096 要素の前後、不規則な空白、改行なしの末尾、バッファをまたぐ入力を確認する。末尾の直後を `PROT_NONE` にした領域でも検証し、範囲外ロードを検出する。

FPS に関係する公式 22 ドライバー・596 ケースを checker で照合した。`components/fast_io.hpp` の変更は friend 宣言のみで、既存の整数読み書きの本体は変更していない。他のドライバーの I/O は従来の経路を使う。yukicoder 1307 の 2 ドライバー、Clang、macOS は CI でも確認する。ローカルには Clang と yukicoder のトークンがない。

結合ツールが `#pragma GCC target` のマクロ出力を扱えないため、同じ指定を `_Pragma` で記述する。2 ヘッダーで、空行を除いたプリプロセッサ出力の一致を確認した。表記変更前の検証時のハッシュと一致の記録も保存する。変更後の結合コードを含め、129 ヘッダーの文書と 21 結合ヘッダーを確認した。

LeakSanitizer は既存環境の ptrace 制約のためローカルでは無効にし、`ASAN_OPTIONS=detect_leaks=0`、`UBSAN_OPTIONS=halt_on_error=1` を指定する。CI では通常の設定で確認する。

記録: [範囲](results/fps_blocked_selection.json)、[通常](results/fps_blocked_check.log)、[Sanitizer](results/fps_blocked_sanitize.log)、[公式ケース](results/fps_blocked_judge.json)、[文書](results/fps_blocked_docs.log)、[pragma の同値性](results/fps_blocked_pragma.json)。

## 再現

```sh
python3 test/benchmark/fps_speed.py --baseline 2be6469cbc4f7864e7920c98ac1a38450f0770eb --output blocked.json --trials 9 --cpu 2 --target haswell --sizes 32768 131073 500000 524288 --operations log pow log:1:1 log:32:1024 --official-root /tmp/library-checker-problems --references /tmp/fps-reference
python3 test/verify.py --unit-only --cxx g++ --selection test/benchmark/results/fps_blocked_selection.json
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --sanitize --cxx g++ --selection test/benchmark/results/fps_blocked_selection.json
```

既存の `--log-candidates` は旧 LogTransform 経路の候補を採用バックエンドと比較する。`--workspace-candidates` の配置比較は新しい Arena にも適用する。
