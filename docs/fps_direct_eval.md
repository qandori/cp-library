---
title: "多点評価の内部直接計算"
documentation_of: //components/fps_direct_eval.hpp
---

`fps_detail::evaluate_direct(dst,f,n,points,m)` は [多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html) が使う内部処理です。通常は `fps/multipoint_eval.hpp` を include して `multipoint_eval` を利用してください。

`f` の先頭 `n` 個を係数として、`points` の先頭 `m` 個での評価値を `dst` に書き込みます。各配列は法 998244353 の内部 `Mint` ワード表現を使い、出力領域は入力領域と重ならないものとします。

この内部関数は x86 のみで定義され、AVX2 対応を実行時に確認してから呼び出す必要があります。公開関数は対応 CPU と対象サイズの場合だけこの経路を選び、非対応 CPU と Mac ARM では既存のスカラー処理を使います。

時間 $$O(nm)$$、出力領域を除く追加空間 $$O(1)$$ です。
