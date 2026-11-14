---
title: FPS の一点評価
documentation_of: //components/fps_horner.hpp
library_category: components/fps
---

`fps_detail::horner(a,n,x)` は、`Mint` の係数配列で表された多項式を一点 `x` で評価する内部部品です。通常は公開関数 `fps::eval(x)` を使います。係数は定数項から並べます。空の多項式は 0 を返し、`x=0` のときは定数項を返します。入力は変更せず、追加の先頭アドレスの配置条件や末尾の余分な領域を要求しません。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::horner(a,n,x)` | a[0] から a[n-1] の多項式を x で評価 | $$O(n)$$ |

通常は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f.eval(x)` を使います。例えば `fps{1,2,3}.eval(2).val()` は 17 です。

`Mint` の内部の 32 bit 整数値は $$[0,2p)$$、$$p=998244353$$ の Montgomery 表現とします。

**計算量**: 係数数を $$n$$ として $$O(n)$$ 時間、追加空間は $$O(1)$$ です。
