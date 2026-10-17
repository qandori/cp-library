# Subset Convolution

## 変更

`convolution/subset_convolution.hpp` の API、空入力と異なる長さの扱い、`O(n log² n)` 時間・`O(n log n)` 領域を維持する。`n` は大きい方の入力長を 2 冪へ切り上げた長さ。

- ranked zeta では、未処理の上位 bit が固定されるため、必ず 0 になる下位 rank を省く。
- ranked Möbius では、最終出力の rank に到達できない上位 rank を省く。
- 下位の段を 256 mask ずつ処理し、配列全体を巡回する回数を減らす。
- mask 内の多項式積を rank の降順に計算して `x` に上書きする。作業行の初期化・コピーをなくす。
- GCC / x86 のみに push/pop で囲んだ `O3` を使う。`math/mint.hpp` の演算子は既に `O3` なので、呼び出し側も揃えるとインライン化・自動ベクトル化が進む。固定 ISA、modint の内部表現、除算、fast-math は使わない。Clang と非 x86 は通常の C++ 実装になる。

`half=2^h`、`j=start+q`、`0≤q<half` とする。zeta の入力側には `start` の上位 bit が全て残るので、rank の下限は `popcount(start)`、上限は `popcount(j)`。

Möbius の更新先は `j+half`。この段より後には上位 `levels-h-1` bit だけを追加できる。その中で既に立っている bit は `popcount(start)` 個なので、最終的に到達する mask の最大 popcount は

```
popcount(start) + popcount(q) + 1
  + (levels - h - 1 - popcount(start))
= levels - h + popcount(q)
```

したがって、それを超える rank は捨ててよい。下限は従来どおり `popcount(j+half)`。将来必要な値には演算順の変更がない。

積の入力は rank `0..count`（`count=popcount(mask)`）だけが非零で、出力で必要なのは `count..min(levels,2*count)`。rank `count` を最後に書けば入力を壊さず上書きできる。各出力 rank の加算順は従来と同じ `i` の昇順で、`x[i]*y[k-i]` の左右も維持する。可換な乗算や 2 の逆元は仮定しない。

探索では pruning、GCC O3、積の上書き、64 / 128 / 256 / 512 / 1024 mask の blocking を比較した。探索時の絶対時間は他の処理の影響を受けるため最終表と混ぜない。`n=2^18` の同一実行内中央値は、旧実装 553.6 ms、pruning+O3 210.0 ms、さらに積の上書き 211.4 ms、256 mask blocking 157.6 ms、blocking あり・作業行を残す形 161.2 ms。上書き単独の改善は確認できなかったが、採用形では blocking と合わせて作業行を省ける。block サイズ間の差は小さく、256 を採用した。別の `n=2^16` 探索では、pruning のみ 51.7 ms、pruning+O3 30.9 ms で、pragma の効果も確認した。

## 再現方法

基準は `2134de7f263cda362edd788e8125634cdbcea8bb` の元実装を namespace に隔離した `subset_convolution_baseline.hpp`。

```sh
g++-14 -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -I . \
  test/benchmark/subset_convolution.cpp -o /tmp/subset-native
g++-14 -std=gnu++23 -O2 -I . \
  test/benchmark/subset_convolution.cpp -o /tmp/subset-generic
taskset -c 3 /tmp/subset-native 6 9
taskset -c 3 /tmp/subset-native 10 9
taskset -c 3 /tmp/subset-native 18 9
taskset -c 3 /tmp/subset-native 20 9 mint
taskset -c 3 /tmp/subset-generic 18 9
taskset -c 3 /tmp/subset-generic 6 9
taskset -c 3 /tmp/subset-generic 10 9
```

引数は `log2(n)`、標本数、型（省略時 `all`、`mint / uint32 / uint64 / int128 / double`）。固定 seed、値 `[0,1000]`。全出力の新旧比較を計測前に行い、各標本で新旧の順番を交互にする。1 標本につき `max(1,16384/n)` 回測った平均の中央値を出す。入力生成・一致確認は計測外、関数内の確保・初期化は計測内、返された出力の破棄は計測外。checksum で結果を使用する。

測定環境は 2026-10-17、Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0。AtCoder の公開コンパイル設定に合わせた native と、`-O2` のみの generic を測る。`NDEBUG` は定義しない。実機 AtCoder、GCC 15.2、macOS / Clang の速度は未測定。

他のローカル測定・コンパイルを止め、CPU 3 に固定して 9 標本を測定した。各欄は旧 → 新の ms（倍率）。

| 型 | native `2^6` | native `2^10` | native `2^18` |
| --- | ---: | ---: | ---: |
| mint | 0.007887 → 0.004193 (1.88x) | 0.302353 → 0.116397 (2.60x) | 296.517 → 136.784 (2.17x) |
| uint32 | 0.004173 → 0.002994 (1.39x) | 0.104852 → 0.075592 (1.39x) | 111.841 → 72.978 (1.53x) |
| uint64 | 0.003473 → 0.002718 (1.28x) | 0.163746 → 0.116889 (1.40x) | 261.538 → 227.190 (1.15x) |
| int128 | 0.005609 → 0.004919 (1.14x) | 0.326253 → 0.276545 (1.18x) | 957.761 → 834.217 (1.15x) |
| double | 0.003725 → 0.003578 (1.04x) | 0.132437 → 0.088750 (1.49x) | 403.740 → 263.217 (1.53x) |

Library Checker 最大サイズ `2^20` の mint は **1998.339 → 960.306 ms (2.08x)**。全出力の一致も確認した。

| 型 | generic `2^6` | generic `2^10` | generic `2^18` |
| --- | ---: | ---: | ---: |
| mint | 0.014415 → 0.008594 (1.68x) | 0.399349 → 0.181562 (2.20x) | 295.246 → 160.130 (1.84x) |
| uint32 | 0.013671 → 0.011055 (1.24x) | 0.257737 → 0.156011 (1.65x) | 150.262 → 93.431 (1.61x) |
| uint64 | 0.009950 → 0.008480 (1.17x) | 0.288256 → 0.193360 (1.49x) | 290.696 → 214.743 (1.35x) |
| int128 | 0.013115 → 0.007397 (1.77x) | 0.448606 → 0.368389 (1.22x) | 914.643 → 787.548 (1.16x) |
| double | 0.010574 → 0.005391 (1.96x) | 0.301105 → 0.137036 (2.20x) | 292.266 → 261.257 (1.12x、再測定) |

最初の generic double `2^18` は 711.535 → 2460.810 ms (0.29x) だった。同じバイナリを当該型だけで 9 標本再測定すると表の結果になった。別の再測定でも 321.435 → 269.595 ms (1.19x) だったが、この回には別のコンパイルが重なっていたため、独立した静穏条件の証拠には数えない。全てのローカルコンパイル・テスト・測定の停止を改めて確認してから、最後に同条件で再測定した結果は **382.551 → 338.599 ms (1.13x)**。静穏条件の 2 回は 1.12x / 1.13x で、大きな退行は再現しなかった。コード変更はしていない。クラウドホストの外部負荷は制御できず、絶対時間のばらつきが大きいため、別実行の絶対時間から native / generic の優劣は判断しない。native double `2^6` の 4% 差など小さい差は有意な高速化と断定しない。

```sh
taskset -c 3 /tmp/subset-generic 18 9 double
```

## Library Checker の AC 実装

2026-10-17、live API の `problem=subset_convolution&status=AC&order=+time` の先頭を確認した。

- [#406006 / Rohan_Kapri](https://judge.yosupo.jp/submission/406006): AC、93 ms、35,430,400 bytes。
- [#352953 / adamant](https://judge.yosupo.jp/submission/352953): AC、94 ms、35,512,320 bytes。

両ソースは cp-algorithms の rank-XOR 方式を使い、rank 0 を別扱いにし、上位 bit ごとの小さい作業領域へ入力を投影している。rank ごとの XOR 変換、SIMD Montgomery 積、逆変換後の `N/2` の逆元を組み合わせる。今回の API は整数・`__int128`・非可換行列環なども扱うため、2 の逆元と特定 modint 表現を要する方式への置換はしなかった。汎用的な rank 範囲削減と cache locality を改善した。

[NyaanNyaan の高速 subset convolution](https://github.com/NyaanNyaan/library/blob/master/math-fast/subset-convolution.hpp) も確認した。こちらは ranked zeta / Möbius、AVX2 の 8 要素 Montgomery 演算、rank の一時転置を使う。固定幅 SIMD と特定の剰余表現に依存するため、そのままは導入していない。

上記の提出時間は Library Checker の I/O 込みの値で、今回のローカル関数測定とは比較しない。上記ソースとのローカル実行速度比較や、新実装のオンライン提出は行っていない。

```sh
python3 test/benchmark/subset_convolution_reference.py /tmp/subset-reference \
  --metadata /tmp/subset-reference.json
```

上記は AC 状態を確認して参照ソースと hash を再取得する。生成した CSV / JSON / log は commit しない。

[AtCoder の公開コンパイル設定](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)

## 検証

`test/unit/subset_convolution.cpp` は愚直な submask 列挙と旧実装を参照する。空、長さ 1、異なる長さ、2 冪とその前後、256 mask block 境界を確認する。乱数入力と `2^16` の全 1 入力を使い、入力を変更しないことも確認する。

対象型は mint、uint32、uint64、`__int128`、double、および default constructor を持たない非可換 2×2 行列環。Boolean subset OR / AND も愚直解と比較し、内部の `__int128` 利用を検証する。

```sh
g++-14 -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -I . \
  test/unit/subset_convolution.cpp -o /tmp/subset-test
/tmp/subset-test
g++-14 -std=gnu++23 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -I . \
  test/unit/subset_convolution.cpp -o /tmp/subset-sanitize
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 /tmp/subset-sanitize
```

通常ビルドと上記 ASan / UBSan は成功。初回はテスト本体に診断はなかったが、終了時に LeakSanitizer が ptrace 環境を理由に失敗した。leak 検査を無効にした同一バイナリの再実行は exit 0。leak 検査、macOS / Clang はこの環境では未検証。

統合先の最新 main `25b0975173c652c60c682debb0b0041aa25f2555` に本変更だけを載せた別作業ディレクトリでも、GCC 14.2 の `-Wall -Wextra -Werror` による `subset_convolution.hpp` / `boolean_convolution.hpp` の単独 include と、推移的な利用先の既存 `test/unit/convolution.cpp` が成功した。ローカルの Clang / macOS / オンラインジャッジ成功は主張しない。
