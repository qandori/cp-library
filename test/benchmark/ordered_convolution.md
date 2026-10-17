# Ordered Convolution

## 変更

`ordered_convolution(a, b)` の API は変更しない。ライブラリ標準の `mint` に限り、長さ 32 以下の再帰区間は `i < j` の係数対を直接計算する。小さい部分問題ごとの入力ベクトル確保と通常の畳み込み呼び出しを省く。区間をまたぐ項は従来の分割統治に残し、区間内の各項を一度だけ加算する。剰余演算では加算順序の変更による誤差はない。

他の型は従来の経路のまま。特に整数の中間 overflow、浮動小数点の加算順序、一般の環の演算条件を新しい直接計算に置き換えない。固定 ISA、追加作業配列、API の型制約は増やさない。計算量も従来どおり。

## 測定条件

基準は `2134de7f263cda362edd788e8125634cdbcea8bb`。同梱 baseline はガード・関数名・include 相対パスだけを変えた元実装。

2026-10-17、Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0。C++23、`-O2 -DNDEBUG`、native のみ `-march=native -ftrivial-auto-var-init=zero`。CPU 4 に固定し、他のローカル測定・コンパイルを止める。AtCoder 実機、GCC 15.2、Mac の速度は未測定。

固定 seed `20261017` で mint の法全域の値を生成し、全出力を baseline と比較してから計測する。整数対照測定は overflow を避けるため `[0,2]`。入力生成と事前比較は計測外。全関数呼び出し、割り当て、出力 checksum、破棄を計測内に含める。各組の新旧実行順を反転し、wall と process CPU time を測る。各中央値の比と、各組の CPU 比の中央値を区別する。

```sh
g++ -std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -Wno-unused-variable -I . \
  test/benchmark/ordered_convolution.cpp -o /tmp/ordered-generic
g++ -std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero \
  -Wall -Wextra -Werror -Wno-unused-variable -I . \
  test/benchmark/ordered_convolution.cpp -o /tmp/ordered-native
```

`-Wno-unused-variable` は NDEBUG で既存 NTT の assert 専用変数が未使用になるため両者に指定する。引数は入力長 `n m`、反復数、型 `mint|int64|all`、標本数。生ログはコミットしない。

## 結果

15 組の測定。時間は反復全体の wall 中央値（ms）、倍率は各組の process CPU time 比の中央値。

| 型 / n × m | 反復 | 汎用 旧 ms | 汎用 新 ms | paired CPU 倍率 | native 旧 ms | native 新 ms | paired CPU 倍率 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| mint / 1 × 1 | 2000000 | 85.368 | 86.626 | 1.014 | 83.721 | 96.921 | 0.966 |
| mint / 17 × 31 | 100000 | 452.734 | 213.496 | 2.114 | 445.482 | 196.750 | 2.230 |
| mint / 32 × 32 | 100000 | 700.379 | 281.582 | 2.594 | 641.410 | 255.657 | 2.452 |
| mint / 1024 × 1024 | 1000 | 326.159 | 177.145 | 1.866 | 322.513 | 187.247 | 1.776 |
| mint / 65536 × 65536 | 10 | 280.038 | 192.615 | 1.395 | 357.220 | 228.812 | 1.517 |
| mint / 32 × 4096 | 4000 | 497.782 | 426.355 | 1.058 | 622.177 | 534.107 | 1.100 |
| mint / 4096 × 32 | 50000 | 1404.920 | 1143.830 | 1.200 | 1568.730 | 1096.830 | 1.457 |
| int64 / 1024 × 1024 | 100 | 1007.970 | 858.214 | 1.044 | 683.815 | 730.586 | 0.972 |

mint の通常サイズでは改善した。`n=m=1` は新しい kernel に入らない対照、int64 はソース上の演算経路を変更しない対照。これらの上下動を改善と扱わず、原因を外部ノイズと断定もしない。native の初回対照は遅かったため、同じバイナリで追加確認した。`n=m=1` は 10000000 反復・31 組で wall 中央値 476.731 → 466.831 ms、paired CPU 比 0.989 倍。int64 は同じ 100 反復・15 組で 662.497 → 693.912 ms、paired CPU 比 0.998 倍だった。初回の悪化を消さず併記し、この差の原因は断定しない。全入力での改善は主張しない。

```sh
for build in generic native; do
  for args in "1 1 2000000 mint" "17 31 100000 mint" "32 32 100000 mint" \
              "1024 1024 1000 mint" "65536 65536 10 mint" "32 4096 4000 mint" \
              "4096 32 50000 mint" "1024 1024 100 int64"; do
    taskset -c 4 /tmp/ordered-$build $args 15
  done
done
taskset -c 4 /tmp/ordered-native 1 1 10000000 mint 31
taskset -c 4 /tmp/ordered-native 1024 1024 100 int64 15
```

## 参照

直接の順序制約付き API と通常の線形畳み込みは区別する。関連する Library Checker `convolution_mod` の fastest AC として、2026-10-17 に確認した [#409465 / Aiyiyi](https://judge.yosupo.jp/submission/409465)（12 ms）のソースを読んだ。998244353 専用の巡回 NTT、AVX2、サイズ別処理を用いるが、`i < j` 制約付き再帰を直接置き換えるものではない。今回 NTT 自体は変更せず、小区間の割り当てをなくす。参照ソース SHA-256: `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。提出のジャッジ時間とローカル関数測定は比較しない。

## 検証

新規 unit test は空入力、左右不一致、32 前後の閾値、非 2 冪サイズ、全剰余域の乱数を確認する。mint、int64、デフォルト構築できない一般の環を愚直計算および baseline と比較し、double の経路も baseline と照合する。

新規 unit test は GCC 14.2 / `-O2 -Wall -Wextra -Werror` と `-O1 -fsanitize=address,undefined -fno-omit-frame-pointer` の両方で成功した。ptrace 下のため sanitizer 実行には `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1` を指定し、リーク検査は実施していない。main `cc4ef913` にこの 5 ファイルだけを重ねた独立ツリーで、依存先選択に従って ordered ヘッダーの単独コンパイルと既存 `test/unit/convolution.cpp` を GCC 14.2 で実行し成功した。新規 unit test の再帰的 include を含む 12 ファイルは既に検証済みのツリーと byte 単位で一致するため、同一テストは再実行していない。`test/ordered_convolution.test.cpp`（yukicoder No.1307）も影響先として選択したが、ローカルでオンラインジャッジ検証は実行していない。Mac は CI の検証対象であり、ローカルでは未実行。
