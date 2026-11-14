---
title: FPS の合成逆元の汎用実装
documentation_of: //components/fps_reversion_generic.hpp
library_category: components/fps
---

`fps_detail::composition_fallback::compositional_inverse(f,deg = -1)` は、`f(g(x)) = x mod x^deg` を満たす `g` を、長さ `deg` の FPS 型 `F` で返します。通常は [合成逆元](compositional_inverse.md) の公開ヘッダーを使います。

既定の長さは `f.size()` です。`0 <= deg <= 1 << 21`、定数項は0とし、`deg >= 2` なら `f.size() >= 2` かつ `f[1] != 0` が必要です。`deg = 0` なら空列、`deg = 1` なら長さ1の0列です。入力は変更しません。

内部で使う `power_projection(f,n,deg = -1)` も提供します。各 `k` について `f(x)^k` の `x^n` の係数を、長さ `deg` の `F` で返します。`0 <= n < 1 << 21`、`0 <= deg <= 1 << 22` とし、既定の長さは `n+1` です。入力は変更しません。

係数型には四則演算・逆元・`mod()`・0との比較、FPS 型には積・対数・指数と、指定個数より後の係数を除く操作が必要です。法は必要な整数の逆元が存在するものを使います。対象の係数数を $$n$$ とし、畳み込みが $$O(n\log n)$$ 時間で行える場合、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
