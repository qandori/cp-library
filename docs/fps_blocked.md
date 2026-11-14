---
title: FPS のブロック変換
documentation_of: //components/fps_blocked.hpp
library_category: components/fps
---

FPS の逆数・対数・指数・冪・平方根・除算と各専用アルゴリズムが共有する内部部品です。公開 API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_log(in,input,out,n,integrate)` | 対数または対数微分を空の out へ追加 | $$O(n\log n)$$ |
| `Arena::zero_initialized()` | 確保直後の領域が零で初期化されていたか | $$O(1)$$ |

通常の利用では [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f.log(n)` を呼びます。以下の関数は `fps_detail::blocked_poly` 内の内部 API です。

`fps_detail::blocked_poly::native_log(in,input,out,n,integrate)` は、`in` の `input` 個の内部 Montgomery 係数から、対数微分の最初の `n-1` 項を求めます。`integrate=true` では積分した対数を、偽では定数項 0 の後に対数微分を格納します。結果は空の `vector<Mint>` である `out` に追加し、長さを `n` にします。`in` は変更せず、`out` と重ねません。公開 FPS 側では `256 <= n <= 2^24`、入力長 64 以上の条件で使います。

入力の範囲外は 0 とし、出力次数より後ろは読みません。係数の内部の 32 bit 整数値は $$[0,2p)$$、定数項は非零です。ここで $$p=998244353$$ です。

`Arena::zero_initialized()` は確保した領域の初期状態が零かを返し、領域を再利用した後の状態は保証しません。公開配列に末尾の余分な領域や先頭アドレスの配置条件は要求しません。作業領域を確保できなければ例外を送出します。

入力と出力の最大係数数を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。
