---
title: FPS 逆数のブロック変換
documentation_of: //components/fps_inverse_blocked.hpp
library_category: components/fps
---

`fps::inv()` が使う内部部品です。通常は公開 FPS API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側で AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_inverse(in,input,out,n)` | 逆数の先頭 n 係数を空の out へ追加 | $$O(n\log n)$$ |
| `append_scaled(a,out,scale)` | a の係数を scale 倍して out へ追加 | 予約済み: 追加係数数に比例、再確保時: 既存要素数も加算 |

関数は `fps_detail::blocked_poly` 内にあります。通常は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f.inv(n)` を使います。

`fps_detail::blocked_poly::native_inverse(in,input,out,n)` は、`in` の `input` 個の `Mint` 係数から、逆数 $$1/f\bmod x^n$$ を求め、空の `vector<Mint>` である `out` に `n` 項を追加します。`1 <= n <= 2^23`、`input >= 1`、定数項が非零であることを前提とします。内部の 32 bit 整数値の範囲は $$[0,2p)$$、$$p=998244353$$ です。入力の範囲外を 0 とし、次数 `n` 以降は読みません。入力は変更せず、`out` と重ねません。公開配列に末尾の余分な領域や先頭アドレスの配置条件は要求しません。

`append_scaled(a,out,scale)` は、通常の剰余表現の 32 bit 整数配列 `a` に `scale` を掛け、結果の整数値を `Mint` の内部表現として `out` の末尾へ追加する補助関数です。`a` と `out` は重ねず、`a` の値と `scale` は $$[0,p)$$ とします。入力は変更しません。

作業領域は `Arena` の破棄時に自動で解放します。確保できなければ例外を送出します。

**計算量**: `native_inverse` は $$O(n\log n)$$ 時間、$$O(n)$$ 追加空間です。`append_scaled` は追加する項数に対して $$O(n)$$ 時間です。
