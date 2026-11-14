---
title: "多点評価の内部直接計算"
documentation_of: //components/fps_direct_eval.hpp
library_category: components/fps
---

`fps_detail::evaluate_direct(dst,f,n,points,m)` は [多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html) が使う内部処理です。通常は `fps/multipoint_eval.hpp` を include して `multipoint_eval` を利用してください。

| 関数 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::evaluate_direct(dst,f,n,points,m)` | `f` の先頭 `n` 個を係数とする多項式を `points` の先頭 `m` 個で評価し、`dst` に書き込む | $$O(nm)$$ |

`f` の先頭 `n` 個を係数として、`points` の先頭 `m` 個での評価値を `dst` に書き込みます。各配列は法 $$p=998244353$$ の内部 `Mint` 表現の整数配列です。係数 $$c$$ を $$c\,2^{32}$$ と法 $$p$$ で合同な $$[0,2p)$$ の整数で保持します。出力領域は入力領域と重ならないものとします。

この内部関数は x86 のみで定義され、AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認してから呼び出す必要があります。公開関数は対応 CPU と対象サイズの場合だけこの経路を選び、それ以外では 1 要素ずつの計算を使います。

時間 $$O(nm)$$、出力領域を除く追加空間 $$O(1)$$ です。
