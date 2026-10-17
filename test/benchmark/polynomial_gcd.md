# 多項式 GCD

## 参考提出・資料

2026-10-17 に Library Checker の上位 AC を取得した。多項式 GCD に直接対応する問題は見つからなかったため、拡張 GCD で解ける [Inv of Polynomials](https://judge.yosupo.jp/problem/inv_of_polynomials) を調査した。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [409589](https://judge.yosupo.jp/submission/409589) | 15 ms | Half-GCD の次数ごとの jump。行列の NTT キャッシュ、必要な成分のみの計算、巡回畳み込み、小さい入力での fraction-free な互除法。AVX2 と専用 I/O |
| [409587](https://judge.yosupo.jp/submission/409587) / [409586](https://judge.yosupo.jp/submission/409586) | 23 / 24 ms | 409589 とソースの SHA-256 が一致 |
| [406000](https://judge.yosupo.jp/submission/406000) | 104 ms | Half-GCD と行列の変換キャッシュ、AVX2 |

[maspy の実装](https://github.com/maspypy/library/blob/main/poly/poly_gcd.hpp) と [MIT の講義資料](https://people.csail.mit.edu/madhu/ST12/scribe/lect06.pdf) の Half-GCD も確認した。互除法のステップを多項式の 2×2 行列でまとめ、上位係数で再帰的に求める。

今回の実装は小さい入力を通常の互除法で処理し、大きい入力を Half-GCD で縮小する。行列積では各成分の NTT を共有する。行列の作用は、結果の次数と高次項の相殺を利用した巡回畳み込みで計算する。GCD 単独では全体の変換行列を蓄積せず、拡張 GCD では行列から Bézout 係数を取り出す。係数型が内部の modint と異なる場合は、入口・出口で変換する。

参考提出の一覧・詳細・ソースのハッシュは `results/polynomial_gcd_reference.json`。提出 406000 はローカルの GCC 15.2 でランダムな最大入力を実行すると SIGSEGV になり、スタック上限を unlimited にしても同じだったため、速度の比較対象から除いた。原因の特定やソースの修正はしていない。

```sh
python3 test/benchmark/polynomial_gcd_reference.py --cache .verify-helper/cache/polynomial-gcd-references --output test/benchmark/results/polynomial_gcd_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。seed 294384、各条件 7 回、実行順はシャッフル、各実装のウォームアップは 1 回。測定中は他のテストを実行していない。全プロセスのスタック上限は unlimited。

子プロセスの起動・入力・計算・出力・終了を含む実時間を測定した。コンパイル・入力生成・出力検査は時間外。全試行で出力のトークン列を照合した。GCD の通常の互除法は、同じ FPS の `divmod` を繰り返す基準実装である。大きいランダム入力の GCD は Half-GCD のみを測定し、同じ入力の逆元を上位 AC と照合した。

| 入力 | 内容 |
| --- | --- |
| random_256 / random_4096 | 係数数 N と N-1 のランダムな多項式 |
| random_50000 | 係数数 50000 と 49999 のランダムな多項式 |
| sparse_50000 | x^49999-1 と x^24999-1 |
| equal_50000 | 同じランダムな多項式を二つ |
| constant_50000 | ランダムな係数数 50000 の多項式と定数 7 |

generic は `-std=c++2b -O2 -DNDEBUG`。参考提出には AVX2 などが必要なため generic でも `-march=haswell` を追加する。native は全実装に `-march=native -ftrivial-auto-var-init=zero` を追加する。参考提出のソースは変更しない。以下は中央値、単位は ms。AtCoder 実機での速度は未測定。公開ジャッジの時間とローカル時間は直接比較しない。

| 条件 | GCD 入力 | Half-GCD | 通常の互除法 |
| --- | --- | ---: | ---: |
| generic | random_256 | 3.509 | 3.495 |
| generic | random_4096 | 6.716 | 18.899 |
| generic | random_50000 | 61.865 | 未測定 |
| generic | sparse_50000 | 5.809 | 5.220 |
| generic | equal_50000 | 5.735 | 6.002 |
| generic | constant_50000 | 4.904 | 4.934 |
| native | random_256 | 3.504 | 3.303 |
| native | random_4096 | 6.494 | 17.399 |
| native | random_50000 | 61.569 | 未測定 |
| native | sparse_50000 | 5.401 | 5.154 |
| native | equal_50000 | 5.744 | 5.738 |
| native | constant_50000 | 4.665 | 4.534 |

| 条件 | 逆元入力 | 今回 | 提出 409589 |
| --- | --- | ---: | ---: |
| generic | random_50000 | 96.560 | 16.675 |
| generic | sparse_50000 | 5.400 | 4.697 |
| generic | equal_50000 | 5.242 | 4.584 |
| generic | constant_50000 | 4.083 | 3.899 |
| native | random_50000 | 94.015 | 16.775 |
| native | sparse_50000 | 5.246 | 4.471 |
| native | equal_50000 | 5.080 | 4.616 |
| native | constant_50000 | 3.814 | 3.841 |

ランダムな 4096 係数では通常の互除法より generic で約 2.8 倍、native で約 2.7 倍速かった。小さい入力・疎な入力・整除関係では一貫した改善は見られなかった。逆元の大きいランダム入力では最上位の専用実装が速く、最速とは主張しない。

全試行・入出力ハッシュ・ソースのハッシュ・環境は `results/polynomial_gcd_generic/results.json` と `results/polynomial_gcd_native/results.json` に保存した。

```sh
python3 test/benchmark/polynomial_gcd.py --reference .verify-helper/cache/polynomial-gcd-references/409589.cpp --output test/benchmark/results/polynomial_gcd_generic
python3 test/benchmark/polynomial_gcd.py --reference .verify-helper/cache/polynomial-gcd-references/409589.cpp --output test/benchmark/results/polynomial_gcd_native --native
```

## 検証

GCC 15.2・Clang 21.1 と ASan/UBSan で、素朴な多項式除算・GCD、GCD の対称性、両方の入力を割り切ること、最高次係数の正規化、Bézout 恒等式、逆元の積を確認した。零多項式・非零の定数・末尾の 0・同一多項式・整除関係・共通因子・疎な多項式・2 冪前後のサイズを含む。異なる modint 型での変換も確認した。

Half-GCD の行列は、作用結果を通常の畳み込みでも計算して比較した。次数の低下量・後ろの多項式の係数数が半分以下になること・行列式が ±1 であることも検査した。

Library Checker Inv of Polynomials は公式 18 ケースを GCC・Clang の両方で通過した。GCC 13 でも単体テストを通過した。対象は新しいヘッダー・単体テスト・逆元の検証プログラムと、推移的に依存する standalone テスト。既存の FPS・NTT は変更していない。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/polynomial_gcd_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/polynomial_gcd_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/polynomial_inv_mod.test.cpp
```
