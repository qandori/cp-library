# Online Dynamic Connectivity の比較

`link_and_cut_00` の追加改善は [専用の比較](dynamic_connectivity_linkcut.md) にまとめています。

2026-10-17、WSL2 Linux x86_64 / Core i7-13700F / GCC 15.2.0。AtCoder 実機での測定ではありません。

## 対象環境と参考実装

[AtCoder の言語一覧](https://img.atcoder.jp/file/language-update/2025-10/language-list.html) にある GCC 15.2.0、`-O2 -march=native -ftrivial-auto-var-init=zero -std=gnu++23` を主な対象としました。測定用の `native` 設定ではこれらのフラグを使い、`-DATCODER -DONLINE_JUDGE` も指定しています。外部ライブラリのリンク指定などは省いています。`assert` は有効です。

[Library Checker の実行時間順 AC 一覧](https://judge.yosupo.jp/submissions?problem=dynamic_graph_vertex_add_component_sum&status=AC&order=%2Btime) の先頭 8 件と追加候補を取得しました。提出情報・ソース SHA-256 は `results/dynamic_connectivity_reference.json` にあります。

| 提出 | 確認した手法 |
| --- | --- |
| [402761 / nandhagk / 57 ms](https://judge.yosupo.jp/submission/402761) | 削除時刻を先読みする expiry AM forest と AVX2 入出力。オフライン処理。入出力込みの参考値を測定しました。 |
| [404999 / Rohan_Kapri / 58 ms](https://judge.yosupo.jp/submission/404999) | 同じ問題の高速 AC としてソースを取得しました。 |
| [340473 / nandhagk / 64 ms](https://judge.yosupo.jp/submission/340473) | 削除時刻を使う AM tree によるオフライン処理。 |
| [235063 / tonegawa / 103 ms](https://judge.yosupo.jp/submission/235063) | Rollback Union-Find によるオフライン処理。 |
| [100559 / maspy / 162 ms](https://judge.yosupo.jp/submission/100559) | 時間軸の分割統治と Rollback Union-Find。 |
| [93624 / suisen / 294 ms](https://judge.yosupo.jp/submission/93624) | オフラインでクエリを保持してから処理。 |

上記の最速値はオンライン API の速度を表していません。本ライブラリは先の操作を参照せず、各 `link`・`cut`・`prod` がその場で完了する構成を維持します。可換モノイドの集約、並行辺、自己ループも対象です。

## 採用した変更

- splay の zig-zig・zig-zag をまとめて処理し、親・祖父・曾祖父の参照とポインタの更新を重複させないようにしました。必要な 2 節点の `pull` と最後の根の更新は維持しています。
- 木ノードを `alignas(32)` にしました。対象の GCC/x86_64 ではノードが 28 byte から 32 byte になり、64 byte のキャッシュラインをまたがない配置になります。木ノード領域は約 14.3% 増えます。辺・ハッシュ表・集約値配列の形式は変更していません。

AVX2 を前提とするコードやグローバルな target pragma は追加していません。変更した箇所は標準 C++ の処理です。基準は `a5853326f05d47d2616b92ed63ff24063af76886` のヘッダーで、`dynamic_connectivity_baseline.hpp` に保存しています。

ノードと集約値の一体化、64 byte 配置、右端ノードのキャッシュ、根の集約値の転用、集約の計算順、削除時の作業配列の再利用、GCC O3 も比較しました。一貫した改善が得られないものや、ノードサイズの増加が大きいものは採用していません。探索結果は `results/dynamic_connectivity_*exploration/` にあります。

## 操作列の測定

100,000 頂点・初期辺の構築に続く 300,000 操作。入力生成は計測外で、グラフの構築・`reserve`・初期辺の追加・問い合わせ・checksum を計測します。CPU 2 固定、実行順を変えた 7 回の中央値です。入力 seed は 713、ハッシュ seed は試行ごとに `713 + 101*trial` とし、同じ試行の全方式で揃えました。seed の固定は測定用のコピーだけに行っています。

`forest` はパス、`cycle` は閉路、`dense` はパスに約 n 本のランダムな辺を加えた列、`star` はスターに葉のパスを加えた列から始めます。これらでは辺削除・成分集約・頂点更新・同じ辺の追加を繰り返します。`query` はランダム木に対する成分集約 60%・更新 30%・連結判定 10% です。集約なしでは成分集約と更新を `size(v)` に置き換えています。

`aggregate=1` は `int64_t` の加算、`aggregate=0` は `DynamicConnectivity<>` です。表は AtCoder の主要フラグを使った `native` 設定、単位は ms です。

| 集約 | 操作列 | 変更前 | 変更後 | 時間短縮 |
| --- | --- | ---: | ---: | ---: |
| あり | forest | 146.21 | 137.04 | 6.3% |
| あり | cycle | 180.05 | 174.91 | 2.9% |
| あり | dense | 280.67 | 277.26 | 1.2% |
| あり | star | 80.73 | 75.79 | 6.1% |
| あり | query | 120.43 | 109.47 | 9.1% |
| なし | forest | 117.13 | 92.43 | 21.1% |
| なし | cycle | 155.77 | 121.80 | 21.8% |
| なし | dense | 279.83 | 253.66 | 9.4% |
| なし | star | 82.69 | 63.86 | 22.8% |
| なし | query | 118.80 | 91.40 | 23.1% |

`-march=native` と `-ftrivial-auto-var-init=zero` を指定しない `generic` 設定でも測定しました。この操作列では集約ありで約 2.1〜8.6%、集約なしで約 14.5〜22.6% の時間短縮でした。全方式・全試行で checksum が一致しています。

## 公式ケースの測定

公式 19 ケースを入力ファイルから読み、実行ファイルの入出力込みで比較しました。CPU 4 固定、同じ主要フラグ、順序を変えた 7 回の中央値です。現在の verification driver と基準版で入出力コードを揃えています。プロセス起動時間も含みます。

| ケース | 変更前 | 変更後 | 402761（オフライン） |
| --- | ---: | ---: | ---: |
| link_and_cut_00 | 758.34 ms | 736.34 ms | 58.59 ms |
| dense_02 | 65.12 ms | 62.08 ms | 24.96 ms |
| max_random_00 | 133.61 ms | 138.91 ms | 35.59 ms |
| uv_swapped_01 | 139.47 ms | 145.63 ms | 35.81 ms |
| 19 ケースの中央値の合計 | 2137.36 ms | 2125.24 ms | 448.77 ms |

入出力込みの合計では約 0.6% の短縮にとどまり、遅くなるケースもあります。オンライン版が常に高速になるとはしていません。402761 は先読みできる解法であり、入出力も異なるため、そのままオンライン版と置き換えられる速度ではありません。

## 検証と再現

GCC・Clang で公式 19 ケースを全件通過しました。既存の BFS とのランダム差分テスト、加算・最小値・デフォルト構築できない独自型の集約、並行辺、自己ループ、代替辺探索、コピー、200,000 頂点のパス、依存先の統合テストも通過しています。ASan/UBSan、ARM64 GCC の静的ビルドと QEMU 実行、結合コードの単体コンパイルも確認しました。Mac 実機では未実行です。

検証対象・公式データの SHA-256 と結果は `results/dynamic_connectivity_validation.json`、公式検証のログは `results/dynamic_connectivity_judge_check.log` にあります。各測定フォルダには生の値、集計、フラグ、CPU、測定対象の SHA-256 を残しています。

```sh
python3 test/benchmark/dynamic_connectivity_reference.py --cache .verify-helper/cache/dynamic_connectivity/reference --output test/benchmark/results/dynamic_connectivity_reference.json
python3 test/verify.py --unit-only --selection test/benchmark/results/dynamic_connectivity_selection.json --cxx g++ clang++
python3 test/verify.py --unit-only --sanitize --selection test/benchmark/results/dynamic_connectivity_selection.json --cxx clang++
python3 .verify-helper/ci.py judge --selection test/benchmark/results/dynamic_connectivity_selection.json
python3 test/benchmark/dynamic_connectivity.py --output test/benchmark/results/dynamic_connectivity_native --native --variants baseline current fused pad32
python3 test/benchmark/dynamic_connectivity.py --output test/benchmark/results/dynamic_connectivity_generic --variants baseline current fused pad32
python3 test/benchmark/dynamic_connectivity_judge.py --tests .verify-helper/cache/2b6e7d11c688d398c5ee7d377261355c/test --reference-cache .verify-helper/cache/dynamic_connectivity/reference --output test/benchmark/results/dynamic_connectivity_judge --cpu 4
```

公式ケースのキャッシュ位置は verification-helper のログに表示されます。CPU 番号は `--cpu` で変更できます。比較用の提出ソースはキャッシュに置き、リポジトリには取得用スクリプトと出典情報を残します。
