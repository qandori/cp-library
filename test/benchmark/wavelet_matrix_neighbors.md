# Wavelet Matrix の順位選択と predecessor / successor

## 変更

公開 API と格納データは変更しない。`prev_value` は上限未満、`next_value` は下限以上の値を返す従来の意味を維持する。

従来は `range_freq` で順位を求めてから `kth_smallest` を呼び、同じ quaternary index を二度たどっていた。新実装は圧縮した境界値の経路を直接たどる。途中で対象区間が空になった場合だけ祖先へ戻り、最も近い利用可能な兄弟から最大値または最小値へ降りる。各階層で試す兄弟は高々三つで、計算量は引き続き O(log σ)。一時領域は最大16階層の固定長配列で、永続的なメモリ増加はない。

空・定数列は直接判定する。長さ8以下の区間の predecessor / successor は圧縮コードを直接走査し、整列を省く。昇順・降順列には既存の専用処理を残す。整数に限らず比較可能な型を扱い、x86 固有命令の追加はない。

quaternary の順位選択も、上位 bit の判定後に下位 bit を数える直列の経路から、digit 0 / digit 0–1 / digit 0–2 の累積数を並列に計算する経路へ変更する。popcount は増えるが依存の連鎖を短くする。sum の選択では少数の値からなる列で逆効果だったため、コンパイル時の分岐で従来の経路を維持する。prefix sum の格納方法も変えない。

## 比較条件

- 比較元: `e476d2233f097421c14ef2d768559d5d8e2b1837`。
- Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0。
- `-std=c++23 -O2` と `-std=c++23 -O2 -march=native`。
- 同じ実行ファイルに比較元と変更後を別クラス名で読み込み、各クエリ走査を交互に実行する。乱数生成、データ構築、sum 用の準備はクエリの計測外。
- 固定 seed 814253、20000 queries。主測定は `--uniform-bounds` で元の列の添字を一様に抽出した値を境界に使う（値域全体の一様乱数ではない）。境界は元の列に存在する値であり、元の列にない境界は正しさのテストで別途扱う。オプションなしでは二つの境界を整列した下側を使い、初期測定の lower-biased な分布を再現できる。各ペアで開始順を変え、CPU 時間と wall time、checksum を記録する。CPU 0 に固定した比較も行う。
- n=1024 / 200000 / 1000000、値域10^9の乱数・256値の重複列・64bit乱数。σ=2 / 4 / 16 の kth と sum、長さ8以下、昇順、降順、定数列と未変更操作も確認する。
- この CPU は AtCoder の実行機ではない。AtCoder 上の実測値、あるいは Mac 上の速度向上を示すものではない。

## 再現

```
mkdir -p test/benchmark/results/wavelet_matrix_neighbors
python test/benchmark/wavelet_matrix_neighbors.py --native --uniform-bounds --seconds 0.15 --pairs 9 > test/benchmark/results/wavelet_matrix_neighbors/native.jsonl
python test/benchmark/wavelet_matrix_neighbors.py --uniform-bounds --seconds 0.15 --pairs 9 > test/benchmark/results/wavelet_matrix_neighbors/generic.jsonl
```

Linux では先頭に `taskset -c 0` を付けると CPU を固定できる。`--candidate-first` でヘッダーの取り込み順を逆にしたコード生成も比較できる。比較元の取得には Git 履歴を使い、作業ディレクトリやリモートを変更しない。raw output は `.gitignore` の対象で、commit しない。

mode: 0=値域10^9の乱数、1=256値、2=符号付き64bit全域、3=昇順、4=降順、5=定数、6=2値、7=4値、8=16値。

operation: 0=build、1=kth_smallest、2=range_freq(upper)、3=range_freq(lower,upper)、4=prev_value、5=next_value、6=count、7=sum_smallest、8=sum_largest、9=range_sum、10=select、11=access、12=eager build、13=build+prepare_sum。

## 参考実装

Library Checker の公式提出を読み、オンライン処理として比較可能かを確認した。以下の judge time は I/O を含む別環境の値で、本ベンチマークとの速度比較には使用しない。

- [403008](https://judge.yosupo.jp/submission/403008)、0.020秒: AVX2 query frontier を用いるオフライン処理。
- [403000](https://judge.yosupo.jp/submission/403000)、0.031秒: 同じくオフライン処理。
- [393928](https://judge.yosupo.jp/submission/393928)、0.035秒: クエリを階層ごとにまとめる batched 処理。
- [402999](https://judge.yosupo.jp/submission/402999)、0.040秒: 独立したオンライン kth query。radix-8、累積 mask と AVX2 の並列 popcount を使う。現実装の quaternary index に対し、bit index 部分だけで概ね2.67倍のメモリを必要とし、同等の sum API はないため、そのまま置換していない。
- [362490](https://judge.yosupo.jp/submission/362490)、0.046秒: static range frequency を endpoint sweep と hash map で解くオフライン処理。

## 検証

既存の単体テストに加え、圧縮値の隙間、深い経路の巻き戻し、空区間、符号付き・符号なしの最大値、σ が4の累乗前後、64要素境界、bool・string・pair、lazy/eager sum 後のクエリを照合する。

## 2026-10-17 の比較結果

測定対象ヘッダーの SHA-256 は `17759549150c7c635e481be3857d92c4622a61c47add9824675a9ca49a7c4993`。各フラグ63条件、合計126条件を CPU 0 に固定して測定した。各条件9ペア、各実装の累積 CPU 時間がともに0.15秒以上になるまで交互に走査した。表は baseline / candidate の CPU 時間比で、1より大きいほど高速。条件ごとに9ペアの比の中央値を取り、集約には条件を等重みとした幾何平均を使う。

| 対象 | 条件数 / フラグ | native | generic |
|---|---:|---:|---:|
| kth、3サイズ × 3分布 | 9 | 1.253 | 1.161 |
| prev / next、3サイズ × 3分布 | 18 | 1.736 | 1.772 |
| kth、σ=2 / 4 / 16 | 3 | 1.551 | 1.263 |
| sum_smallest / sum_largest、6分布 | 12 | 1.005 | 0.991 |
| build・eager build・prepare_sum と他の未変更クエリ | 9 | 1.008 | 0.999 |
| 長さ8以下・昇順・降順・定数の prev / next | 12 | 1.237 | 1.288 |

n=200000 の主要クエリの内訳:

| 分布 | native kth | native prev | native next | generic kth | generic prev | generic next |
|---|---:|---:|---:|---:|---:|---:|
| 値域10^9 | 1.228 | 1.431 | 1.449 | 1.113 | 1.500 | 1.522 |
| 256値 | 1.427 | 2.902 | 2.879 | 1.260 | 2.690 | 2.630 |
| 64bit全域 | 1.223 | 1.449 | 1.445 | 1.074 | 1.563 | 1.454 |

native の kth 全9条件は1.000–1.492倍、prev / next 全18条件は1.223–2.902倍。大きな配列の kth はメモリアクセスの影響が強く、全条件で同じ改善幅になるわけではない。generic はそれぞれ0.935–1.346倍、1.228–2.690倍だった。

各バッチの CPU 時間の最小 / 中央値 / 最大は native 0.150 / 0.158 / 0.578秒、generic 0.150 / 0.159 / 1.030秒。共有環境で一時的な遅延があり、外れ値を削除していない。CPU 時間と wall time の両方を記録し、乱数・checksum の一致も確認した。

### 不利な条件とコード生成順の確認

主測定で5%を超える低下が出た条件も省略しない:

| フラグ | n / 分布 / 操作 | 主測定の比 | ペアの最小–最大 |
|---|---|---:|---:|
| native | 200000 / 値域10^9 / sum_largest | 0.911 | 0.307–1.103 |
| generic | 1000000 / 値域10^9 / kth | 0.935 | 0.856–1.059 |
| generic | 200000 / 256値 / sum_smallest | 0.933 | 0.899–1.062 |
| generic | 200000 / 昇順 / next | 0.931 | 0.329–1.254 |
| generic | 200000 / 降順 / prev | 0.945 | 0.843–1.237 |

事前に決めた native の9条件で、ヘッダー取り込み順を逆にした同一設定の測定も行った。n=200000 の kth は分布順に1.232 / 1.426 / 1.151倍、値域10^9 の prev / next は1.464 / 1.495倍、sum_smallest / sum_largest は0.996 / 1.026倍、短区間 prev / next は1.092 / 1.052倍。sum_largest の低下はこの確認では再現しなかった。

残る generic の不利な4条件は、一度だけ取り込み順を逆にし、0.3 CPU秒以上 × 15ペアで確認した。順に kth 1.009倍（0.845–1.266）、sum_smallest 1.001倍（0.925–1.097）、昇順 next 0.959倍（0.877–1.174）、降順 prev 0.982倍（0.935–1.058）だった。5%を超える中央値の低下はいずれも再現しなかった。ただし、これは全入力・全環境での非退行を保証するものではない。主測定の不利な数値を確認測定で置換せず、双方を残している。追加の反復で有利な値を選ぶことはしていない。

採用の判断基準は、主要対象で15%以上の幾何平均の改善と、代表条件で再現する5%超の低下がないこと。native の kth と prev / next はこの改善幅を満たした。sum・構築・frequency などの未変更機能の高速化は主張しない。

### 採用しなかった案

- 最初の直接 predecessor / successor は定数列や短区間で余分な経路処理が発生したため、そのままでは採用せず専用処理を加えた。
- 累積 rank の新しい route を sum にも使った案は、σ=2 / 4 / 16 で native 約0.85–0.90倍、generic 約0.76–0.86倍に低下したため却下した。採用版の sum は従来の route と同じ計算をコンパイル時に選ぶ。低い σ の sum の最終比は native 0.997–1.025、generic 0.984–1.002。
- 構築時の histogram 融合は native の代表3条件で0.970 / 1.035 / 1.003倍と十分な改善がなかったため採用していない。
- 条件分岐を展開した frequency の案も一貫した改善がなく採用していない。

### 正しさの確認

上記 SHA の版で GCC 14.2.0 のヘッダー単独コンパイルと Wavelet Matrix 全単体テスト（-O2）、ASan + UBSan（-O1、leak detection 無効）に合格した。別の adversarial suite でも、全順位の kth / sum、符号付き・符号なし64bit、正確に表現できる小数、深い backtrack、比較だけを提供する独自型、空・定数列を確認した。sum 用 route は比較元の処理と token 単位で一致することも確認した。Apple Clang / Mac と Linux の CI は PR の最新版で別途確認する。
