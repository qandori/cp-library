---
title: FPS の合成逆元の内部処理
documentation_of: //components/fps_reversion.hpp
library_category: components/fps
---

`compositional_inverse` が使う内部部品です。Linux x86_64 / GCC でのみ定義され、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。法は $$p=998244353$$ です。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_reversion(in,input,out,n)` | 合成逆元の先頭 n 係数を空の out へ追加 | $$O(n\log^2 n)$$ |

関数は `fps_detail::composition_poly` 内にあります。通常は [合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse.hpp.html) の `compositional_inverse(f,n)` を使います。

`fps_detail::composition_poly::native_reversion(in,input,out,n)` は、内部 Montgomery 係数の `input` 個の入力から、合成逆元の最初の `n` 項を空の `vector<Mint>` である `out` へ追加します。`in` と `out` は重ねず、入力は変更しません。公開側では $$64\le n\le2^{21}$$、入力長 2 以上、$$f_0=0$$、$$f_1\ne0$$ の条件で呼びます。入力長を超える係数は 0 として扱います。

利用者の配列の先頭アドレスの配置や、末尾への余分な領域は要求しません。作業領域を確保できなければ例外を送出します。

入力と出力の最大係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、追加空間は $$O(n)$$ です。
