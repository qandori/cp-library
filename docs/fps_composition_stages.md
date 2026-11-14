---
title: FPS の合成・射影の共通段
documentation_of: //components/fps_composition_stages.hpp
library_category: components/fps
---

`fps_detail::composition_fallback` の内部部品です。通常は [合成](compose.md)・[power projection](power_projection.md)・[合成逆元](compositional_inverse.md) の公開ヘッダーを使います。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

`composition_stages(f,n,count)` は、定数項が0の FPS `f` と正整数 `n,count` から、`std::vector<CompositionStage<F>>` を返します。`count <= n <= 1 << 22` とし、係数型は加減乗算ができるものを使います。各段は `width,before,after,parity` と偶数・奇数係数列 `even,odd` を持ちます。入力は変更しません。

`trim(f)` は末尾の0を削除します。`multiply(a,b)` は FPS 型の積を返します。積の係数数を $$n$$ とし、畳み込みが $$O(n\log n)$$ 時間で行える場合、各段の構築は全体で $$O(n\log^2 n)$$ 時間・$$O(n\log n)$$ 追加空間です。
