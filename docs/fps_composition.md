---
title: FPS の合成・射影の内部処理
documentation_of: //components/fps_composition.hpp
library_category: components/fps
---

Linux x86_64 / GCC 向けの内部部品です。呼び出し側で AVX2（複数の整数をまとめて処理する CPU 命令）の実行時判定を行います。通常は公開 API の [合成](https://qandori.github.io/cp-library/fps/compose.hpp.html)・[power projection](https://qandori.github.io/cp-library/fps/power_projection.hpp.html) を使ってください。他の環境では[汎用実装](fps_composition_generic.md)へ戻ります。法は $$p=998244353$$ です。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

このヘッダーは[合成の専用変換](fps_compose_blocked.md)と[射影の専用変換](fps_projection_blocked.md)を両方読み込む互換ヘッダーです。`uint32_t` の配列を直接渡す内部 API も使えます。内部から片方の演算だけを使う場合は対応する個別ヘッダーを include し、変換用の定数表・列変換・作業領域などは[共通部品](fps_composition_common.md)を共有します。

`fps_detail::composition_poly` の `Arena`・`Transform`・`Half` はブロック変換の部品を共有します。内部配列は Arena から確保し、利用者の入力に末尾の余分な領域や先頭アドレスの配置条件を要求しません。確保失敗時には通常の整列配列へ戻り、それも失敗した場合は例外となります。

| 内部 API | 内容 | 計算量 |
| --- | --- | --- |
| `compose_log(n)`・`compose_scratch(n)` | 合成に必要な変換長の $$\log_2$$／作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `compose(t,f,g,h,scratch)` | `h.size()` 項の $$f(g)$$。普通剰余の入力・出力、$$g_0=0$$。入力・出力は重ねない | $$O(n\log^2 n)$$ |
| `projection_log(n)`・`projection_scratch(n)` | 係数取得に必要な変換長の $$\log_2$$／作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `power_projection(t,g,a,scratch)` | $$a_k=[x^{n-1}]g^k$$、$$n=\texttt{a.size()}$$、$$g_0=0$$ | $$O(n\log^2 n)$$ |
| `weighted_projection(t,g,w,a,scratch)` | $$\displaystyle a_k=\sum_j w_j[x^j]g^k$$、`w.size()==a.size()>32`、$$g_0=0$$ | $$O(n\log^2 n)$$ |
| `native_compose(f,nf,g,ng,out,n)` | 内部 Montgomery 係数の入力から、長さ `n` の合成を空の `vector<mint>` へ追加 | $$O(n\log^2 n)$$ |
| `native_projection(g,ng,out,n)` | 内部 Montgomery 係数から、`n` 個の冪について取り出した係数を空の `vector<mint>` へ追加 | $$O(n\log^2 n)$$ |
| `native_weighted_projection(g,ng,w,out,n,count)` | `n` 個の重みによる射影の最初の `count` 項を空の `vector<mint>` へ追加。$$0<\texttt{count}\le n$$ | $$O(n\log^2 n)$$ |

普通剰余の係数は $$[0,p)$$、Montgomery 係数の内部の 32 bit 整数値は $$[0,2p)$$ です。入力長を超える係数は 0 とし、出力次数以降は読みません。`uint32_t` の配列を直接渡す API の作業領域は Arena と同じ整列・余白を持たせます。重み付きの `uint32_t` 配列を扱う API では、作業領域に `projection_scratch(n) + Arena::footprint(6*detail::projection_length(n))` 個の整数を用意し、`Transform` の大きさには `projection_log(n)+1` を使います。

`native_compose` では $$n\le2^{21}$$ とします。`Tables` が `Transform` の定数表を参照する場合は、使用終了まで `Transform` とその `Arena` を破棄しないでください。作業領域には `compose_scratch`・`projection_scratch` が返す要素数を用意します。

項数を $$n$$ として時間は $$O(n\log^2 n)$$、追加空間は合成が $$O(n\log n)$$、射影が $$O(n)$$ です。
