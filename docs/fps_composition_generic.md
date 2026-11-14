---
title: "FPS の合成・射影の汎用実装"
documentation_of: //components/fps_composition_generic.hpp
library_category: components/fps
---

合成・power projection・合成逆元を FPS の型で共通化した内部実装です。軽量 FPS と、通常版 FPS で CPU 固有の専用処理を使わない場合に共有します。通常は各演算の公開ヘッダーを include してください。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

積・逆数・対数・指数などの演算には、渡された形式的冪級数の型に備わる関数を使います。係数の表現や積の計算方法は、その型に従います。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、各段の分母を含む追加空間は $$O(n\log n)$$ です。
