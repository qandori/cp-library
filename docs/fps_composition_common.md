---
title: "FPS の合成・射影の共通変換"
documentation_of: //components/fps_composition_common.hpp
library_category: components/fps
---

[合成の専用変換](fps_compose_blocked.md)と[射影の専用変換](fps_projection_blocked.md)が共有する内部部品です。Linux x86_64 / GCC 向けで、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を実行時に確認します。通常は各演算の公開ヘッダーを include してください。

このページの射影（power projection）は、各冪 $$f(x)^0,f(x)^1,\ldots$$ から、指定した次数の係数、または係数の重み付き和を取り出して並べる操作です。

`fps_detail::composition_poly` に、変換で使う定数の表 `Tables` と、作業領域を各配列に分ける `Carve` などをまとめています。[ブロック変換](fps_blocked.md)の `Arena`・`Transform` と同じ係数表現を使い、合成と射影で共通処理を再利用します。法は 998244353 です。

長さ $$n$$ の変換用の定数表の構築時間・保持空間は $$O(n)$$ です。変換を組み合わせた演算全体の条件・計算量は[合成・射影の内部 API](fps_composition.md)を参照してください。
