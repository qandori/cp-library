---
title: ワーシャルフロイド法の内部演算
documentation_of: //components/warshall_floyd_kernel.hpp
library_category: components/util
---

[ワーシャルフロイド法](https://qandori.github.io/cp-library/graph/warshall_floyd.hpp.html)の内部部品です。通常は数値範囲の判定と変換を行う `WarshallFloyd` を使います。

`warshall_floyd_detail::Storage<T>` は先頭を 64 バイト境界に配置する配列です。行列は `B * B` 要素のブロックを行優先に並べ、各ブロック内も行優先で保持します。実装本体では `B = 32` です。端数の頂点は独立した仮頂点で埋め、対角を 0、他を `inf` にします。

`run<T,B,Negative>(data,blocks,inf,selected)` はブロック数 `blocks` の正方行列を更新し、負閉路がなければ `true` を返します。`selected` は 0 が通常演算、1 が AVX2、2 が AVX-512F。省略時は `backend()` で実行 CPU に応じて決めます。明示する場合は CPU がその命令に対応していることが必要です。`T` は符号付き 32 bit・64 bit・128 bit 整数で、128 bit は通常演算を使います。

`Negative = false` では全ての有限値が非負であることが必要です。符号なし加算と最小値を使い、`inf` を越えた候補を採用しません。表現範囲を越えた有限距離と到達不能を区別する判定は、呼び出し元で行います。

`Negative = true` では、到達不能値を加算から除外します。対角ブロック内の各経由頂点を処理する前に対角成分を調べ、負閉路を検出したら停止します。有限値の絶対値の最大を $$W$$、仮頂点を除く頂点数を $$n$$ として、呼び出し元は $$2(n+1)W < \mathrm{inf}$$ となる内部型を用意します。`inf` は正で、内部型の最大値以下です。この条件で、負辺を含む有限値の加算も内部型に収まります。

配列の要素数は少なくとも `(blocks * B)^2`、各ブロックの端数はなく、`B` は 32 の倍数としてください。内部関数はこれらの前提や数値範囲を再検証しません。

仮頂点を含む頂点数を $$p=B\cdot\mathrm{blocks}$$ として、計算時間は $$O(p^3)$$、配列以外の追加空間は $$O((p/B)^2)$$ です。
