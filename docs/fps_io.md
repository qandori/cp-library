---
title: FPS の一括入出力
documentation_of: //components/fps_io.hpp
---

Library Checker ドライバーの係数読み書きに使う内部部品です。Linux x86_64 / GCC 用に定義され、呼び出し側で AVX2 対応を確認します。他の環境では通常の整数入出力を使います。

`fps_detail::read_coefficients(input,out,n)` は `FastInput` から符号なし 32 bit 整数を `n` 個読み、空の `vector<Mint>` に追加します。2 トークンを SIMD で変換し、整数境界・空白・入力末尾を確認します。入力バッファの末尾や不規則な空白では既存の整数読み込みに戻ります。利用者が渡す入力に余分な読み取り領域は要求しません。

`fps_detail::write_coefficients(output,in,n)` は `n` 個の係数を `FastOutput` へ書きます。各係数を 9 桁分の領域に右詰めし、余白は空白とします。16 値ずつ SIMD で整形し、最後は改行します。`n=0` は改行のみです。通常の `FastOutput::write` の形式は変更しません。

`FastInput`・`FastOutput` の既存 API と係数の値は維持します。これらの関数は通常の整数入出力と同様、各トークンが指定の整数型に収まる入力を前提とします。

時間は $O(n)$、追加の作業領域は $O(1)$ です。
