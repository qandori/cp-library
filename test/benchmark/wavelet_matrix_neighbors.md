# Wavelet Matrix の順位選択と predecessor / successor

## 変更

公開 API と格納データは変更しない。`prev_value` は上限未満、`next_value` は下限以上の値を返す従来の意味を維持する。

従来は `range_freq` で順位を求めてから `kth_smallest` を呼び、同じ quaternary index を二度たどっていた。新実装は圧縮した境界値の経路を直接たどる。途中で対象区間が空になった場合だけ祖先へ戻り、最も近い利用可能な兄弟から最大値または最小値へ降りる。各階層で試す兄弟は高々三つで、計算量は引き続き O(log σ)。一時領域は最大16階層の固定長配列で、永続的なメモリ増加はない。

空・定数列は直接判定する。長さ8以下の区間の predecessor / successor は圧縮コードを直接走査し、整列を省く。昇順・降順列には既存の専用処理を残す。整数に限らず比較可能な型を扱い、x86 固有命令の追加はない。

quaternary の順位選択も、上位 bit の判定後に下位 bit を数える直列の経路から、digit 0 / digit 0–1 / digit 0–2 の累積数を並列に計算する経路へ変更する。popcount は増えるが依存の連鎖を短くする。sum 用の経路情報は同じ累積数から再構成し、prefix sum の格納方法は変えない。

## 比較条件

- 比較元: `e476d2233f097421c14ef2d768559d5d8e2b1837`。
- Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0。
- `-std=c++23 -O2` と `-std=c++23 -O2 -march=native`。
- 同じ実行ファイルに比較元と変更後を別クラス名で読み込み、各クエリ走査を交互に実行する。乱数生成、データ構築、sum 用の準備はクエリの計測外。
- 固定 seed 814253、20000 queries。主測定は `--uniform-bounds` で元の列から独立に選んだ境界値を使う。オプションなしでは二つの境界を整列した下側を使い、初期測定の lower-biased な分布を再現できる。各ペアで開始順を変え、CPU 時間と wall time、checksum を記録する。CPU 0 に固定した比較も行う。
- n=1024 / 200000 / 1000000、値域10^9の乱数・256値の重複列・64bit乱数。長さ8以下、昇順、降順、定数列と未変更操作も確認する。
- この CPU は AtCoder の実行機ではない。AtCoder 上の実測値、あるいは Mac 上の速度向上を示すものではない。

## 再現

```
mkdir -p test/benchmark/results/wavelet_matrix_neighbors
python test/benchmark/wavelet_matrix_neighbors.py --native --uniform-bounds --seconds 0.15 --pairs 9 > test/benchmark/results/wavelet_matrix_neighbors/native.jsonl
python test/benchmark/wavelet_matrix_neighbors.py --uniform-bounds --seconds 0.15 --pairs 9 > test/benchmark/results/wavelet_matrix_neighbors/generic.jsonl
```

Linux では先頭に `taskset -c 0` を付けると CPU を固定できる。`--candidate-first` でヘッダーの取り込み順を逆にしたコード生成も比較できる。比較元の取得には Git 履歴を使い、作業ディレクトリやリモートを変更しない。raw output は `.gitignore` の対象で、commit しない。

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

## 測定状況

最終比較測定は実行中。これは correctness CI を先に実行するためのドラフトで、速度改善の採用判定と最終結果は未確定。CI が成功しても、この状態ではマージしない。
