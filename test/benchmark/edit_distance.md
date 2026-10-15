# 編集距離のベンチマーク

```sh
g++ -std=c++23 -O2 -DNDEBUG -I . test/benchmark/edit_distance.cpp -o /tmp/edit-distance-bench
/tmp/edit-distance-bench library random26 200000 5
/tmp/edit-distance-bench scalar64 random26 200000 5
/tmp/edit-distance-bench avx2 random26 200000 5
/tmp/edit-distance-bench avx512 random26 200000 5
```

引数は実装、入力の種類、文字列長、繰り返し回数です。単位は ms、中央値を出力します。`avx2`・`avx512` は対応する x86_64 CPU で実行します。`library` は共通部分の除去・部分列判定・共通文字の判定・処理量を制限した wavefront 探索も含み、CPU に応じて更新処理を選びます。

入力は `random2`・`random4`・`random26`・`random256`、同一文字列の `identical`、1・100・1,000 箇所を置換する `near1`・`near100`・`near1000`、挿入と削除の `indel100`、共通文字のない `disjoint`、大きなブロックを逆順にする `blocks`、周期列をずらす `periodic`、短い方を 100 文字にする `unbalanced` です。乱数 seed は 1234 です。

両方 200,000 文字の場合、更新ループの反復回数は以下です。

| 更新処理 | 1 回の位置数 | 反復回数 |
| --- | ---: | ---: |
| `scalar64` | 64 | 625,000,000 |
| `avx2` | 256 | 156,400,000 |
| `avx512` | 512 | 78,200,000 |

状態と文字マスクを 64 byte 境界に整列し、8 ワード単位に埋めて端数処理をなくしています。文字は整数列に変換せず、256 個のポインターからマスクを引きます。SIMD の加算は生成・伝播マスクで桁上がりを一括処理します。AVX2 の生成マスクは符号反転と比較を使わず、AVX-512 の論理式は ternary logic 命令にまとめています。GCC の `O3` は SIMD 関数だけに適用し、呼び出し側の設定は変更しません。

比較用に、ブロックごとに境界差分を渡す `block`、独立した桁上がりを持つ `add128`、ループを展開する `add128_unroll2`・`unroll2`・`unroll4`、論理式を変える `d0_block`・`d0_add128`、128 bit ブロックの `block128`・`d0_block128`、状態を交互に置く `interleaved`、通常の DP の `scalar`、wavefront 探索の `wave1`・`wave4`・`wave16` を残しています。wavefront 単体は処理量の上限で打ち切ると `-1` を返します。

2026-10-15 の macOS arm64 / GCC 15.2 では、26 種類・100,000 文字同士の更新処理で `block` 約 417 ms、`add128` 約 223 ms を記録しました。AVX2・AVX-512 の実機時間はこの環境では計測できていません。

両 SIMD 処理は、通常の DP との比較、20 万文字同士、生成・伝播・非伝播の組み合わせ全 6,561 通り、ASan・UBSan で検証しています。AVX 非対応の環境でも、[SIMDe](https://github.com/simd-everywhere/simde) を使って同じ関数本体を検証できます。ライブラリ本体に SIMDe は必要ありません。

```sh
git clone --depth 1 https://github.com/simd-everywhere/simde.git /tmp/simde
python3 test/verify_edit_distance_simd.py --simde /tmp/simde --cxx g++
python3 test/verify_edit_distance_simd.py --simde /tmp/simde --cxx clang++ --sanitize
```

ビット並列法は [Myers の論文](https://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf)、編集回数ごとの探索は [wavefront alignment](https://github.com/smarco/WFA-paper) を参考にしています。
