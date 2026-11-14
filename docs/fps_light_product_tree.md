---
title: "軽量 FPS の積木"
documentation_of: //components/fps_light_product_tree.hpp
library_category: components/fps
---

```cpp
#include "lib/components/fps_light_product_tree.hpp"

std::vector<mint> xs{0, 1, 2};
fps_light_detail::ProductTree<mint> tree(xs);
auto values = tree.evaluate(fps{1, 2, 3}); // {1,6,17}
```

[軽量 FPS の多点評価・補間](https://qandori.github.io/cp-library/fps/fps_light_algorithms.hpp.html) が共有する内部部品です。`fps_light_detail::ProductTree<T>` は[共通積木](polynomial_product_tree.md)の `ProductTree<FormalPowerSeries<T>>` の別名です。各葉に一次式を置き、上の節点ほど積を保持します。評価は多項式の剰余を子へ渡して行います。通常は公開関数 `multipoint_eval`・`interpolate` を使ってください。

点数を $$m$$、評価する多項式の係数数を $$n$$ とします。

| 要素 | 内容 | 時間計算量 |
| --- | --- | --- |
| `ProductTree<T>(xs)` | 評価点から積木を構築。点の重複・空の点列を許す | $$O(m\log^2 m)$$ |
| `tree.evaluate(f)` | `xs` の順の評価値を `std::vector<T>` で返す | $$O((n+m)\log^2(n+m))$$ |
| `tree.interpolate(ys)` | 対応する値から点数と同じ係数数の `FormalPowerSeries<T>` を返す。点は互いに異なる必要がある | $$O(m\log^2 m)$$ |
| `tree.count` / `tree.size` | 点数／点数以上の最小の 2 冪（空なら 1） | 参照: $$O(1)$$ |
| `tree.product` | 1 始まりの節点配列。添字 0 は未使用 | 参照: $$O(1)$$ |

`product[1]` は $$\displaystyle \prod_{i=0}^{m-1}(x-x_i)$$ です。空の点列では定数多項式 1 です。`product[size+i]` は有効な点では $$x-x_i$$、末尾の埋め草では定数 1 です。構築後は `product`・`size`・`count` を変更しないでください。

法と係数型は [軽量 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) と同じ条件です。点数は $$2^{23}$$ 以下です。最上位の積が畳み込みの上限を超える場合は、共通積木が分割して計算します。評価する多項式の長さと、剰余計算などに必要な長さも軽量 FPS の上限に収めてください。

木の保持は $$O(m\log m)$$ 空間です。補間は $$O(m)$$ 追加空間、評価は $$O(n+m)$$ 追加空間で、構築済みの積木の空間は別に必要です。空の点列では構築・評価・補間とも $$O(1)$$ 時間・空間です。
