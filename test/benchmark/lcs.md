# LCS のベンチマーク

2026-10-15、macOS arm64 / GCC 15.2、`-std=c++23 -O2 -DNDEBUG`。乱数 seed 1234、5 回の中央値、単位は ms です。ランダム入力は実装・入力の実行順を混ぜています。

```sh
g++ -std=c++23 -O2 -DNDEBUG -I . test/benchmark/lcs.cpp -o /tmp/lcs-bench
/tmp/lcs-bench length random26 100000 5
/tmp/lcs-bench restore random26 100000 5
```

引数は実装、入力の種類、文字列長、繰り返し回数です。`unbalanced` は長い方が 100,000 文字、短い方が 100 文字、その他は両方 100,000 文字です。

| 入力 | 長さ計算 | 復元込み | LCS 長 |
| --- | ---: | ---: | ---: |
| ランダム 2 種類 | 95.97 | 208.82 | 81,144 |
| ランダム 4 種類 | 97.20 | 229.44 | 65,420 |
| ランダム 26 種類 | 100.83 | 215.46 | 32,549 |
| ランダム 256 種類 | 99.06 | 216.30 | 11,754 |
| 同一文字列 | 0.032 | 0.040 | 100,000 |
| 約 1% の位置を書き換え | 98.00 | 208.50 | 99,044 |
| 共通する文字なし | 0.547 | 0.468 | 0 |
| `a`・`b` の大きなブロックを逆順に配置 | 95.30 | 95.24 | 50,000 |
| 長さが大きく異なる | 0.001 | 0.039 | 100 |

更新式は [Hyyrö らのビット並列 LCS](https://users.dcc.uchile.cl/~gnavarro/ps/jea06.pdf) を基に、減算を XOR に置き換え、桁上がりを 128 bit 加算で扱っています。短い方をビット列にし、文字マスクは実際に登場する文字だけ確保します。64 文字以下は桁上がりのない処理に切り替えます。

減算を使う更新式、overflow 組み込み関数、大小比較、ビット演算による桁上がり、2・4 ワードのループ展開、2・4・8 行の同時更新を比較しました。ベンチマークの `subtract`・`overflow`・`compare`・`bitcarry`・`unroll2`・`unroll4`・`batch2`・`batch4`・`batch8` で再現できます。複数行の同時更新は入力によって数 % 改善しましたが、悪化する場合もあり、追加領域と実装量の小さい `add128` を採用しています。

共通する接頭辞・接尾辞の除去、部分列判定、共通しない文字の除去も行います。復元はビット並列の Hirschberg 法で、分割位置の評価にもビット列を直接使います。再帰前に左右の行を解放し、小さい部分問題だけ通常の DP に切り替えます。

正しさは短い 2 文字列の全探索、通常の DP との乱数比較、64 bit 境界、NUL を含む全 256 種類のバイト、10 万文字、AOJ `ALDS1_10_C` 全 5 ケースで検証しています。長さ計算・復元とも GCC・Clang で通過し、Clang の ASan・UBSan と x86_64 実行でも確認しています。

## SIMD 更新処理

x86_64 では AVX-512・AVX2 を自動選択します。正順・逆順の行計算を両方 SIMD 化し、長さ計算と Hirschberg 法による復元に使います。SIMD 非対応の環境は従来の 64 bit 更新処理です。追加のコンパイルオプションは必要ありません。

```sh
/tmp/lcs-bench scalar64 random26 200000 5
/tmp/lcs-bench avx2 random26 200000 5
/tmp/lcs-bench avx512 random26 200000 5
/tmp/lcs-bench restore_avx2 random26 200000 5
/tmp/lcs-bench restore_avx512 random26 200000 5
```

`scalar64`・`avx2`・`avx512` は更新処理単体、`restore_avx2`・`restore_avx512` は復元を含みます。SIMD の指定には対応する CPU が必要です。

文字マスクと状態を 64 byte 境界に整列し、8 ワード単位に埋めて端数の更新をなくしています。加算の桁上がりを生成・伝播マスクでまとめて計算し、AVX-512 では更新式を ternary logic 命令にまとめています。GCC の `O3` は SIMD 関数だけに適用します。

両方 200,000 文字の場合、更新ループの反復回数は 64 bit 版で 625,000,000 回、AVX2 で 156,400,000 回、AVX-512 で 78,200,000 回です。上の時間表は SIMD 追加前の macOS arm64 での計測値で、AVX2・AVX-512 の実機時間は手元では未計測です。

正順・逆順の行全体を通常版と比較し、復元結果の長さと部分列であることも確認しています。20 万文字同士、ベクトル境界を越える桁上がり全 6,561 通り、ASan・UBSan を含みます。AVX 非対応の環境での検証には、編集距離と共通の補助スクリプトを使えます。

```sh
python3 test/verify_edit_distance_simd.py --library lcs --simde /tmp/simde --cxx g++
python3 test/verify_edit_distance_simd.py --library lcs --simde /tmp/simde --cxx clang++ --sanitize
```

`/tmp/simde` には [SIMDe](https://github.com/simd-everywhere/simde) のソースを配置します。ライブラリ本体には不要です。
