---
title: "疎 FPS の次数圧縮・前処理"
documentation_of: //components/fps_sparse_core.hpp
library_category: components/fps
---

`fps_detail::SparseBase<Engine>` は、疎 FPS の次数の最大公約数による圧縮・小さな整数冪の計算を行う内部部品です。[通常版の疎演算](https://qandori.github.io/cp-library/components/fps_sparse.hpp.html) から使います。項の型・収集処理は [fps_sparse_terms.hpp](fps_sparse_terms.md) で軽量版と共有します。

`Terms`・`Kind`・`collect`・`get` の引数・返り値・前提条件は通常版と同じです。`Engine` は `solve<kind>`・`geometric<exponential>`・`scale` を提供します。`get` は、非零係数の次数が全て $$d$$ の倍数なら次数を $$d$$ で割った小さい列で計算し、結果に 0 を補って元の次数へ戻します。CPU 固有の命令は要求しません。

非零係数数を $$k$$、求める係数数を $$n$$ として、`get` は $$O(nk)$$ 時間・$$O(n+k)$$ 追加空間です。項が空の場合は、結果を初期化する $$O(n)$$ 時間・空間を使います。`collect` は走査する係数数を $$m$$ として $$O(m)$$ 時間、収集した項数に比例する追加空間です。
