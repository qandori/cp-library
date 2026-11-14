---
title: Taylor Shift の中間積
documentation_of: //components/fps_taylor_blocked.hpp
library_category: components/fps
---

`fps/taylor_shift.hpp` が使う内部部品です。Linux x86_64 / GCC で定義され、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を確認します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::taylor_detail::shift(f,c)` | f(x+c) の係数列で f を更新 | $$O(n\log n)$$ |

通常は [Taylor shift](https://qandori.github.io/cp-library/fps/taylor_shift.hpp.html) の `taylor_shift(f,c)` を使います。`n` は f の係数数です。

`fps_detail::taylor_detail::shift(f,c)` は内部 `Mint` の係数列 `f` を $$f(x+c)$$ の係数列へ書き換えます。前提は $$256\leq \lvert f\rvert\leq 2^{19}$$、$$c\neq0$$、法 $$998244353$$ です。長さは変わりません。公開関数は範囲外や他の環境で既存実装を使います。

係数数を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。
