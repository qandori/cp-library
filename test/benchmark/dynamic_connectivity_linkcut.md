# Dynamic Connectivity の link_and_cut_00

2026-10-18 JST。最終比較元は `2be6469cbc4f7864e7920c98ac1a38450f0770eb`。オンライン API と可換モノイドの集約、並行辺、自己ループ、コピー、Mac の動作互換を維持する。

## 原因と変更

[公式ジェネレーター](https://github.com/yosupo06/library-checker-problems/blob/master/graph/dynamic_graph_vertex_add_component_sum/gen/link_and_cut.cpp) は、300,000 頂点の一部に大きな連結成分を作り、追加した順に辺を削除する。100 操作ごとに成分集約を行い、頂点値の更新はない。辺が関わる頂点は 74,300 個である。

この列では、代替辺の探索と上位レベルへの昇格が集中する。変更前の計数用ビルドでは約 337 万回の splay、2,022 万回の回転、22.6 万本の木辺の昇格が発生した。回転の約半分は切断と結合にあり、連結判定も約 619 万回の回転を占めた。他の入力と同じ負荷ではない。

採用した変更は次の二つである。

- 頂点の木ノードは他頂点との最初の辺追加時に作る。それまでは値と頂点表だけで扱い、`set`・`prod`・`size`・`same` は従来通り使える。頂点番号と木ノード番号を分離し、ノードを追加順に近い領域へ配置する。最初の辺追加前に更新した値も引き継ぐ。
- 切断する二つの有向辺ノードについて、最大 4 段だけ親を調べる。片方が根に近いと分かった場合は、その側から切断する。判定できなければ従来の順序を使う。解放する対の先頭は維持する。

32 byte のノード配置と既存の HDT / Euler Tour Tree を維持した。計算量は変更せず、追加の木ノード管理表も増やしていない。検証用ドライバーは既存の FastInput / FastOutput を使う形へ変更した。

計数用ビルドの `link_and_cut_00` は、確保した木ノード数が 824,513 → 587,394、頂点ノード数が 445,716 → 212,519、回転数が 20,218,403 → 19,966,011 だった。ノードの大きさは両方とも 32 byte、プロセスの最大 RSS は 69,520 → 60,140 KiB。回転数の削減だけでは説明できず、ノードを小さい領域にまとめた効果が大きいと考えられる。ハードウェアのキャッシュミス自体は測定していない。[計数結果](results/dynamic_connectivity_linkcut/profile.json)には上位レベル・入力と答えの保持も含む。

## 参考提出

[時間順 AC 一覧](https://judge.yosupo.jp/submissions?problem=dynamic_graph_vertex_add_component_sum&status=AC&order=%2Btime) の上位と比較候補を取得した。取得日時・URL・掲載時間・ソース SHA-256 は [参照一覧](results/dynamic_connectivity_linkcut_reference.json) にある。

| 提出 | 確認した手法 |
| --- | --- |
| [402761 / 57 ms](https://judge.yosupo.jp/submission/402761) | 削除時刻を先読みした expiry AM forest、専用 AVX2 入出力。全体比較の参考として再コンパイル |
| [404999 / 58 ms](https://judge.yosupo.jp/submission/404999) | 操作列から生存期間を作り、同様のオフライン forest で集約 |
| [147929 / 243 ms](https://judge.yosupo.jp/submission/147929) | クエリを保持し、時間の分割と巻き戻せる Union-Find で処理 |

掲載時間は取得時のジャッジ上の値である。これらのオフライン解法は、各操作がその場で完了する本ライブラリの代用にはしていない。後述の参考値は同じローカル環境で実行したものとし、掲載時間とは直接比較しない。

## 条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。`-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -DATCODER -DONLINE_JUDGE`、assert は有効。`/tmp/cp-dc-linkcut.lock` でコンパイル・検証・計測を排他する。実行ファイルと入出力は Linux の `/tmp` に置く。

1 回のウォームアップ後、新しいプロセスで各 9 回測定する。順序の seed は 20261018、ハッシュの seed は試行ごとに `713 + 101 * trial` とし、両版に同じ値を使う。以下は中央値、単位 ms。ソースをコピーした後は固定し、終了時にも SHA-256 を照合した。

- `kernel` は入力解析を計測前に行い、グラフ構築・`reserve(q)`・オンライン API 呼出し・答えの保持を測る。将来の操作はグラフ処理へ参照させない。checksum と出力は計測外。
- 全体はプロセス起動・通常ファイルの入出力・終了を含む。旧 stdio / 新 stdio は同じ変更前のドライバー、旧 FastIO / 新 FastIO は同じ変更後のドライバーで、ライブラリと入出力の効果を分ける。
- 計算部分は公式正答の全整数から作った checksum と照合する。全体は各方式の出力を公式 checker で確認する。checker と SHA-256 の計算は時間に含めない。

AtCoder・Library Checker 実機の測定ではなく、最速性や全入力での改善は主張しない。

## 結果

`link_and_cut_00` の計算部分は 456.127 → 376.366 ms、約 17.5% 短縮した。同じ FastIO で比べた全体は 482.380 → 420.034 ms、約 12.9% 短縮。従来の stdio ドライバーからの全体の差は 502.202 → 420.034 ms、約 16.4% で、入出力変更も含む。

| 公式入力 | 変更前 kernel | 変更後 kernel | 変更前 stdio | 変更後 stdio | 変更後 FastIO | 参考 offline |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `dense_00` | 19.098 | 20.716 | 38.402 | 39.628 | 25.553 | 15.641 |
| `dense_01` | 14.730 | 15.483 | 34.452 | 34.348 | 22.106 | 18.695 |
| `dense_02` | 15.606 | 16.315 | 33.529 | 34.419 | 22.184 | 17.699 |
| `example_00` | 0.004 | 0.004 | 0.691 | 0.675 | 0.682 | 0.660 |
| `link_and_cut_00` | 456.127 | 376.366 | 502.202 | 410.662 | 420.034 | 37.383 |
| `max_random_00` | 18.324 | 12.234 | 50.109 | 42.617 | 23.510 | 15.526 |
| `max_random_01` | 17.866 | 12.213 | 53.260 | 44.029 | 24.912 | 16.576 |
| `max_random_02` | 19.563 | 13.104 | 52.289 | 44.161 | 26.182 | 16.077 |
| `random_00` | 16.444 | 11.198 | 43.152 | 38.182 | 20.391 | 14.330 |
| `random_01` | 7.555 | 6.179 | 27.433 | 25.390 | 13.540 | 10.230 |
| `random_02` | 2.763 | 2.593 | 10.526 | 10.233 | 5.405 | 4.401 |
| `random_03` | 3.156 | 3.220 | 13.041 | 13.393 | 7.232 | 6.121 |
| `random_04` | 9.811 | 4.515 | 24.764 | 20.132 | 10.345 | 7.238 |
| `small_00` | 0.018 | 0.014 | 0.711 | 0.696 | 0.675 | 0.678 |
| `small_01` | 0.012 | 0.011 | 0.676 | 0.696 | 0.671 | 0.639 |
| `small_02` | 0.017 | 0.019 | 0.671 | 0.671 | 0.650 | 0.642 |
| `uv_swapped_00` | 17.776 | 12.105 | 50.205 | 42.682 | 23.847 | 16.312 |
| `uv_swapped_01` | 20.102 | 13.344 | 53.821 | 44.842 | 24.517 | 16.301 |
| `uv_swapped_02` | 18.558 | 13.227 | 61.015 | 46.131 | 24.824 | 16.280 |
| 各中央値の合計 | 657.533 | 532.861 | 1050.948 | 893.586 | 697.260 | 231.427 |

密な小グラフの `dense_*` は計算部分が数％増える場合がある。大きい頂点数でのランダム入力や `uv_swapped_*` は短縮した。対象ケースも他の入力より重く、オフラインの上位提出との差は残る。

`-march=native` を外した計算部分の追試でも、`link_and_cut_00` は 484.415 → 425.656 ms、約 12.1% 短縮した。全試行・checksum・ソースと公式入力のハッシュ・CPU とコンパイラは [native](results/dynamic_connectivity_linkcut/native/results.json)、[native 環境](results/dynamic_connectivity_linkcut/native/environment.json)、[generic](results/dynamic_connectivity_linkcut/generic/results.json) にある。計数用ビルドの実行時間を、この表の速度比較には使っていない。

## 試した案

探索は `abfcfcd99e4f4d0d9740a5add1b718193f690cc6` を比較元とし、各 5 回または 7 回測定した。両比較元の DynamicConnectivity は同一である。ウォームアップなしの候補選別であり、上の最終条件とは異なる。以下の値は各探索内の中央値、単位 ms。

| 案 | link_and_cut_00 の観測・判断 |
| --- | --- |
| flag 探索の根を引き継ぐ | 503.005 → 548.029。昇格する辺も増え、不採用 |
| 削除した辺の端点から探索 | 513.414 → 549.055。不採用 |
| 昇格前に同じ頂点の隣接辺を 16 本まで確認 | 513.414 → 502.223。明確な利得が小さく不採用 |
| 連結判定を 1 回の splay と現在の根で行う | 505.103 → 505.466。根を追跡するコードの追加に対し利益なし |
| 成分の辺 ID の XOR から代替辺を検査 | 545.743 → 598.514。集約の維持費用が大きく、dense でも悪化 |
| 親を 32 段まで読んで連結判定を省略 | 511.850 → 485.768。一貫した全体改善が小さく不採用 |
| 両側から境界を探して短い側で結合 | 500.246 → 512.010。不採用 |
| 右端ノードのキャッシュ | 500.246 → 520.804。不採用 |
| 二つ目の有向辺を先に splay してから外側を結合 | 515.367 → 527.576。不採用 |
| 木辺を左端・右端順に昇格、探索方向の変更 | 安定した改善なし |
| splay の根の集約をコピーして再計算を省略 | 単独で 518.362 → 492.187。遅延構築との組合せでは 424.379 → 430.802 で不採用 |
| 遅延構築のみ | 500.874 → 424.379。採用 |
| 遅延構築と切断順の変更 | 512.776 → 421.823。採用 |
| 遅延構築・切断順・28 byte ノード | 512.776 → 407.296。dense の悪化が大きいため、32 byte 配置を維持 |
| GCC O3 | 遅延構築では 438.675 → 437.996。追加 pragma を見送る |

[explorations.json](results/dynamic_connectivity_linkcut/explorations.json) に全試行・条件・候補スクリプトを保存した。候補スクリプトは `scripts` のキーの場所へ復元できる。初期の処理別回転数も同ファイルにある。

## 検証・再現

変更箇所と依存先に絞り、既存の BFS 差分・可換モノイド・並行辺・自己ループ・代替辺探索・コピー・200,000 頂点のパスを確認した。新しい回帰テストは、大きく離れた頂点番号、最初の辺追加前の値更新、孤立頂点の集約、非デフォルト構築型、ノード再利用を確認する。`NDEBUG` でも実行を省略しない。

GCC 15.2・GCC 13.4・Clang 21.1.8、Clang ASan/UBSan、GCC の `NDEBUG` で成功した。公式19ケースは GCC・Clang で全件成功。文書の再生成、結合ヘッダーのコンパイル、変更対象の API 表と log 表記の検査も成功した。[対象とソースハッシュ](results/dynamic_connectivity_linkcut/validation.json)、[単体](results/dynamic_connectivity_linkcut/check.log)、[公式ケース](results/dynamic_connectivity_linkcut/judge.log)、[文書](results/dynamic_connectivity_linkcut/docs.log) に記録する。Mac はリポジトリの Apple Clang CI で確認する。

```sh
python3 test/benchmark/dynamic_connectivity_linkcut_reference.py --cache .verify-helper/cache/dc-linkcut/reference --output test/benchmark/results/dynamic_connectivity_linkcut_reference.json
python3 test/benchmark/dynamic_connectivity_linkcut.py --output test/benchmark/results/dynamic_connectivity_linkcut/native
python3 test/benchmark/dynamic_connectivity_linkcut.py --generic --kernel-only --only link_and_cut_00 dense_00 max_random_00 uv_swapped_00 --output test/benchmark/results/dynamic_connectivity_linkcut/generic
python3 test/benchmark/dynamic_connectivity_linkcut_profile.py --output test/benchmark/results/dynamic_connectivity_linkcut/profile.json
python3 test/verify.py --unit-only --selection test/benchmark/results/dynamic_connectivity_linkcut/validation.json --cxx g++ clang++
python3 .verify-helper/ci.py judge --selection test/benchmark/results/dynamic_connectivity_linkcut/validation.json
```

公式 problem repository と入力・正答・checker を事前に生成する。配置が異なる場合は `--problem` を指定できる。`--only` で入力、`--kernel-only` で計算部分だけを選べる。各計測のバイナリと一時ファイルは終了時に削除する。
