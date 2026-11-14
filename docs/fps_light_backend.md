---
title: "軽量 FPS の共通実装"
documentation_of: //components/fps_light_backend.hpp
library_category: components/fps
---

軽量 FPS と追加演算の共通部品です。通常は公開ヘッダーを include してください。

FPS の公開ヘッダーは `fps/fps_light.hpp`・`fps/fps_light_algorithms.hpp`、積木は `components/fps_light_product_tree.hpp` です。NTT だけを使う場合は `components/fps_light_ntt.hpp` を読み込みます。複数を include しても定義は一度だけで、通常版 FPS の実装は読み込みません。

FPS の型と基本演算は [基本演算の共通部品](fps_light_core.md) に置き、このヘッダーで積木と追加演算を組み合わせます。基本演算だけが必要な内部部品は `components/fps_light_core.hpp` を include できます。

係数数を $$n$$、評価点数を $$m$$、分子・分母の最大係数数または漸化式の次数を $$d$$ とすると、Taylor shift は $$O(n\log n)$$ 時間・$$O(n)$$ 追加空間、Bostan–Mori と線形漸化式は項番号を $$k$$ として $$O(d\log d\log k)$$ 時間・$$O(d)$$ 追加空間です。多点評価は $$O((n+m)\log^2(n+m))$$ 時間・$$O((n+m)\log(n+m))$$ 追加空間、補間は $$O(m\log^2 m)$$ 時間・$$O(m\log m)$$ 追加空間です。基本演算の時間・空間は [軽量 FPS](fps_light.md) と同じです。include だけで実行時の前計算は行いません。
