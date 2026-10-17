# 素数判定の試し割り

基準は `2134de7`。公開 API と 64 bit 符号なし整数全域の対応は維持する。

## 変更

`is_prime` の小さい素数 12 個による除外を定数ごとに展開する。GCC 14.2 / `-O2` では変更前のループに `divq` が残っていた。変更後は定数除算の乗算・シフト・比較を使う。

Montgomery 演算、Miller–Rabin の底、Pollard Rho、`factorize` は変更しない。CPU 固有命令や GCC 専用属性も追加していない。Mac 実機の性能は未測定。

## 参照した AC 実装

2026-10-17 に Library Checker API の `problem=...&status=AC&order=%2Btime&limit=3` で取得し、先頭 3 件のソースを調べた。提出時のジャッジ時間と下記のローカル時間は比較しない。参照コードをそのまま移植してはいない。

| 問題 | 提出 | 確認した手法と判断 |
| --- | --- | --- |
| Primality Test | [405740 / Rohan_Kapri](https://judge.yosupo.jp/submission/405740), [365922](https://judge.yosupo.jp/submission/365922), [365921 / rogi52](https://judge.yosupo.jp/submission/365921) | 先頭は 34 ms。底 2 と、大きい表からハッシュで選ぶもう 1 個の底を並列に処理する。テーブルの証明・対象範囲と追加サイズを伴うので採用せず、既存の決定的な底を維持する。定数による小さい素数の除外を比較した。 |
| Factorize | [405743 / Rohan_Kapri](https://judge.yosupo.jp/submission/405743), [317402 / aqxa2k](https://judge.yosupo.jp/submission/317402), [317398 / aqxa2k](https://judge.yosupo.jp/submission/317398) | 先頭は 13 ms。2 系列の Pollard Rho、まとめた GCD、遅延 Montgomery 演算。先頭ソースは法を `2^62` 未満に制限するため、64 bit 全域を扱う既存 API の置換には使わない。幅ごとの Montgomery 分離を試したが、後述の測定で今回は不採用とした。 |

取得したソースの SHA-256:

* 405740: `916f85372b99395b25e2b5faee04eff39662e9bbd3f4638d938f2092b9c7586a`
* 365922: `358cd845c461f70f40f2a4a041b508ad19a7719411a613e3c9b12a40906c90df`
* 365921: `14e114d0503d88adea3bf9ee994e2f3625dc4347f5edcb421ce4df2990d75214`
* 405743 / 317402: `2f80b86bfdf49dec75fe403efe471ebc88808353029cd3752faf3cbce716af68`
* 317398: `530d440f2d815047848ea80c3ba48e5d6a253fe8ad25f8c231734457121b92c7`

## 採用しなかった変更

`n < 2^32` を 32 bit Montgomery 演算に分け、素数判定と Pollard Rho を共通テンプレート化する候補も検証した。最初の独立 9 回測定では汎用ビルドの `semiprime` が 22.456 ms → 26.205 ms、`hard` が 62.690 ms → 66.722 ms となった。15 回の追加測定では `semiprime` の逆転も見られたが、`hard` は 125.218 ms → 131.357 ms と遅く、native の 32 bit 素数判定も安定した改善を示さなかった。追加測定だけを根拠に最初の悪化をノイズとは断定せず、この候補は全体を撤回した。さらに `factorize` の定数試し割りと Montgomery 逆数初期化だけの候補も試したが、長い 15 回測定で汎用版のランダム 32 bit 分解が 91.614 ms → 108.348 ms、64 bit 分解が 293.295 ms → 312.149 ms となった。小さい素数の累乗では改善しても一般の入力に悪化が残るため、これらも撤回した。以下の結果は `is_prime` の除外だけに絞った最終版。

## 測定条件

* 2026-10-17、Debian GNU/Linux 13 / Linux x86_64、Intel Xeon Platinum 8573C。
* GCC 14.2.0、C++23、`-O2 -DNDEBUG -Wall -Wextra -Werror`。native 版だけ `-march=native -ftrivial-auto-var-init=zero` を追加する。
* CPU 1 に固定し、他の作業用コンパイル・測定と時間を分ける。1 回のウォームアップ後、変更前後を交互に 15 回測った中央値。順序は試行ごとに反転する。
* RNG seed は `20261017`。入力生成は計測外。checksum 集計は計測内。全試行で変更前後の checksum を比較する。
* AtCoder 実機、GCC 15.2、Mac で測った結果ではない。

`small` は 0 からの連番、`random32/64` は各幅の一様乱数、`odd32/64` は下位 bit を 1 にした乱数、`prime32/64` は乱数から次の素数まで進めた入力。`factor32/64` は一様乱数の素因数分解。`semiprime` は 10 万から 20 万付近の素数 2 個、`hard` は 30 億から 40 億付近の素数 2 個の積。`power` は 2 から 37 の小さい素数の累乗。

中央値の単位は ms。時間は入力全体の合計。素数だけの 2 行は 1 試行を約 100 ms 以上にした追加測定で、他の行は最初の 15 回測定。

| 入力 | 個数 | 汎用 変更前 | 汎用 変更後 | 倍率 | native 変更前 | native 変更後 | 倍率 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 連番 | 100000 | 8.974 | 6.907 | 1.30 | 10.460 | 9.419 | 1.11 |
| ランダム 32 bit | 100000 | 11.518 | 10.996 | 1.05 | 11.107 | 9.427 | 1.18 |
| ランダム 64 bit | 100000 | 23.520 | 21.100 | 1.11 | 19.649 | 19.485 | 1.01 |
| 奇数 32 bit | 50000 | 10.780 | 9.112 | 1.18 | 10.987 | 11.141 | 0.99 |
| 奇数 64 bit | 50000 | 20.377 | 19.717 | 1.03 | 29.855 | 28.886 | 1.03 |
| 素数 32 bit | 100000 | 146.400 | 139.136 | 1.05 | 102.714 | 98.117 | 1.05 |
| 素数 64 bit | 30000 | 121.325 | 120.831 | 1.00 | 119.218 | 116.188 | 1.03 |

全入力での改善は主張しない。native の奇数 32 bit は約 1.4% 遅く、速度向上を確認できなかった。短い素数のみの測定では、汎用 32 bit が 9.595 ms → 9.749 ms、汎用 64 bit が 45.267 ms → 46.676 ms、native 32 bit が 22.679 ms → 23.751 ms と悪化した。これを消さず、1 試行約 98–146 ms の上表の追加測定と分けて記録する。追加測定では悪化しなかったが、最初の差の原因を断定しない。native 64 bit の最初の短い測定 1 件は検証用コンパイルと重なったため採用していない。

```sh
python3 test/benchmark/primality_factorize.py --cxx g++-14 --cpu 1 --rounds 15
python3 test/benchmark/primality_factorize.py --cxx g++-14 --cpu 1 --native --rounds 15
```

スクリプトの最終設定は 15 回、`prime32` が 100000 個、`prime64` が 30000 個。長い素数のみの 4 行を単独で再測定する場合は次のコマンドを使う。引数は入力種類・個数・試行回数の順。

```sh
g++-14 -std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -I . \
  test/benchmark/primality_factorize.cpp -o /tmp/primality-generic
g++-14 -std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror \
  -march=native -ftrivial-auto-var-init=zero -I . \
  test/benchmark/primality_factorize.cpp -o /tmp/primality-native
taskset -c 1 /tmp/primality-generic prime32 100000 15
taskset -c 1 /tmp/primality-generic prime64 30000 15
taskset -c 1 /tmp/primality-native prime32 100000 15
taskset -c 1 /tmp/primality-native prime64 30000 15
```

最終版で `factor32`・`factor64`・`semiprime`・`hard`・`power` を指定して測る場合、素因数分解の実装は変更前後で同じであり、変更しないコードの対照測定になる。最終版の高速化を示す行ではない。上記の素因数分解の悪化は、撤回した別の候補を測った記録。

基準実装は `primality_factorize_baseline.hpp`。生成した CSV やログはリポジトリに保存しない。

## 検証

`test/unit/primality_arithmetic.cpp` は 100 万以下を篩と照合する。独立した 128 bit 剰余実装で乱数の素数判定を照合し、`2^32` 前後の 8192 整数、既知の強擬素数、32/64 bit 上限付近の法の加算・乗算・累乗を検査する。既存の `factorize.cpp` と `components_number_theory.cpp` も対象とする。

最終版の `primality_arithmetic.cpp` は GCC 14.2 / `-O2 -Wall -Wextra -Werror` と `-O1 -fsanitize=address,undefined -fno-omit-frame-pointer` の両方で成功した。ptrace 下で LeakSanitizer が使えない環境なので、sanitizer 実行は `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1` とした。メモリリーク検査は対象外。

main `020d9344` 上で依存先を選択し、`factorize.cpp`、`components_number_theory.cpp`、`convolution.cpp`、`standalone.cpp` と、素数判定・素因数分解・乗法的畳み込みの各ヘッダ単独コンパイルを GCC 14.2 で実行して成功した。同じ 3 機能の judge driver も依存先として確認したが、オンラインジャッジ検証はローカルで実行していない。
