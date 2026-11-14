---
title: "多点評価・補間の内部木"
documentation_of: //components/fps_product_tree.hpp
library_category: components/fps
---

`fps_detail::ProductTree<T>` は [多点評価](https://qandori.github.io/cp-library/fps/multipoint_eval.hpp.html)・[補間](https://qandori.github.io/cp-library/fps/interpolate.hpp.html)が共有する内部部品です。通常は各関数の公開ヘッダーを include してください。

[共通積木](polynomial_product_tree.md)に `FormalPowerSeries<T>` を渡した実装を継承します。`ProductTree<T>(xs)` で構築し、`tree.evaluate(f)`・`tree.interpolate(ys)` を使えます。点列と同じ順序で評価し、補間では互いに異なる点を使います。内部の積・剰余・微分は高速 FPS に委譲します。

従来の static API も互換用に残しています。

| 内部 API | 内容・返り値 |
| --- | --- |
| `build(xs,size,block,reciprocal,frequency = nullptr)` | `block` 点ずつの一次式をまとめた木を返す。`size` を必要な 2 冪まで増やす。`reciprocal` が真なら各葉の積は $$\displaystyle\prod_j(1-x_jx)$$、偽なら $$\displaystyle\prod_j(x-x_j)$$ |
| `evaluate(points,tree,frequency,size,block,root,consume = false)` | 従来の中間表現 `root` から多項式を復元し、共通積木で評価する |

互換 API の `frequency` は変換キャッシュを保持せず、`build` では空の各要素で初期化します。static `evaluate` の `frequency`・`consume` は結果に影響しません。専用の評価・補間の木の経路は[評価・補間の専用実装](fps_evaluation.md)が扱います。

点数を $$m$$、評価する多項式の係数数を $$n$$ として、通常の構築・補間は $$O(m\log^2 m)$$ 時間・木の保持に $$O(m\log m)$$ 空間、評価は $$O((n+m)\log^2(n+m))$$ 時間です。構築済みの木を除く追加空間は、評価が $$O(n+m)$$、補間が $$O(m)$$ です。空の点列の構築・評価・補間は $$O(1)$$ 時間・空間です。係数型・点数・畳み込みの上限は[共通積木](polynomial_product_tree.md)と同じです。

互換 `build` では、呼び出し前の `size` を正の 2 冪、`block` を正整数とします。$$b=\texttt{block}$$、$$g=\lceil m/b\rceil$$、更新後の `size` を $$s$$ とすると、時間は $$O(s+mb+m\log^2 m)$$、返す木と `frequency` の保持空間は $$O(s+gb+m\log m)$$ です。$$gb$$ は葉が確保する係数の容量を含みます。`block` を大きくすると葉の構築に使う時間も増えます。

互換 `evaluate` には、対応する `build` の木と同じ `size`・`block` を渡します。`size * block` は `int` の範囲に収めてください。$$R$$ を `root.size()`・`points.size()`・`size * block` の最大値とし、復元する多項式と剰余計算に必要な長さを FPS の上限内に収めると、時間は $$O(R\log^2 R)$$、追加空間は $$O(R\log R)$$ です。渡した木の保持空間はこれに含めません。
