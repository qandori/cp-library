---
title: FPS の指数・冪の内部変換
documentation_of: //components/fps_exp_blocked.hpp
library_category: components/fps
---

`fps::exp()`・`fps::pow()` と形式的合成逆関数が使う内部部品です。Linux x86_64 / GCC で定義されます。呼び出し側は AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。他の環境では公開 FPS の既存実装を使います。以下では $$p=998244353$$、係数数を $$n\ge1$$ とします。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

`fps_detail::blocked_poly` に次の関数を定義します。

| 関数 | 内容 | 計算量 |
| --- | --- | --- |
| `exp_log(n)` / `exp_scratch(n)` | 指数に必要な変換長の $$\log_2$$ の最大値／作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `exp(t,f,g,scratch)` | `g.size()` 項の $$\exp(f)$$。`f` の定数項は 0 | $$O(n\log n)$$ |
| `power_log(n)` / `power_scratch(n)` | 冪に必要な変換長の $$\log_2$$ の最大値／作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `power(t,f,e,c,g,scratch)` | `g.size()` 項の $$c\exp(e\log(f/f_0))$$。$$f_0\ne0$$、`e,c` は $$[0,p)$$ | $$O(n\log n)$$ |
| `native_exp(in,input,out,n,differentiated=false)` | `input` 個の `Mint` 係数から指数を計算し、空の `vector<Mint>` である `out` へ $$n$$ 項を追加 | $$O(n\log n)$$ |
| `native_power(in,input,out,n,e,c,zeros=0)` | `zeros` 個の零に続けて、`Mint` 入力の $$c\exp(e\log(f/f_0))$$ の $$n$$ 項を `out` へ追加。`c` は `Mint`、返り値は `void` | $$O(n\log n+m+\mathrm{zeros})$$ |

整数配列を直接渡す API の `f,g` はそれぞれ `span<const uint32_t>`・`span<uint32_t>` です。係数は $$[0,p)$$ の通常の剰余値です。`f` の範囲外は 0 として扱います。`f` と `g` は同じ先頭アドレスを指定して上書きできます。それ以外の部分的な重なりは認めません。`g`・`scratch` は `Arena` から確保して先頭アドレスが 32 byte の倍数になるように配置し、作業領域を入力・出力と重ねません。`scratch` の長さは対応する `*_scratch(n)` 以上、`t.lg_max()` は `*_log(n)` 以上とします。`Arena::take()` の呼び出しごとに必要な `Arena::footprint()` の余白も、Arena 全体の確保に含めます。

native 関数は入力を変更せず、入力・出力を重ねません。入力の内部整数値は `Mint` の内部 Montgomery 表現 $$[0,2p)$$ です。入力と出力に末尾の余分な領域や先頭アドレスの配置条件は要求しません。`native_exp` の `differentiated=true` は、入力を通常の指数の引数ではなく $$q=xf'$$ と解釈します。定数項はどちらも 0 とします。`native_power` の入力の定数項は非零です。公開 FPS は $$256\le n\le2^{24}$$ の条件でこれらへ分岐します。

時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。`native_power` は `out` の既存の要素を保持し、`zeros` 個の零と計算結果を末尾へ追記します。`out.size()+zeros+n <= out.max_size()` を満たしてください。零の追記には $$O(\mathrm{zeros})$$ 時間・空間がかかります。既存の要素数を $$m$$ とすると、再確保が必要な場合は $$O(m)$$ 時間・空間が加わります。`out.reserve(out.size()+zeros+n)` で容量を確保しておけば、既存の要素の移動はありません。`*_log()`・`*_scratch()` は $$O(1)$$ 時間・空間です。入力の最大長を $$a$$ とすると native 関数が読む長さは $$\min(a,n)$$ です。
