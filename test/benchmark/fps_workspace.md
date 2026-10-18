# FPS の作業領域

比較元は `e348f396c801964e72d4b1460a39dae91fdc88a3`。2026-10-18 UTC に測定した。前回の FPS 高速化に対する追加変更であり、公開 FPS API と既存の `join_windows` の関数型を維持する。

## 上位提出との比較

前回取得した [逆元 409370](https://judge.yosupo.jp/submission/409370)、[対数 409452](https://judge.yosupo.jp/submission/409452)、[指数 409662](https://judge.yosupo.jp/submission/409662)、[平方根 409728](https://judge.yosupo.jp/submission/409728)、[冪 409718](https://judge.yosupo.jp/submission/409718) のソースを再確認した。取得時の順位・時刻・ソース SHA-256 は [参照一覧](results/fps_speed_reference.json) にある。最終比較では各問題の上位 2 提出を変更せず再コンパイルする。

上位実装の `poly::Arena` は各配列の間に 64 byte を置き、Linux の大きなページも利用する。既存の FPS は、長さが 2 冪の変換結果を隙間なく並べていた。多項積和で同じ添字を参照すると、多数の配列が L1 キャッシュの同じ集合へ集中する配置だった。

## 変更

級数除算・対数・指数・冪・平方根が保持する変換結果の先頭間隔を、変換長に 16 ワードを加えた長さへ変更する。変換自体の長さと演算手順は維持する。新しい `join_windows_strided` に先頭間隔を渡し、既存の `join_windows` は従来の連続配置を扱う。

大きな作業領域は `components/fps_workspace.hpp` へ分離する。Linux では 2 MiB 以上の要求に対して匿名マッピングを作り、2 MiB 境界に整列して `MADV_HUGEPAGE` を要求する。要求サイズは 2 MiB 単位に切り上げ、整列のためにさらに 2 MiB の仮想アドレス領域を取る。大きなページの利用可否は OS と資源に依存する。

マッピングに失敗した場合と他の環境では、従来と同じ 64 byte に整列した配列確保を使う。マッピング上では配置 new によって配列の生存期間を開始し、所有するポインタが確保方法と範囲を保持して解放する。要素は使用前に書き込む。確保サイズの算術はオーバーフローを検査する。FPS の係数配列の型・所有権・返り値は維持する。

## 測定条件

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効、ヘッダーの既存 O3 指定を含む。THP 設定は `enabled = madvise`、`defrag = madvise`。設定の実際の文字列は JSON に保存する。

予備実行 1 回の後、seed 20261018 で実行順をシャッフルして 9 回測定する。各試行は別プロセスで、初期化・確保を含む。入力生成とチェックサムは計算部分の時間から除き、全版のチェックサムを照合する。プロセス全体は公式入力を通常のファイルから読み、通常のファイルへ出力する。全出力を公式 checker で検証し、checker の時間は測定から除く。測定中は検証やコンパイルを並行実行しない。

これは AtCoder・Library Checker の実機での測定ではない。上位提出の掲載時間とローカル時間を混ぜず、同じローカル入力と条件で比較する。最速にはまだ差が残る。

## 計算部分

524,288 項。単位 ms。間隔のみは通常の整列確保、ページのみは配列間隔を従来のままにする。

| 操作 | 前回版 | 間隔のみ | ページのみ | 両方 |
| --- | ---: | ---: | ---: | ---: |
| 逆元 | 10.280 | 10.211 | 9.475 | 9.426 |
| 対数 | 19.999 | 18.466 | 18.304 | 17.292 |
| 指数 | 22.997 | 21.233 | 21.164 | 19.688 |
| 平方根 | 13.416 | 13.211 | 18.020 | 12.602 |
| 冪 | 43.576 | 39.311 | 39.832 | 36.600 |
| 級数除算 | 17.394 | 15.977 | 15.268 | 14.245 |
| 多項式除算 | 17.373 | 16.778 | 17.485 | 16.478 |
| 平方 | 10.430 | 10.457 | 10.163 | 10.099 |
| 畳み込み | 11.715 | 11.574 | 11.738 | 12.016 |

実装を変更していない平方・畳み込みの差も掲載する。小さい差を効果とは扱わない。

## プロセス全体

公式の最大規模の入力。単位 ms。各列は同じ環境で再コンパイルした実行ファイルの測定値。

| 操作 | 前回版 | 変更後 | 上位 2 提出 |
| --- | ---: | ---: | --- |
| 逆元 | 20.203 | 19.632 | 409370: 10.987, 409242: 12.339 |
| 指数 | 31.653 | 29.011 | 409662: 15.891, 409661: 15.343 |
| 対数 | 28.447 | 25.869 | 409452: 13.224, 409363: 13.653 |
| 平方根 | 22.773 | 21.718 | 409728: 10.933, 409316: 11.795 |
| 冪 | 51.215 | 46.192 | 409718: 23.226, 409358: 25.358 |

入出力は今回変更していない。`current_old_io` は `current` と同じ I/O を持つ別ディレクトリでのビルドで、試行の変動を見る参考値として JSON に残す。入力・ソース・フラグ・全試行は [最終測定](results/fps_workspace.json) にある。

## メモリ

同じ計算部分のドライバー、524,288 項、各 1 回。入力生成の 2 配列など、プロセス全体を含む最大 RSS。単位 KiB。

| 操作 | 前回版 | 変更後 |
| --- | ---: | ---: |
| 逆元 | 12676 | 12796 |
| 対数 | 18448 | 18608 |
| 指数 | 18444 | 18652 |
| 平方根 | 14592 | 14740 |
| 冪 | 20692 | 20768 |

[メモリ測定](results/fps_workspace_memory.json) にソースのハッシュと、単独で確保・書き込み後に `/proc/self/smaps_rollup` から確認した値も残す。RSS は環境と大きなページの割当て状況に依存し、全環境で一定の使用量を保証する値ではない。

## 検証

依存先を抽出し、20 ヘッダーと 14 単体テストを GCC で確認した。GCC の ASan・UBSan も実行した。最終調整後は 20 ヘッダーと `fps.cpp`・`fps_speed.cpp`・`fps_workspace.cpp` を再確認し、同じ 3 単体テストの ASan・UBSan も成功した。作業領域の単体テストでは空・小規模・2 MiB 前後、移動と再代入、配列間の隙間の保持、剰余上限付近を確認する。元の 3 引数の関数ポインタ型もコンパイルで確認する。

最終版の公式ケースは 22 ドライバー・596 ケースが成功した。yukicoder 1307 の 2 ドライバーは、ローカルにトークンがないため CI で確認する。Clang と macOS も CI で確認する。ローカルの LeakSanitizer は既存環境の ptrace 制約のため無効にし、`ASAN_OPTIONS=detect_leaks=0` と `UBSAN_OPTIONS=halt_on_error=1` を指定した。

126 ヘッダーの文書を確認し、影響する 20 結合ヘッダーをコンパイルした。ローカルの文書生成には利用可能な GCC を使った。

検証ログは [通常](results/fps_workspace_check.log)、[最終調整後](results/fps_workspace_final_check.log)、[Sanitizer](results/fps_workspace_sanitize.log)、[最終 Sanitizer](results/fps_workspace_final_sanitize.log)、[公式ケース](results/fps_workspace_judge.json)、[文書](results/fps_workspace_docs.log) にある。

## 候補

[選別データ](results/fps_workspace_experiments.json) にソースハッシュ・生の試行・条件を残す。配列間隔は 8・16・32・64 ワードを比較し、16 ワードを採用した。大きなページだけを使う候補では平方根に大きな遅延が出たため、間隔の変更と組み合わせる。

部分変換を級数除算へ単純に適用する候補、逆元の完全 NTT 化、radix-4 の分解、SIMD 葉のループ展開と奇数側のロード変更、逆元の 2 配列間の間隔変更も比較した。密な入力全体で安定した改善を確認できず、採用していない。

## 再現

```sh
python3 test/benchmark/fps_speed.py --baseline e348f396c801964e72d4b1460a39dae91fdc88a3 --output workspace.json --workspace-candidates --operations inv log exp sqrt pow div_series divmod square multiply --official-root /tmp/library-checker-problems --references /tmp/fps-reference
python3 test/benchmark/fps_workspace_memory.py --output memory.json
python3 test/benchmark/fps_speed_verify.py --official-root /tmp/library-checker-problems --selection test/benchmark/results/fps_workspace_selection.json --output verification.json
```

参照ソースと公式入力の準備は [前回の再現手順](fps_speed.md#再現) に従う。メモリ測定は Linux の `wait4` が返す各子プロセスの最大 RSS を使い、速度測定とは別に行う。
