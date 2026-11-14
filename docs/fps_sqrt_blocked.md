---
title: FPS 平方根のブロック変換
documentation_of: //components/fps_sqrt_blocked.hpp
library_category: components/fps
---

`fps::sqrt()` が使う内部部品です。通常は公開 FPS API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側で AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_sqrt(in,input,out,n,constant)` | 定数項を指定して平方根を `out` の末尾へ追加。返り値は `void` | $$O(n\log n+s)$$ |
| `sqrt_log(n)` | 必要な変換長の $$\log_2$$ | $$O(1)$$ |
| `sqrt_scratch(n)` | 必要な作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `sqrt(t,f,c,g,scratch)` | 平方根を通常表現で計算 | $$O(n\log n)$$ |
| `sqrt_steps(t,f,c,a,b,ht)` | 平方根の計算済み係数数を倍に増やす | $$O(m\log m)$$ |
| `sqrt_last_step(t,f,gt,ht,work,hi)` | 最後の半分を復元 | $$O(m\log m)$$ |

関数は `fps_detail::blocked_poly` 内にあります。通常は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f.sqrt(n)` を使います。表の `n` は出力係数数、`m` は更新に使う変換長です。

`fps_detail::blocked_poly::native_sqrt(in,input,out,n,constant)` は、`in` の `input` 個の `Mint` 係数から、定数項が `constant` の平方根 $$g^2=f\bmod x^n$$ を求めます。結果の `n` 項を `vector<Mint>` である `out` の末尾へ追加し、既存の要素は保持します。`1 <= n <= 2^23`、`input >= 1`、定数項が非零、`constant * constant == in[0]` を前提とします。内部の 32 bit 整数値の範囲は $$[0,2p)$$、$$p=998244353$$ です。入力の範囲外を 0 とし、次数 `n` 以降は読みません。入力は変更せず、`out` と重ねません。公開配列に末尾の余分な領域や先頭アドレスの配置条件は要求しません。

公開 FPS 側で先頭の零係数と定数項の平方根を確認してから、この関数を呼びます。先頭の零の個数が奇数、または定数項の平方根が法上に存在しない場合は、公開 API が空配列を返します。入力が零多項式の場合も公開 API 側で処理します。

通常の剰余値 $$[0,p)$$ から直接使う内部 API は次のとおりです。

- `sqrt_log(n)` は必要な `Transform` の `lg_max()` の値を返します。
- `sqrt_scratch(n)` は `sqrt` が必要とする作業領域の 32 bit 整数の要素数を返します。
- `sqrt(t,f,c,g,scratch)` は `g.size()` 項の平方根を求めます。`f`・`g`・作業領域を重ねず、`c*c == f[0] != 0` とします。`g` と作業領域は `Arena` から確保し、`t.lg_max() >= sqrt_log(g.size())` を満たします。
- `sqrt_steps(t,f,c,a,b,ht)` は 64 以上の 2 冪 `m=a.size()` まで平方根を求め、`ht` に $$-1/g\bmod x^{m/2}$$ の長さ `m` の変換を残します。`a`・`b`・`ht` は重ならない `Arena` の長さ `m` の領域です。
- `sqrt_last_step(t,f,gt,ht,work,hi)` は、長さ `m` の `gt`・`ht` から続く `hi.size()` 項を求めます。`gt` は平方根の長さ `m` の変換、`ht` は `sqrt_steps` の出力とし、`1 <= hi.size() <= m` とします。`work` は長さ `m` の `Arena` 領域です。`hi` は `f` の次数 `m` 以降と同じ領域でも構いません。他の領域は重ねません。

作業領域は `Arena` の破棄時に自動で解放します。確保できなければ例外を送出します。

**計算量**: 平方根全体は $$O(n\log n)$$ 時間、$$O(n)$$ 追加空間です。`native_sqrt` で `out` の再確保が必要な場合は、既存の要素数 $$s$$ に比例する時間・空間が加わります。`out.reserve(out.size()+n)` で容量を確保しておけば、既存の要素の移動はありません。`sqrt_log`・`sqrt_scratch` は $$O(1)$$ 時間です。
