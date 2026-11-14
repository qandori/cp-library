---
title: 多点評価の専用処理
documentation_of: //components/fps_multipoint.hpp
library_category: components/fps
---

`fps_detail::evaluation_poly` の多点評価用の内部部品です。通常は [多点評価](multipoint_eval.md) を使います。Linux x86_64 / GCC で定義され、AVX2（複数の整数をまとめて処理する CPU 命令）が必要です。

`native_evaluate(f,points)` は `mint` の係数列と評価点から評価値の `vector<mint>` を返します。評価点の重複を許します。内部の 32 bit 整数値を扱う `evaluate` と必要な作業領域を返す `evaluate_words` の前提条件は [まとめページ](fps_evaluation.md) を参照してください。

係数数を $$n$$、点数を $$m$$ として、時間は $$O((n+m)\log^2(n+m))$$、追加空間は $$O((n+m)\log(n+m))$$ です。
