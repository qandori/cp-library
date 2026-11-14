---
title: "多項式の共通積木"
documentation_of: //components/polynomial_product_tree.hpp
library_category: components/fps
---

`polynomial_detail::ProductTree<F>` は、FPS の型 `F` を受け取る多点評価・補間の内部部品です。[高速 FPS の積木](fps_product_tree.md)と[軽量 FPS の積木](fps_light_product_tree.md)が共有します。通常は各版の多点評価・補間の公開関数を使ってください。

各葉に $$x-x_i$$ を置き、親には子の多項式の積を保存します。評価では子へ多項式の剰余を渡し、補間では積多項式の導関数を評価して各点の重みを求め、子の結果を結合します。積・除算・微分などは `F` へ委譲します。

| 内部 API | 内容・返り値 |
| --- | --- |
| `ProductTree<F>(xs)` | 評価点から構築。`T = F::value_type` とし、`xs` は `vector<T>` |
| `tree.evaluate(f)` | 点列と同じ順序の評価値を `vector<T>` で返す。点の重複を許す |
| `tree.interpolate(ys)` | 点列に対応する値から、点数と同じ係数数の `F` を返す |
| `tree.count` / `tree.size` | 点数／点数以上の最小の 2 冪。空列の `size` は 1 |
| `tree.product` | 1 始まりの節点配列。添字 0 は未使用 |

`product[1]` は $$\displaystyle\prod_{i=0}^{m-1}(x-x_i)$$ です。空の点列では定数 1 を保持し、評価・補間の結果は空列です。補間では点数と値の個数が一致し、各点における積多項式の導関数の値が割り算に使える必要があります。0 でない全ての値で割れる係数型（素数 modint など）なら点が互いに異なることと同値です。構築後の `product`・`size`・`count` は変更しないでください。

点数は $$2^{23}$$ 以下です。点数が上限と一致するときは、最上位の積を分割して計算し、畳み込みを上限内に収めます。`ProductTree<F, Limit>` で畳み込みの上限を別に指定することもできます。評価する多項式の長さと、剰余計算などに必要な長さも `F` の上限に収めてください。係数型と畳み込みの条件は使う FPS に従います。

点数を $$m$$、評価する多項式の係数数を $$n$$ として、構築・補間は $$O(m\log^2 m)$$ 時間、木の保持空間は $$O(m\log m)$$ です。評価は $$O((n+m)\log^2(n+m))$$ 時間・$$O(n+m)$$ 追加空間、補間の追加空間は $$O(m)$$ です。追加空間に構築済みの木は含めません。
