---
title: FPS の合成の汎用実装
documentation_of: //components/fps_compose_generic.hpp
library_category: components/fps
---

`fps_detail::composition_fallback::compose(outer,inner,deg = -1)` は、FPS 型 `F` の2つの列から `outer(inner(x)) mod x^deg` を `F` で返します。通常は [合成](compose.md) の公開ヘッダーを使います。

`deg = -1` なら `outer.size()` 項を求めます。`0 <= deg <= 1 << 22` とし、`inner` の定数項は0とします。空入力も扱え、`deg = 0` なら空列、`outer` が空なら長さ `deg` の0列です。入力は変更しません。係数型には加減乗算・0との比較が必要です。

求める係数数を $$n$$ とし、畳み込みが $$O(n\log n)$$ 時間で行える場合、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
