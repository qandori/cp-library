# BigInt と Boost の比較

2026-10-17、自作 BigInt を改善しても固定長 Boost を上回れなかったため、指定に従って `math/bigint.hpp`・利用説明・専用テスト・README の項目を削除しました。No.3669 の Dijkstra テストは Boost の `cpp_int` を使います。

## 原因

[提出 1182839](https://yukicoder.me/submissions/1182839) は固定長の `uint512_t` で 197 ms、[提出 1185228](https://yukicoder.me/submissions/1185228) は可変長の `cpp_int` で 248 ms でした。後者も速いため、固定長かどうかだけでは説明できません。どちらも Boost の専用 `gcd` を使います。

No.3669 の共通分母 `lcm(1,...,300)` は 432 bit、十進 130 桁です。従来版はこのサイズでもヒープ確保が発生し、GCD では商を使わない剰余にも商・余りの両方を計算していました。正規化と一時配列を伴う多倍長除算を互除法の各段階で繰り返すコストが主因です。既存の AVX2 加減算は 32 limb 以上を対象とし、この問題の約 8 limb には使われません。

Boost 1.90 の [GCD 実装](https://github.com/boostorg/multiprecision/blob/boost-1.90.0/include/boost/multiprecision/cpp_int/misc.hpp) は Lehmer 法で複数回の互除法をまとめ、小さい値を通常の整数 GCD に落とします。2 進基数の桁演算も、この規模の算術では有利でした。

## 試した改善

- 配列の埋め込みを 2 limb から 8 limb に拡大。
- 上位 2 limb を正規化して使う Lehmer GCD と、128 bit 以下の binary GCD を追加。
- 小さい除算は浮動小数点で商を推定し、整数の加減算で両方向に補正。正規化・逆正規化を省略。
- Dijkstra のヒープから move で取り出し、挿入時のコピーを削減。`inf - cost` は頂点ごとに計算。

GCD に 128 bit の近似値を直接使う案も試しましたが、64 bit に正規化した案より遅く不採用でした。最終候補は `bigint_yuki_candidate.patch` に保存しています。対象入力の全出力を Boost・参考提出と照合しました。汎用ライブラリとしての全演算の検証は行わず、公開ライブラリには採用していません。Dijkstra の改善は採用し、整数・浮動小数点・128 bit・`cpp_int` で検証します。

## 測定条件

Linux x86_64 / WSL2、Intel Core i7-13700F、GCC 15.2、Boost 1.90.0、CPU 2 固定。汎用条件は `-std=c++2b -O2 -DNDEBUG`、native 条件はさらに `-march=native -ftrivial-auto-var-init=zero`。自作 BigInt 内の既存の GCC O3 指定も含みます。judge の実行時間とは比較できません。

No.3669 の制約内の連結な単純無向グラフで `N=30000, M=50000`、分子・分母は 1～300 の既約分数です。連結用の辺を鎖・ランダム木・星の 3 形状で作り、残りをランダムに追加します。固定シード `36691182839`、ウォームアップ 1 回、順序を固定シード `1185228` で混ぜた 7 回の中央値です。入力ハッシュ・出力ハッシュ・各試行・コンパイラ情報を `results/bigint_yuki_generic/results.json` と `results/bigint_yuki_native/results.json` に保存しています。

型を比較するドライバでは、Fast I/O、グラフ構築、Dijkstra、出力形式を共通にしました。GCD は各型で用意された関数を使い、従来版は元の剰余反復です。壁時計時間は起動・入出力・終了を含みます。全出力の SHA256 が一致することを各実行で確認します。

### 型を共通ドライバで比較

単位は ms。改善案・Boost は改善後の Dijkstra、従来版は改善前の Dijkstra です。

| 条件 | 入力 | 従来版 | 改善案 | Boost uint512_t | Boost cpp_int |
| --- | --- | ---: | ---: | ---: | ---: |
| generic | 鎖 | 244.55 | 58.66 | 43.91 | 59.65 |
| generic | ランダム木 | 192.52 | 54.54 | 37.31 | 55.20 |
| generic | 星 | 80.82 | 43.47 | 29.50 | 45.28 |
| native | 鎖 | 247.73 | 59.68 | 43.85 | 62.04 |
| native | ランダム木 | 197.88 | 57.35 | 38.74 | 57.08 |
| native | 星 | 80.61 | 45.42 | 30.16 | 47.89 |

改善案は従来版より約 1.8～4.2 倍高速でした。`cpp_int` と同程度ですが、`uint512_t` に対して約 1.3～1.5 倍遅く、両方の Boost 実装を安定して上回る結果には至りませんでした。

鎖入力・generic の区間別中央値です。区間の中央値の和は壁時計の中央値とは一致しません。GCD 区間には結果格納用配列の準備も含みます。

| 型 | 準備 | Dijkstra | GCD | 除算 | 十進出力 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 従来版 | 10.08 | 18.88 | 185.31 | 12.48 | 1.84 |
| 改善案 | 9.22 | 14.32 | 18.32 | 7.94 | 1.87 |
| uint512_t | 7.76 | 8.48 | 15.62 | 2.90 | 3.58 |
| cpp_int | 9.18 | 14.86 | 18.01 | 4.49 | 3.84 |

### 参考提出の原文

参考提出は原文のまま `-std=c++23 -O2 -DONLINE_JUDGE` でビルドしました。native は同じ追加フラグを使います。原文は `cin/cout` を使用し、辺の重み生成・ヒープの比較・上限の検査も共通ドライバとは異なります。型だけの差は前の表で比較してください。

| 条件 | 入力 | 1182839 uint512_t | 1185228 cpp_int |
| --- | --- | ---: | ---: |
| generic | 鎖 | 49.76 | 70.83 |
| generic | ランダム木 | 42.48 | 64.66 |
| generic | 星 | 33.30 | 51.62 |
| native | 鎖 | 48.25 | 72.36 |
| native | ランダム木 | 44.31 | 63.72 |
| native | 星 | 33.54 | 53.01 |

### Library Checker 上位 AC

2026-10-17 に上位 AC の原文も確認しました。[加算 390082](https://judge.yosupo.jp/submission/390082) は 7 ms で、AVX2 加減算・小さい整数の直接計算・高速入出力を使用。[乗算 389886](https://judge.yosupo.jp/submission/389886) は 25 ms で、十進基数と筆算・FFT の切り替えを使用。[除算 391180](https://judge.yosupo.jp/submission/391180) は 29 ms で、小さい値の通常整数経路・Newton 逆数と FFT・作業領域再利用を使用します。これらの judge 時間は入出力を含み、GCD を多数回使う No.3669 の計測ではありません。参照一覧と原文ハッシュは `results/bigint_yuki_reference.json` にあります。

Dijkstra では [shortest_path 407097](https://judge.yosupo.jp/submission/407097) の 38 ms の AC も確認しました。整数キーの radix heap、CSR 形式の隣接配列、AVX2 入力を使用します。今回の汎用の多倍長重みには整数キー用の手法を直接適用せず、比較ヒープのコピーを削減しました。標準整数型の既存の radix heap 経路は引き続き使用します。

## 再現

Linux で Boost のヘッダーと AtCoder Library を用意し、リポジトリのルートから実行します。履歴の取得に `git fetch --unshallow` が必要な場合があります。

```sh
python3 test/benchmark/bigint_yuki_reference.py --cache /tmp/bigint-references --output /tmp/bigint-references.json
python3 test/benchmark/bigint_yuki.py --reference-dir /tmp/bigint-references --acl-include /path/to/ac-library --boost-include /path/to/boost --output /tmp/bigint-generic
python3 test/benchmark/bigint_yuki.py --reference-dir /tmp/bigint-references --acl-include /path/to/ac-library --boost-include /path/to/boost --native --output /tmp/bigint-native
```

`--boost-include` は標準検索パスに Boost があれば省略できます。`--cpu`・`--trials`・`--cxx` で条件を指定できます。従来版は `a6c027bfe8b8c35a809e24a8429f53ece621cd5f` から、候補は同じ版と保存した差分からキャッシュ内へ復元します。最終版 `b15be54` の不均衡乗算の変更は、この問題で使う 1 limb の整数との乗算には影響しません。

## 検証

GCC 15.2・Clang 21.1.8 でヘッダー単独・Dijkstra・Fast I/O・ヘッダー併用の unit test が成功しました。Clang の ASan・UBSan も通過しています。Dijkstra は標準整数・浮動小数点・128 bit の上限・切断・0 重み・ランダムグラフを確認し、Library Checker shortest_path の公式 29 ケースが両コンパイラで成功しました。

No.3669 は公開例 2 件と Python Fraction を独立した正解にしたランダム 60 ケースを両コンパイラで確認しました。公式全ケースと Apple Clang は既存の GitHub CI で検証します。過去のマイクロベンチマークを履歴から復元して動かす経路も確認しています。

## 削除前の測定記録

以下は以前の実装・検証の記録です。旧マイクロベンチマークは履歴から復元して実行できます。

Boost のヘッダーがコンパイラの検索パスにある環境で、リポジトリのルートから実行します。

```sh
python3 test/benchmark/bigint_history.py --output .verify-helper/cache/bigint-history
g++ -std=c++23 -O2 -DNDEBUG -I .verify-helper/cache/bigint-history -I . test/benchmark/bigint.cpp -o /tmp/bigint-bench
/tmp/bigint-bench 100000 4 0
/tmp/bigint-bench 100000 4 1
```

引数は十進桁数、演算ごとの反復数、実行順です。実行順 0 は BigInt を先に、1 は Boost を先に測定します。macOS の Homebrew 版 Boost では `-I /opt/homebrew/opt/boost/include` を追加します。出力は `桁数,演算,BigInt の ms,Boost の ms,Boost/BigInt` です。末尾に演算名を指定すると、その演算だけ測定します。

GitHub Actions の `bigint history benchmark` は、削除前の最終版を履歴から復元して実行します。CPU・コンパイラ・生の測定値を artifact に保存し、AVX2 の使用可否も出力します。通常の verify ではベンチマークを実行しません。GitHub Actions と各 judge の結果は区別してください。

[参考にした AC 提出 390082](https://judge.yosupo.jp/submission/390082) は多倍長整数の**加算**用です。API 上の記録は 7 ms で、AVX2 の加減算、十進変換のテーブル、mmap と出力バッファ、短い入力に対する整数型での直接計算を利用しています。この提出時間には入出力も含まれます。

今回は次を変更しました。

- 内部配列の最初の 2 要素をオブジェクト内に置き、36 桁までの値は動的確保を省略。両入力が 18 桁以内の加減算は符号付き 64 ビット整数で処理します。64 ビット環境で配列部分のサイズは std::vector と同じ 24 バイトです。
- AVX2 では 16 個の 64 ビット桁をまとめて加減算。繰り上がりの生成・伝播をビット列にし、整数加算で桁間の伝播を計算します。AVX2 非対応 CPU にはスカラー経路を残しています。
- 入力は 8 文字の同時変換を使い、18 桁のブロックを専用経路で処理。出力は 200 バイトの 2 桁テーブルと並列に分割できる 4 桁・8 桁の変換を使います。
- `str()` のゼロ埋めを省略。`to_chars()` と `FastOutput` を組み合わせ、一時文字列を作らず出力できます。
- `FastInput`・`FastOutput` を独立した部品に分離。入力は mmap と fread を切り替え、トークン探索は 8 文字ずつ処理します。加算・乗算・除算の verify ドライバで使用します。ドライバは短い入力でも BigInt の演算を通します。

手元では GCC・Clang で公式の加算 22 件・乗算 20 件・除算 26 件、Python 整数との 10,486 ケース、ASan・UBSan を確認しています。繰り上がりの生成・伝播の全 8 桁パターン、配列長と基数の境界、自分自身との加減算、入力の mmap・パイプ・バッファ境界もテスト対象です。x86_64 用のコンパイルとスカラー経路も確認しました。手元の Mac では AVX2 を実行できず、SIMDe 0.8.2 のソフトウェア実装で AVX2 の各命令を置き換えた経路も、部品テストと Python 整数との 10,486 ケースを通過しました。AVX2 実機での実行確認・Linux/GCC での速度測定は未実施です。上記 Actions で測定できます。

## 初期版の測定記録

以下は今回の変更前の記録です。Mac の結果を根拠に x86_64 向けの切り替え値は調整していません。

2026-10-16、macOS arm64、GCC 15.2、Boost 1.90 の `boost::multiprecision::cpp_int` と比較しました。固定シードで作った 4 組の正整数を使い、反復数は `max(4, 300000 / 桁数)`、5 回の中央値です。実行順を交互にし、算術の計測には十進変換を含めず、計測後に全結果のチェックサムと末尾の値を照合しています。除算の除数は被除数の半分の桁数です。

| 十進桁数 | 乗算 BigInt | 乗算 Boost | 除算 BigInt | 除算 Boost |
| ---: | ---: | ---: | ---: | ---: |
| 32 | 0.000031 | 0.000028 | 0.000058 | 0.000022 |
| 300 | 0.000317 | 0.000202 | 0.001033 | 0.000641 |
| 1,000 | 0.002037 | 0.001533 | 0.007140 | 0.002457 |
| 3,000 | 0.013510 | 0.009650 | 0.037380 | 0.030300 |
| 10,000 | 0.136933 | 0.069933 | 0.328867 | 0.301400 |
| 100,000 | 1.431000 | 2.700500 | 5.321000 | 30.074000 |

10 万桁の詳細です。

| 演算 | BigInt ms | Boost ms | Boost/BigInt |
| --- | ---: | ---: | ---: |
| 加算 | 0.010250 | 0.004000 | 0.39 |
| 乗算 | 1.431000 | 2.700500 | 1.89 |
| 自乗 | 0.938000 | 2.739250 | 2.92 |
| 除算 | 5.321000 | 30.074000 | 5.65 |
| 998244353 による除算 | 0.031000 | 0.073000 | 2.35 |
| 十進文字列から構築 | 0.059500 | 8.966000 | 150.69 |
| 十進文字列へ変換 | 0.076750 | 272.595500 | 3551.73 |

加算や小中サイズの算術では Boost が速いケースがあります。上記はこの環境・入力での結果で、x86_64 での速度は未計測です。

試した変更は以下です。

- 内部基数を $$10^9$$・$$10^{18}$$ で比較し、桁数と加減算の走査量を減らせる $$10^{18}$$ を採用。
- 小さい積は係数を一つずつ集計する方式にし、一時配列を削減。
- Karatsuba は作業領域を再利用し、ゼロ埋めした領域を再帰で計算しないように変更。自乗では対称な積も省略。
- $$10^6$$ に分解する Karatsuba も比較したが、今回の切り替え範囲では改善せず不採用。
- NTT は既存の SIMD 実装を利用し、ビット反転・Montgomery 表現への入出力変換・係数の中間配列を省略。
- 符号付きの桁分割はランダム入力で遅くなったため、疎な場合に限定。係数の絶対値上界を検査してから使う。
- 桁の正規化と除数 1 桁の除算は、事前計算した逆数による積の上位部分と補正で処理。
- 大きな除算は精度を倍増させる整数 Newton 法とブロックごとの除算を利用。末尾のゼロは積から取り除き、結果の桁移動として処理。

GCC・Clang で Library Checker の加算・乗算・除算の全公式ケースを確認し、Python 整数との 10,486 ケースの照合、独立した筆算との照合、ASan・UBSan を実施しています。変換長上限を超える分割経路も、420 万桁同士の積を独立した桁ごとの計算と照合しました。

## 桁数が偏った乗算の Linux/GCC 比較

追加の切り替え条件・整数の上界・再現手順は [bigint_unbalanced.md](bigint_unbalanced.md) を参照してください。
