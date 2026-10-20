# 二項係数 prefix sum の差分更新と ARC190 B

2026-10-20。`IncrementalBinomialPrefixSum` の表をコピーで共有できるようにし、同じ表を使う `binom(n,k)` を追加した。差分更新も高速化した。

## 参考資料

- [対象の提出 80215048](https://atcoder.jp/contests/arc190/submissions/80215048)：取得時は AC、1788 ms、669356 KiB。ソースを取得して比較した。公開ジャッジ時間と以下のローカル時間は直接比較しない。
- [ARC190 B](https://atcoder.jp/contests/arc190/tasks/arc190_b)：上限は N=10000000、Q=200000。[公式解説](https://atcoder.jp/contests/arc190/editorial/11920) と [公式解答 61518705](https://atcoder.jp/contests/arc190/submissions/61518705) の累積値の差を取る方法を確認した。公式解答のソースもそのまま比較した。
- 直接対応する Library Checker 問題は確認できなかった。[既存の調査](binomial_prefix_sum_incremental.md) と [maspy の prefix_sum_of_binom](https://maspypy.github.io/library/mod/prefix_sum_of_binom.hpp) の Mo 法を確認し、同じ漸化式を比較した。
- ARC190 B の実行時間順の AC 一覧も取得を試みたが、サインイン画面が返った。最速提出を取得したとは扱わず、上の公式 AC を参考にした。取得情報とソースのハッシュは `results/binomial_prefix_incremental/references.json`。

## 固定の前計算

提出では、`COMB_MAX` と `INF` がともに 20002022。`init_comb()` が階乗・逆階乗を作り、`pre1(INF)` と `pre2(INF)` もそれぞれ階乗・逆階乗・2の累乗を作る。合計8本の配列を入力の大きさに関係なく作り、約640 MBになる。さらに `ans` の使用領域が加わる。サンプルの実行時間の大部分は、この前計算とメモリの確保・書き込みである。

取得したソースでは、`DEBUG` を定義しないと `cerr` が無効になり、`endl` は改行文字へ置換される。今回の遅さをデバッグ出力や flush のためとは判断しない。

入力後に `IncrementalBinomialPrefixSum<> pre1(n); auto pre2 = pre1;` とする。`pre1.binom(n,k)` から通常の二項係数も得られるので、別の `comb.hpp` は不要。前計算は実際の N までの1組だけになる。小さい入力で1000万個を作る必要はない。この方式で最大入力を処理する場合は、Nまでの前計算を1回行う。

コピー後の状態は独立し、表だけ共有する。`build` は新しい表を作って自分だけ切り替える。既存の別々のコンストラクタは、それぞれ前計算する動作を維持する。

## ライブラリの更新

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ として、従来の行・列の更新に加え、

$$S(n+1,k+1)=2S(n,k)+\binom nk$$

$$\displaystyle S(n-1,k-1)=\frac{S(n,k)-\binom{n-1}{k-1}}2$$

を使い、同じ方向に動く部分を一度に処理する。1列だけの移動は128 bitの区間累積を使わず、二項係数を直接加減算する。既定の Montgomery mint では、2で割る処理を内部の整数の偶奇補正と右シフトで行う。他の modint では従来の逆元との乗算を使う。

状態をローカル変数で更新し、`query` をインライン展開する。初期候補では関数が大きくなり、行だけ動く入力で約10〜20%遅くなったため、その形は採用していない。任意順のクエリに対応し、問題固有の入力分岐やクエリの並べ替えは加えていない。

## 提出コードの整理

`arc190_b_sweeps.cpp` は元の4走査を維持する。入力後の表の構築・共有、`binom` の再利用と、補正項を積にまとめる変更を行った。

`binomial_prefix_incremental_arc190_b.cpp` は累積値の差を使う。0始まりの座標 a,b に対し、

$$X_k=S(N-k,a+1)-S(N-k,a-k+1)$$

$$Y_k=S(N-k,b+1)-S(N-k,b-k+1),\qquad F_k=4^{k-1}X_kY_k$$

とし、答えを $$F_k-F_{k-1}$$、$$F_0=0$$ とする。指定された k と k-1 だけを処理し、連続するクエリの累積値は再利用する。4つの状態は同じ前計算を共有し、N個の答えの配列は持たない。クエリが昇順なので、状態の移動量の総和は $$O(N)$$。累乗の計算を含めて $$O(N+Q\log N)$$ 時間、$$O(N)$$ 空間となる。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。コンパイル・他の検証と測定を重ねず、各方式をウォームアップ1回、実行順をシャッフルして7回測定した。全試行で出力またはチェックサムを照合した。環境・全試行・フラグ・ソースのハッシュは `results/binomial_prefix_incremental/measurements.json`。

旧ライブラリは `555b183` のクラスを別名で保存し、新旧を同じ実行ファイルで比較した。共通の階乗・modint・二項係数の部品が同じ commit と一致することをスクリプトで確認する。旧版・新版の各2表を独立して作る条件と、新版で表を共有する条件を測定した。次は native の更新部分だけの中央値、ms。初期化は時間外とし、その時間も生データに保存した。

| 操作列 | 旧版 | 新版（独立表） | 新版（共有表） |
| --- | ---: | ---: | ---: |
| 行だけ増減 | 18.014 | 15.279 | 15.348 |
| 行・列を同じ方向に増減 | 34.625 | 23.011 | 22.774 |
| 同じ行で1列ずつ移動 | 16.077 | 11.162 | 11.689 |
| 小幅のランダム移動 | 72.338 | 64.960 | 64.697 |
| 大幅のランダム移動 | 18.534 | 14.901 | 14.884 |
| 元の4走査のクエリ列 | 44.015 | 30.788 | 30.575 |

大幅移動は N=10000 で2000クエリ。他は N=1000000。生成規則は `binomial_prefix_incremental.cpp` に置く。

次は native のプログラム全体の中央値、ms。プロセス起動・前計算・入出力・終了を含み、入力生成とコンパイルは含まない。`/usr/bin/time` の呼び出しも共通に含むため、特にサンプルの数msは呼び出しの固定費を含む。元の提出は未使用の `atcoder/all` の include だけ削除してコンパイルした。公式解答は元の cin/cout と固定上限を維持する。4走査版・累積差分版は同じ FastInput/FastOutput を使う。

| 入力 | 元の提出 | 4走査版 | 累積差分版 | 公式解答 |
| --- | ---: | ---: | ---: | ---: |
| sample 1 | 271.342 | 4.840 | 4.149 | 69.441 |
| sample 2 | 270.486 | 4.176 | 4.757 | 70.108 |
| sample 3 | 269.674 | 4.680 | 4.590 | 70.218 |
| N=10000000、中央付近 | 1154.640 | 591.951 | 150.976 | 344.268 |
| N=10000000、隅 | 1077.331 | 508.092 | 121.996 | 325.892 |

大きい入力は Q=200000、固定 seed で重複なく昇順の k を生成。中央は a=N/2,b=N/2+1、隅は a=1,b=N。新旧・公式解答の全出力が一致した。中央の最大使用メモリの中央値は元の提出651.86 MiB、4走査版157.49 MiB、累積差分版119.23 MiB。AtCoder 実機での新コードの時間、全システムケースの AC、全ての座標・クエリ分布に対する速度は今回確認していない。

## 検証

GCC・Clang の既存ユニットテスト、Clang ASan/UBSan、コピー・代入・片方の再構築・`binom` を確認。法2・3・17・101・1000000007、空の表、範囲外の k、全遷移・ランダム遷移を含む。両提出コードを、N<=6の全座標で分割方法を直接列挙した91ケースと、独立な二項係数の和による100ケースで照合した。GCC・Clang の Bundle の構文検査も通過した。

```sh
flock /tmp/cp-wm-opt.lock python3 test/benchmark/binomial_prefix_incremental.py
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/binomial_prefix_incremental/selection.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/binomial_prefix_incremental/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/binomial_prefix_incremental_check.py
```

## CI の検証方法

AtCoder は[システムテストケースの公開を停止](https://atcoder.jp/posts/1377)しているため、ARC190 B は外部ジャッジの自動取得ではなく `test/unit/binomial_prefix_incremental_arc190_b.cpp` で同じ提出用コードを検証する。公開サンプル3件、N<=6 の全座標・全分割列挙、連続・飛び飛びのクエリ、Q=200000 の隅の恒等式を使う。既存のライブラリ単体テストとともに GCC・Clang・ASan/UBSan の対象となる。これは AtCoder の非公開システムテストと同等の網羅性や AC を保証するものではない。上記の測定結果・ソースハッシュは測定時点の記録として保持する。
