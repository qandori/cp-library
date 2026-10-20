# DSU on Tree

2026-10-21。Linux x86_64 / WSL2、Intel Core i7-13700F、GCC15.2.0。

## 参照と方針

[ABC454 G の公式解説](https://atcoder.jp/contests/abc454/editorial/19112) は、最大の子の情報を残し、軽い子の情報を追加し直す DSU on Tree を説明している。
[Nyaan の実装](https://nyaannyaan.github.io/library/tree/dsu-on-tree.hpp) も参照した。
取得した固定コミット・ソース SHA256 は [参照記録](results/dsu_on_tree_reference.json) に保存した。
参照実装は前処理2回と実行を再帰で処理し、頂点ごとの削除と状態のリセットをコールバックで指定する。

本実装は重い子を最初に訪れる DFS 順を逆から走査する。
この順序なら軽い子を処理・破棄してから重い子の状態を引き継げるため、実行中の再帰やフレーム配列が不要になる。
軽い子全体は Euler 順の一つの連続区間として追加する。
前処理には既存の `UndirectedCSR` を使い、前処理中の DFS スタックも出力配列の未使用部分へ置く。
公開 API は頂点ごとの `run(add,remove,answer)` と、一括リセットする `run_clear(add,answer,clear)`。
根を残す指定、任意の根での再構築、部分木の区間・頂点列の取得にも対応する。

Library Checker に DSU on Tree 自体の問題はないため、検証には
[Vertex Add Subtree Sum](https://judge.yosupo.jp/problem/vertex_add_subtree_sum) を利用する。
取得時の時間順上位 [403571](https://judge.yosupo.jp/submission/403571) は34 ms、
[405058](https://judge.yosupo.jp/submission/405058) は35 msだった。
いずれも Euler 順と幅広のセグメント木を使い、一本道の特別処理や、親の番号が小さいという制約を利用している。
この問題の和には DSU on Tree より適しているが、一般の部分木の最頻値などを処理する代わりにはならない。
本ライブラリには入力の番号順や形に依存する特別処理を入れていない。

検証用提出では、各頂点の初期値・更新を時刻軸の Fenwick Tree に追加し、部分木の集計時にその頂点に付いた各時刻の問い合わせへ答える。
これは DSU on Tree の動作確認用であり、当該問題の fastest を狙う解法ではない。
参照提出の judge 時間と下のローカル時間は比較しない。

## 測定

`-std=c++23 -O2 -march=native -Wall -Wextra -Werror`、assert 有効、CPU2固定。
1回のウォームアップ後、20万頂点は5回、250万頂点は3回、実装順を混ぜて実行した。
下表は process CPU 時間の中央値、単位 ms。
乱数種は20261021。色の個数と、最多の色の種類数を全部分木について集計する。
木・色の生成と入出力は前処理・実行の計時から除外する。

「フレーム方式」は本資料用の比較実装で、同じ前処理・重い子・コールバックを使う。
軽い子・重い子・集計・破棄を明示スタックでスケジュールし、確認付きの公開アクセサで頂点・区間を得る。
実行時間にはスタックの確保と操作も含む。外部ライブラリそのものとの速度比較ではない。
`run_clear` は触れた色の個数と頻度配列を一括リセットする。
この集計では個別の逆操作より軽いが、利用者が書くコールバックによって結果は変わる。

| 頂点数 | 木 | 前処理 | フレーム方式 | `run` | `run_clear` |
| ---: | --- | ---: | ---: | ---: | ---: |
| 200000 | 一本道 | 3.88 | 2.773 | 1.964 | 1.719 |
| 200000 | 星 | 3.59 | 3.876 | 2.539 | 2.159 |
| 200000 | 平衡二分木 | 3.42 | 7.918 | 6.953 | 5.207 |
| 200000 | 箒 | 3.63 | 3.492 | 2.187 | 1.937 |
| 200000 | ランダム | 9.16 | 10.332 | 7.341 | 6.135 |
| 2500000 | 一本道 | 52.95 | 48.675 | 40.800 | 36.402 |
| 2500000 | 星 | 47.90 | 71.640 | 48.134 | 42.792 |
| 2500000 | ランダム | 287.56 | 432.728 | 303.412 | 237.515 |

前処理時間・実行時間・プロセス全体の wall 時間・最大 RSS・全試行・ソースと実行ファイルの SHA256 は
[生データ](results/dsu_on_tree.json) に保存した。
全方式で追加回数・回答のチェックサムが一致し、追加回数は部分木サイズから求めた式とも一致した。
この環境での測定であり、AtCoder 本番や Mac 上の速度は未測定。

## 検証

[検証記録](results/dsu_on_tree_validation.json) と [ログ](results/dsu_on_tree_validation.log) を保存した。

- 空・1頂点、辺順と頂点番号を入れ替えた600本のランダム木、任意の根で、集計時の有効頂点全体を愚直解と照合。
- `run`・`run_clear`、終了時の破棄・保持、再実行、根の変更、部分木区間・最大の子、move-only コールバックを確認。
- 20万頂点の一本道・星・平衡二分木を単体テストで確認。
- GCC・Clang、Clang ASan+UBSan、ヘッダー単体 include、NDEBUG、説明の両コード例を確認。
- Library Checker 公式16ケースを GCC・Clangの両方で照合。
- ABC454 G の公式3サンプルと、120個のランダム入力を愚直な最頻値と照合。
- ABC454 G の250万頂点・単色の入力を、逆順の部分木サイズ計算による独立解と照合。
- ドキュメント181件のメタデータと、追加1件の展開コードを確認。

GCCの成功済み単体テストはソース・実行ファイルのハッシュを記録して再利用し、変更のない範囲を再実行しなかった。
CPU 固有の命令を使わず、Clang の通常経路でも確認した。実機の macOS は未検証。

## 再現

Linux で GCC・Clang・Python・`test/requirements.txt` の依存を用意する。
前処理済み公式テストは検証スクリプト内の Library Checker キャッシュに置く。
検証スクリプトは、有効な GCC 単体検証記録がなければコンパイルと実行から確認する。
今回保存した最初の GCC 単体検証記録は、初回のコンパイルと実行結果をそのまま保存したもの。

```sh
python3 test/benchmark/dsu_on_tree_reference.py \
  --cache .verify-helper/cache/dsu-on-tree/reference \
  --output test/benchmark/results/dsu_on_tree_reference.json
python3 test/benchmark/dsu_on_tree.py \
  --work .verify-helper/cache/dsu-on-tree/benchmark \
  --output test/benchmark/results/dsu_on_tree.json
python3 test/benchmark/dsu_on_tree_validate.py
```

## CI の検証方法

AtCoder は[システムテストケースの公開を停止](https://atcoder.jp/posts/1377)しているため、ABC454 G の提出用コードは `dsu_on_tree_mode.cpp` に置き、`test/unit/dsu_on_tree_mode.cpp` で公開サンプル3件と小さい木の直接集計を検証する。既存の DSU on Tree 単体テスト・Library Checker の部分木和テストは維持する。これは AtCoder の非公開システムテストと同等の網羅性や AC を保証するものではない。上記の測定結果・ソースハッシュは測定時点の記録として保持する。
