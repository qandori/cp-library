# Wavelet Matrix 高速化

2026-10-19。通常の索引を8分岐へ変更し、AVX2による並列 rank と最終段の省略を採用した。構築時のソート結果の再利用、個数検索の出現位置列の探索、任意の `prepare_count()` も追加した。既存 API は維持している。

以下は手元の Linux x86_64 における測定であり、AtCoder・Library Checker 実機の順位や最速を保証するものではない。

## 最終ローカル測定

合成入力の問い合わせ数は50000。単位は ms、9回の中央値。問い合わせの行では構築を含めず、和の問い合わせは和の準備も計時外へ置いた。全118条件の試行値・checksum は [native](results/wavelet_matrix_opt/native/results.json)、ソース hash と実行条件は [environment](results/wavelet_matrix_opt/native/environment.json) に保存した。

| 要素数・値 | 操作 | 旧版 | 今回 | 時間の変化 |
| --- | --- | ---: | ---: | ---: |
| 200000・64 bit 型、0以上10億未満 | 構築 | 7.452 | 5.876 | −21.1% |
| 同上 | kth | 5.657 | 4.489 | −20.6% |
| 同上 | upper 未満の個数 | 7.771 | 6.774 | −12.8% |
| 同上 | 和を含む構築 | 13.997 | 12.424 | −11.2% |
| 200000・256種類 | kth | 1.865 | 1.462 | −21.6% |
| 同上 | upper 未満の個数 | 1.573 | 0.960 | −38.9% |
| 200000・4096種類 | kth | 3.071 | 2.476 | −19.4% |
| 同上 | upper 未満の個数 | 2.446 | 1.336 | −45.4% |
| 200000・64 bit 全域 | 構築 | 8.447 | 7.311 | −13.4% |
| 同上 | kth | 5.796 | 5.030 | −13.2% |
| 1000000・64 bit 型、0以上10億未満 | 構築 | 51.185 | 39.472 | −22.9% |
| 同上 | upper 未満の個数 | 16.508 | 18.144 | +9.9% |

500000要素・250000種類の整数での `count` は、未準備の今回が3.420 ms、`prepare_count()` 後が1.390 ms、旧版が3.000 msだった。準備時間は2.026 ms。問い合わせ数が少ない場合は準備費用を回収できないため、通常構築にハッシュ表を含めていない。準備後の `select` は旧版2.551 msから0.843 msとなった。

改善しない条件もある。500000要素・256種類の未準備 `count` は0.987→1.279 ms、200000要素・256種類の `sum_smallest` は4.719→5.100 ms、広い値域の `sum_largest` は10.092→10.836 msだった。和の問い合わせを全て高速化したものではない。

1000000要素・64 bit 全域の kth は、最初の9回の中央値が12.752→18.527 msだった。今回の試行範囲は11.569〜25.549 msで、同規模の0以上10億未満の入力は12.477→12.520 msだった。8分岐の索引領域の増加は大規模入力で不利になる可能性があるが、この測定の差の原因は確定していない。

この差を確認するため、ソースを変えず3条件だけ15回で再測定した。[再測定](results/wavelet_matrix_opt/recheck_large.json) の中央値は、0以上10億未満が12.321→11.839 ms、256種類が2.474→2.420 ms、64 bit 全域が12.147→11.961 msだった。最初の結果を除外せず両方を残す。大規模入力の安定した高速化までは確認できていない。

rank 領域は、4分岐が1段0.5 byte/要素、8分岐が1段2 byte/要素。値が1000000種類なら約5 MBから約14 MBへ増える。構築時の一時領域は圧縮しており、1000000要素・0以上10億未満の構築におけるプロセス最大RSSは63444→55416 KiBだった。最大RSSには元配列・問い合わせ配列・構築用の一時領域も含む。

`-march=native` なしの32条件も [generic](results/wavelet_matrix_opt/generic/results.json) に記録した。runtime dispatch によりAVX2対応CPUではこのビルドでもAVX2を使う。非AVX2経路の正答は別途 scalar 強制ビルドで検証する。

## 公式入力と上位提出の比較

公式45ケースの出力は全て一致した。kth の24ケースでは、構築と問い合わせを同じ FastInput・uint32 型・計時範囲で #402999 の online core とも比較した。[全69行の結果](results/wavelet_matrix_opt/official/results.json)、[条件・hash](results/wavelet_matrix_opt/official/environment.json) に保存している。

構築＋問い合わせの中央値、単位 ms：

| Range Kth Smallest | 旧版 | 今回 | online #402999 |
| --- | ---: | ---: | ---: |
| `max_random_00` | 25.291 | 20.885 | 19.880 |
| `max_random_01` | 25.568 | 21.364 | 20.244 |
| `max_random_02` | 25.555 | 21.107 | 19.949 |
| `max_random_03` | 25.428 | 20.709 | 20.377 |
| `max_random_04` | 25.630 | 20.907 | 20.064 |
| `random_03` | 7.959 | 6.170 | 5.860 |
| `random_04` | 6.131 | 4.822 | 4.848 |

最大ランダム5ケースで旧版より16.4〜18.6%短縮し、online #402999との差は1.6〜5.8%だった。`max_random_00` の構築単独は5.572→4.744 ms、問い合わせ単独は19.653→16.141 ms、参照はそれぞれ4.539・15.200 ms。ここでの構築・問い合わせ・合計はそれぞれ別に中央値を取っている。

入力から出力までの whole は次の通り。旧 stdio は従来の `cin`・`cout`、共通 FastIO は今回の verification driver を両方の WM に適用したもの。kth・frequency の FastIO driver は uint32 型を使う。参照の列は各提出自身の I/O を含む。

| 問題・ケース | 旧版＋旧 stdio | 旧版＋共通 FastIO | 今回＋共通 FastIO | 参照＋専用 I/O |
| --- | ---: | ---: | ---: | ---: |
| kth `max_random_00` | 52.784 | 36.376 | 38.144 | 25.305（online） |
| kth `dense_small_a_00` | 14.492 | 5.708 | 5.252 | 4.314（online） |
| frequency `random_03`・2種類 | 73.643 | 28.072 | 30.550 | 89.148（online） |
| frequency `random_04`・707種類 | 98.629 | 55.081 | 63.942 | 54.299（online） |
| frequency `random_05`・250000種類 | 116.137 | 69.917 | 51.435 | 75.628（online） |
| sum `max_random_00` | 486.878 | 425.938 | 414.417 | 39.318（offline） |

frequency の今回には `prepare_count()` の時間を含む。250000種類では同じ FastIO の旧版より26.4%短縮した。一方、707種類は16.1%、kth `max_random_00` は4.9%遅く、独立した kernel の改善がそのまま streaming I/O loop の速度になるとは限らなかった。offline の sum 上位提出にも大きな差が残る。全クエリを先読みする専用アルゴリズムと、独立した問い合わせを提供する WM の差を含む結果である。

## 比較した Library Checker 提出

2026-10-19 JST に公開 v3 API で、対象3問題の AC 提出を実行時間順に各100件取得した。掲載時間は取得時の値であり、以下の投稿日は UTC。全39ソースの SHA-256、投稿日、取得日時、上位一覧は [参照データ](results/wavelet_matrix_opt_reference.json)、再取得と online WM の core 抽出は [取得スクリプト](wavelet_matrix_opt_reference.py) に保存した。

| 問題 | 提出 | 投稿日 UTC | 掲載時間 | 実装と比較上の条件 |
| --- | --- | --- | ---: | --- |
| Range Kth Smallest | [403008 / nandhagk](https://judge.yosupo.jp/submission/403008) | 2026-09-16 18:52:35 | 20 ms | offline。圧縮後、1段分の prefix を全クエリで共有し、AVX2 でクエリ群を処理。全段の WM を保持しない |
| Range Kth Smallest | [393928 / chaihf](https://judge.yosupo.jp/submission/393928) | 2026-08-16 13:12:00 | 35 ms | binary WM の全クエリ一括処理。段を外側ループにし、64 bit bitmap と32 bit prefix を使用 |
| Range Kth Smallest | [402999 / nandhagk](https://judge.yosupo.jp/submission/402999) | 2026-09-16 18:13:40 | 40 ms | **online の8分岐 WM**。32要素ごとの累積 mask と累積 prefix、AVX2 rank。独立した kth API の主な比較対象 |
| Range Kth Smallest | [208756 / oldyan](https://judge.yosupo.jp/submission/208756) | 2024-05-13 05:38:09 | 82 ms | online wavelet tree。値ソート後の添字列の merge から rank bitmap を構築し、quantile を個別に処理 |
| Range Kth Smallest | [332152 / nandhagk](https://judge.yosupo.jp/submission/332152) | 2025-11-27 08:13:00 | 85 ms | online binary WM。12 byte の packed block、BZHI と POPCNT。要素数200000・18段を固定 |
| Static Range Frequency | [362490 / IceKylin](https://judge.yosupo.jp/submission/362490) | 2026-03-26 12:19:51 | 46 ms | offline。全クエリの左右端点を位置で bucket 分けし、左から hash の出現回数を更新して prefix 差を回答 |
| Static Range Frequency | [205664 / oldyan](https://judge.yosupo.jp/submission/205664) | 2024-05-02 19:08:10 | 129 ms | online。hash で値ごとの出現位置列を取得し、lower_bound 2回で数える。count 専用 |
| Static Range Sum with Upper Bound | [402379 / nandhagk](https://judge.yosupo.jp/submission/402379) | 2026-09-14 17:34:05 | 58 ms | offline。値順に要素とクエリを radix sort し、AVX2 の16分木に count と sum を格納。値・要素数の上限を使って64 bit に pack |
| Static Range Sum with Upper Bound | [393944 / chaihf](https://judge.yosupo.jp/submission/393944) | 2026-08-16 13:21:28 | 81 ms | offline。値順の radix sort と Fenwick tree による count・sum の sweep |

offline の上位提出は全クエリの先読みでメモリ配置と処理順を変えている。今回の WM は構築後に独立した問い合わせを受け付けるので、online の提出と比較し、offline 提出は別の手法として記録した。各提出の掲載時間には専用 I/O、対象命令、固定幅・固定サイズなどの条件が含まれる。

## 方法の比較

従来の4分岐 WM は、64要素ごとに low/high bitmap と4種類の prefix を32 byteで保持する。今回参考にした8分岐配置は、32要素ごとに「digit が各閾値以下」の32 bit mask を8個、対応する累積 prefix を8個置き、1 block を64 byteに揃える。AVX2 の nibble lookup と multiply-add で8個の部分 popcount を同時に求め、左右の累積個数を k と比較して次の digit を選ぶ。最終段では添字の再マップを省き、同じ word に収まる区間では prefix も参照しない。

1段の rank 領域は従来の0.5 byte/要素から2 byte/要素へ増える一方、1段で3 bit処理するため依存する段数が減る。単純に段数だけで有利とは判断せず、値数・要素数・区間長を変えて比較した。非 AVX2 環境では同じ配置を scalar の3回の二分判断で参照する。符号付き整数、64 bit、比較型、小区間、整列済み入力への対応を維持する。

和は4分岐の前処理を使う。`WaveletMatrix(a,true)` は最初から和向けの構造を作り、通常構築から `prepare_sum()` を呼んだ場合も、以後の問い合わせを4分岐へ切り替えて8分岐の各レベルの rank 領域を解放する。和に必要な prefix 数、構築時間、kth の依存段数を一つの配置へ無理に統一せず、和の利用を境に処理を選ぶ。

整数の構築では、radix sort 済みの添字列を出現位置列へ流用する。安定な LSD sort なので、同じ値の出現位置は元の添字順を保つ。キーから最小値を引くと11 bit radix のパス数が減る場合にだけ引き、64 bit key と32 bit index の一時要素は12 byteへ詰める。異なる値の配列の余剰容量は、要素数の2倍を超えたときに縮める。

`prepare_count()` は64 bit以下の整数の exact lookup 用の表を明示的に準備し、`count` と `select` の辞書検索を短縮する。順序境界を求める `range_freq` などは従来の比較検索を使う。通常構築では表を確保せず、異なる値が1024個以下または連続した整数値域の場合も表を作らない。高頻度値の count は、値数と出現回数に応じて WM の rank または出現位置列の二分探索を使う。

## 測定条件

AtCoder の現在の GCC 設定は [公式言語一覧](https://img.atcoder.jp/file/language-update/2025-10/language-list.html) で確認した。C++23/GCC 15.2.0 の主要な対象オプションは `-O2`、`-std=gnu++23`、`-march=native`、`-ftrivial-auto-var-init=zero`、`-DATCODER`、`-DONLINE_JUDGE`。GCC の LTO は2026-06-15に外されたという [公式告知](https://atcoder.jp/posts/language_update20260616_en) も確認した。確認日は2026-10-19 JST。

ローカル測定は Linux x86_64/WSL、Core i7-13700F、GCC 15.2.0、CPU affinity 2。上記の主要オプションで native と、`-march=native` を外した generic を測る。GCC 13・Clang・Mac は正答と移植性の確認にも使う。実際のコンパイラ、CPU情報、フラグ、各ソースと入力の SHA-256 は測定結果の environment/validation に保存する。AtCoder・Library Checker の実機で未測定の実行時間については主張しない。

最終比較は旧版 `523956fbb612ed975d73d1fcac414238f8ec2d16` と今回のソースを同じ条件でコンパイルする。各ケースは1回 warmup 後に9回測り、実行順を固定 seed で shuffle して中央値を使う。重い処理は `/tmp/cp-wm-opt.lock` で直列化する。

- 合成入力では生成と対象外の前処理を計時外へ置き、構築・kth・count・和などを分ける。結果を使う checksum 更新は問い合わせのループ内に含める。
- 公式 kth の kernel 比較は、旧版・今回・#402999 core を同じ uint32 入力と FastInput で実行する。入力解析と領域確保後に構築・問い合わせを計り、checksum と出力は計時外へ置く。参照の専用 I/O を使った提出全体の時間と区別する。
- 公式 whole 比較は入力解析・構築・問い合わせ・出力・プロセス起動を含む。旧 I/O と共通 FastIO を別の列に置き、同じ I/O の列で本体の差を比較する。出力は Linux の一時ファイルに書き、公式 checker で照合する。frequency の今回の driver は `prepare_count()` も実行する。
- 問題リポジトリは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。kth 24ケース、frequency 11ケース、sum 10ケースを使用する。frequency の最大3ケースは n=q=500000、異なる値の数は2・707・250000で、重複度による差も確認できる。

探索段階の pair 数、繰り返し数、タイマー範囲は各 raw record に従う。特に count 探索の whole は `/dev/null` 出力であり、最終 whole の出力先と異なる。過去の試作比率を最終ソースの測定値へ混ぜない。

## 不採用案

| 案 | 判断 |
| --- | --- |
| 全クエリを先読みして段ごとに一括処理 | online API と前提が異なるため、汎用 WM の置き換えには使わない |
| 和も8分岐・7行の prefix へ統一 | 和の前処理や部分和に退行が残り、和には4分岐を選んだ |
| 和のために8分岐と4分岐を両方保持 | 通常の問い合わせも和向けの構造で処理できるため、和準備後は8分岐の rank 領域を解放する方針を選んだ |
| 要素数だけの閾値で4分岐と8分岐を切り替える | 値数や操作によって傾向が変わり、単一のサイズ閾値は採用しなかった |
| Quad route の三項式化、累積 prefix、和の scalar 3 bit 判定、SIMD register-permute | 安定した速度利益が得られず、通常 Oct route の小配列による lane 選択を維持した |
| 小さい値域だけ exact lane を使う、rank helper を変える、1段 interval を特化する | 小値域 count の改善が小さいか不安定で、広い値域の freq/kth が退行した |
| native 時だけ Quad helper を強制 inline | 改善が一貫せず、通常の8分岐呼び出し側から和向け helper のコードを分離する方針を選んだ |
| native 時だけ kth の private/public 関数を強制 inline | 公式2ケースの whole/kernel で効果が混在した。現行 private callee の命令列は whole/kernel で同一で、I/O loop との差の原因は確定しなかった |
| count の hash を通常構築で常に作る | kth や構築だけの用途にも時間・メモリを使うため、明示的な準備 API を選んだ |

小値域では8分岐 rank の領域が4分岐の4倍になるため、命令数だけを減らしても差を埋めにくい可能性がある。cache miss のカウンタは測っておらず、これは要因の推測として扱う。

再現用の試作ソース・生成スクリプト・生の測定値は [rank](results/wavelet_matrix_opt/explorations_rank.json)、[構築](results/wavelet_matrix_opt/explorations_build.json)、[backend 選択](results/wavelet_matrix_opt/explorations_backend.json)、[小値域 rank](results/wavelet_matrix_opt/explorations_small_rank.json)、[値・出現位置の探索](results/wavelet_matrix_opt/explorations_lookup.json)、[count 準備](results/wavelet_matrix_opt/explorations_count.json)、[I/O loop と inline](results/wavelet_matrix_opt/explorations_io_inline.json) に保存した。共通 driver と復元ソースの区別は [探索資料の監査](results/wavelet_matrix_opt/explorations_manifest.json) に記録した。

`prepare_count()` の表は load factor を最大0.5とし、code+1だけを32 bitで保持する。表の追加メモリは `4 * bit_ceil(2 * σ)` byteで、今回のσ=250000では2 MiB。乱数化した splitmix64 を使うが、hash lookup は期待 $$O(1)$$・最悪 $$O(\sigma)$$、表の構築は期待 $$O(\sigma)$$・最悪 $$O(\sigma^2)$$。`count` の区間処理の計算量は別に加わる。`build` は表を無効化するが、確保容量は保持するため、再利用するオブジェクトのメモリは過去の値数にも依存する。

## 検証・再実行

GCC 15.2・13.4 で対象ヘッダー単体と WM・Dynamic WM の unit、Clang 21 で ASan/UBSan・NDEBUG・scalar 強制構成を確認した。境界長、8の冪付近の値数、signed/unsigned 64 bit、128 bit 和、比較型、bool、lazy/eager sum、prepared count、copy/rebuildを含む。公式45ケースは GCC・Clang の両方で成功した。

記録は [GCC・公式テスト](results/wavelet_matrix_opt/validation.json)、[Clang](results/wavelet_matrix_opt/validation_clang.json)、[count追加テスト](results/wavelet_matrix_opt/validation_count.json)、[独立レビュー](results/wavelet_matrix_opt/review_rank_final.json)、[docs・bundle](results/wavelet_matrix_opt/validation_docs.json)。変更のない対象は依存ソースの hash が一致する成功結果を再利用した。

```sh
python3 test/benchmark/wavelet_matrix_opt_reference.py --top 0 --extra 402999 205664 402379 --cache .verify-helper/cache/wm-opt/reference --output .verify-helper/cache/wm-opt/reference/metadata.json
python3 test/benchmark/wavelet_matrix_opt.py --output test/benchmark/results/wavelet_matrix_opt/native
python3 test/benchmark/wavelet_matrix_opt.py --generic --quick --output test/benchmark/results/wavelet_matrix_opt/generic
python3 test/benchmark/wavelet_matrix_opt.py --suite official --output test/benchmark/results/wavelet_matrix_opt/official
```

CPU 2と対応する公式入力・checker、Python 3、GCCが必要。探索段階の初期 dual-sum 比較は、最後の結果保存時のファイルエラーで中断したため [初期版](results/wavelet_matrix_opt/initial_dual/environment.json) として区別した。採用版の native・generic・official は全て完了し、前後のソース hash も一致している。
