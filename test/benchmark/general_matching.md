# General Matching のベンチマーク

2026-10-16 に [Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) の実行時間順の AC 提出を確認しました。最速は複数の提出が 2 ms で並んでいます。

- [404900](https://judge.yosupo.jp/submission/404900)・[386231](https://judge.yosupo.jp/submission/386231): DSU で Blossom を縮約する BFS 型の厳密解を確認しました。
- [88850](https://judge.yosupo.jp/submission/88850)・[71785](https://judge.yosupo.jp/submission/71785): 配列による隣接リストと、縮約後に base をまとめて更新する方式を確認しました。
- [384089](https://judge.yosupo.jp/submission/384089): 初期マッチングと長さ 3 の増加路による前処理を確認しました。

比較用の 404900 は `general_matching_reference.py` が API から一時取得します。コア比較ではテンプレート・入出力を除き、マクロを展開し、相手の頂点を `-1` に揃える薄いアダプターを付けます。入出力込みの比較では元の提出コードをそのままコンパイルします。元ソースの SHA-256 は実行 artifact に保存します。

## 再現方法

GitHub Actions の `general matching benchmark` は手動実行です。`verify` を無効にすると、変更のない本体の単体・sanitizer・公式テストを省き、合成グラフでの比較だけを行います。

```sh
python3 test/benchmark/general_matching_variants.py /tmp/matching-variants
python3 test/benchmark/general_matching_reference.py /tmp/matching-variants
g++ -std=c++23 -O2 -I . -I /tmp/matching-variants test/benchmark/general_matching.cpp -o /tmp/matching-bench
/tmp/matching-bench 500 5 20 > /tmp/matching.csv
```

引数は頂点数・試行数・試行内の反復数です。頂点ラベルと辺順を固定シードでシャッフルし、各試行で実装の実行順もシャッフルします。`exposed_path` は内部のマッチングを先に作り、長い増加路が残る順序にします。

構築・辺追加と `build()` を分けて測定し、両者の合計を比較します。入力生成・入出力・正解照合はコア計測から除きます。各反復で相手の対称性、実際の辺の存在、頂点の重複がないこと、組数を確認し、全実装の組数を照合します。

| グラフ | 内容 |
| --- | --- |
| `sparse` | 平均次数がおよそ 6 の乱数グラフ |
| `dense` | 各辺を確率 1/2 で追加 |
| `complete` | 完全グラフ |
| `odd_cliques` | 9 頂点ずつの完全グラフ |
| `star`・`path`・`tree` | 星形・パス・ランダム木 |
| `bipartite` | 左右同数の疎な二部グラフ |
| `bipartite_skew` | 片側が全体の 1/8 の二部グラフ |
| `blossom_chain` | 三角形を辺で連結 |
| `exposed_path` | 長い増加路を残すパス |

## 比較した変更

| 名前 | 内容 |
| --- | --- |
| `plain` | CSR と DSU を使う Blossom 法。初期マッチングなし |
| `greedy` | 辺追加中に初期マッチングを作り、全体の上限に達したら探索を省略 |
| `reverse` | 探索する根の順序を逆転 |
| `short` | 長さ 3 の増加路を 1 巡処理してから Blossom 法 |
| `short2` | 前処理を 2 巡 |
| `free_first` | CSR の走査で空き頂点を先に探す |
| `bitset` | 1024 頂点以下をビット集合で保持し、空き頂点との積を先に調べる |
| `bitset_short`・`bitset_short2` | ビット集合と短い増加路の前処理を組み合わせる |
| `components` | 連結成分ごとの上限を求め、二部成分は小さい側からだけ探索 |
| `components_short`・`components_short2` | 上記と短い増加路の前処理を組み合わせる |
| `current` | 現在の採用版 |

採用版は `components_short` を基に、不要な作業配列の確保も遅延させています。初期マッチングだけで全体の上限に達した場合は隣接リストを構築せず終了します。それ以外は短い増加路を処理し、CSR でグラフを保持します。二部成分では左右の小さい方、それ以外は成分の頂点数の半分を上限とします。上限が埋まらない場合は Blossom 法で厳密に解きます。

ビット集合は密なグラフを改善する場合がありましたが、星形・偏った二部グラフ・奇数クリークで遅くなりました。前処理 2 巡も安定した改善がなく、採用していません。

## 検証

6 頂点以下の全グラフと 18 頂点以下の乱数グラフを部分集合 DP と照合します。辺の追加後の再構築、コピー、自己ループ、多重辺、未マッチ頂点も確認します。4096 頂点までの長い増加路と連続する Blossom も検証します。
