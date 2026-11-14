---
title: "軽量 FPS の合成・射影の共通実装"
documentation_of: //components/fps_composition_light.hpp
library_category: components/fps
---

[軽量 FPS の合成](https://qandori.github.io/cp-library/fps/compose_light.hpp.html)、[power projection](https://qandori.github.io/cp-library/fps/power_projection_light.hpp.html)、[合成逆元](https://qandori.github.io/cp-library/fps/compositional_inverse_light.hpp.html)で共有する実装です。通常は各公開ヘッダーを include してください。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

[合成・係数取得の汎用実装](fps_composition_generic.md)に軽量 FPS の型を渡します。積などの級数演算は全て軽量版を使い、通常版の FPS 本体は読み込みません。

対象の係数数を $$n$$ として、時間は $$O(n\log^2 n)$$、保持する各段の分母を含む追加空間は $$O(n\log n)$$ です。
