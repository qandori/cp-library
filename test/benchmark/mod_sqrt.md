# 素数 mod の平方根

## 参考

2026-10-17 に Library Checker sqrt_mod の実行時間順の上位 AC を取得しました。

| 提出 | 記録 | 方法 |
| --- | ---: | --- |
| [405759](https://judge.yosupo.jp/submission/405759) | 13 ms | 30 bit 以下の法、32 bit Montgomery、AVX2 で 8 問を並列処理。拡大体の累乗と逆元による乱択法 |
| [169592](https://judge.yosupo.jp/submission/169592) | 13 ms | 同系統の AVX2 一括処理 |
| [169494](https://judge.yosupo.jp/submission/169494) | 22 ms | AVX2 一括処理 |
| [215966](https://judge.yosupo.jp/submission/215966) | 33 ms | スカラーの Cipolla 法、動的 Montgomery modint |
| [331256](https://judge.yosupo.jp/submission/331256) | 35 ms | スカラーの Cipolla 法、動的 Montgomery modint、blazingio |

judge の時間には入出力を含み、下の手元の測定値とは比較できません。原文の SHA256・取得時刻・順位一覧は `results/mod_sqrt_reference.json`、取得スクリプトは `mod_sqrt_reference.py` に保存しました。

## 実装

公開 API は `mod_sqrt(a,p)` です。`uint64_t` の任意の奇素数と 2 に対応し、平方根のうち小さい非負整数を返します。解がなければ `nullopt` です。法以上の入力も正規化します。素数判定と共有状態は持ちません。

- `p mod 4 = 3` は累乗 1 回と平方の照合で処理し、別の Euler 判定を省略。
- その他は `p-1=q*2^s` として `a^((q-1)/2)` から候補と `a^q` を共有計算。`a^q` の位数で解の有無も判定。
- `p mod 4 = 1` に対する平方剰余の相互法則で、2・3・5・7 が非平方剰余と分かる場合は探索を省略。それ以外は入力・法から seed を作る SplitMix64 で探索。
- 非平方剰余の 2 冪乗を配列に保存し、Tonelli–Shanks の補正で再利用。
- 位数の指数が 24 以上の場合は 2 群の離散対数を分割して補正。小さい場合は通常の補正ループ。
- 32 bit の法には独立した Montgomery32 を使用。法が `2^30` 未満の累乗では内部の値を `2p` 未満に保ち、最後だけ正規化。外部への返り値は常に法未満。
- 32 bit を超える法には既存の Montgomery64 を使用。`p+1` を直接作らず、64 bit 上限付近の法にも対応。

非零の平方剰余について `r=a^((q+1)/2)`、`b=a^q` とすると `r^2=a*b` です。非平方剰余から作った `g` の位数は `2^s` です。`b` の位数が `2^k` のとき、`b=(g^(2^(s-k)))^e` の `e` を求め、`r` に `g^(-e*2^(s-k-1))` を掛けて補正できます。平方剰余では `k<s` なので負のシフトは発生しません。

離散対数は指数の下位半分を累乗で求め、その寄与を逆元で取り除いて上位半分を求めます。計算量は `T(k)=2T(k/2)+O(k)`、全体は期待 `O(log p+s log s)`、作業配列と再帰の空間は `O(log p)` です。小さい非平方剰余が見つからない場合の探索を期待時間として数えています。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を比較しました。参照提出中の既存の GCC target 指定はそのままです。405759 は AVX2・BMI・BMI2 を使います。

全実装に同じ通常ファイルを標準入力として渡します。405759 の原文は mmap を前提としているため、パイプは使用しません。計測はプロセス起動・入力・計算・出力・終了を含む壁時計時間です。ウォームアップ 1 回の後、順序を混ぜた 7 回の中央値を採用しました。コンパイルや他のテストを計測中に実行していません。

`library` は今回の実装、`Tonelli64` は改善前の通常の Tonelli–Shanks と Montgomery64、`Cipolla` は同じ Montgomery 部品・Fast I/O を使う比較用スカラー実装です。後者 2 個は `mod_sqrt.cpp` と `mod_sqrt_baseline.hpp` に保存しています。参考提出は原文でビルドしました。

| 入力 | 問い合わせ数 | 条件 |
| --- | ---: | --- |
| mixed31 | 100,000 | `10^8 <= p < 10^9` の素数 256 個から選択 |
| mod4_3 | 100,000 | 同じ範囲で `p mod 4 = 3` |
| ntt998 | 100,000 | `p=998244353` |
| full32 | 100,000 | `p=3221225473`、`v2(p-1)=30` |
| mixed64 | 20,000 | 4294967311、9223372036854775783、18446744069414584321、18446744073709551557 |
| two_adic64 | 20,000 | `p=18446744069414584321`、`v2(p-1)=32` |

入力の 1/4 は乱数の平方、1/4 は `p-1`、残りは一様な剰余です。入力 seed は 2126432、測定順序 seed は 28063 です。Python 整数の累乗で平方剰余かを独立に判定し、全返り値の平方を照合しました。参考提出と根の選び方が違っても、小さい方へ正規化した出力ハッシュが一致することを各実行で確認しています。

## 結果

単位は ms です。30 bit の対応範囲を超える条件は参考提出を実行していません。

| 条件 | 入力 | library | Tonelli64 | Cipolla | 405759 AVX2 | 215966 | 331256 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| generic | mixed31 | 20.95 | 22.72 | 57.23 | 17.91 | 37.41 | 37.49 |
| generic | mod4_3 | 19.28 | 19.30 | 49.35 | 17.16 | 33.09 | 33.27 |
| generic | ntt998 | 35.48 | 40.20 | 52.37 | 19.66 | 36.28 | 36.62 |
| generic | full32 | 46.22 | 51.62 | 51.41 | — | — | — |
| generic | mixed64 | 10.59 | 11.44 | 23.17 | — | — | — |
| generic | two_adic64 | 15.62 | 18.85 | 25.65 | — | — | — |
| native | mixed31 | 20.82 | 22.63 | 54.76 | 17.37 | 36.04 | 35.76 |
| native | mod4_3 | 17.77 | 18.59 | 47.35 | 16.58 | 31.33 | 31.14 |
| native | ntt998 | 35.39 | 39.62 | 50.03 | 18.87 | 34.30 | 36.62 |
| native | full32 | 46.67 | 51.75 | 52.03 | — | — | — |
| native | mixed64 | 10.94 | 11.57 | 23.23 | — | — | — |
| native | two_adic64 | 16.17 | 19.32 | 26.10 | — | — | — |

この入力では改善前の Tonelli64 と同程度～約 1.2 倍で、上位のスカラー Cipolla と同程度または高速でした。ただし native の ntt998 では 215966 が速く、AVX2 一括処理を超える結果には至っていません。最速とは主張しません。

初期版の通常の Tonelli–Shanks と 32 bit 切り替えだけの測定も `results/mod_sqrt_initial_generic.json`・`mod_sqrt_initial_native.json` に残しました。32 bit へ変えるだけでは十分な改善にならなかったため、遅延累乗・非平方剰余の選択・大きな 2 群の補正を追加しました。分割を位数の指数 8 以上に適用する案は ntt998 で遅く、24 以上に限定しました。

最終版の生データ・全試行・ハッシュ・環境は `results/mod_sqrt_generic/results.json` と `results/mod_sqrt_native/results.json` にあります。

## 検証

- 1500 以下の全素数・全剰余を、小さい方の根の総当たり表と照合。
- 32 bit 上限を超える法、`2^63` より大きい法、最大の 64 bit 素数、高い 2 冪因子の法を確認。
- 平方を作った入力と、一様な 64 bit 入力を 128 bit の `%` による独立した累乗・Euler 判定と照合。
- Montgomery32 は合成数を含む 30,000 個の奇数法で変換・積・和・累乗・縮約を確認。指数 0・`UINT32_MAX` も使用。
- GCC 15・GCC 13・Clang の unit test、ヘッダー単独・併用、ASan・UBSan、Library Checker の公式 13 ケースを確認。Mac は既存の portability CI で確認します。

## 再現

AVX2・BMI2 に対応した Linux x86_64 で実行します。

```sh
python3 test/benchmark/mod_sqrt_reference.py --cache /tmp/mod-sqrt-references --output /tmp/mod-sqrt-references.json
python3 test/benchmark/mod_sqrt.py --references /tmp/mod-sqrt-references --output /tmp/mod-sqrt-generic
python3 test/benchmark/mod_sqrt.py --references /tmp/mod-sqrt-references --native --output /tmp/mod-sqrt-native
```

`--cpu`・`--trials` で CPU と試行数を指定できます。公開ヘッダー自体は SIMD 命令や GCC の target 指定に依存しません。
