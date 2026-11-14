---
title: 多点評価・補間の内部処理
documentation_of: //components/fps_evaluation.hpp
library_category: components/fps
---

多点評価・補間の内部部品をまとめて読み込みます。通常は `fps/multipoint_eval.hpp` または `fps/interpolate.hpp` を使います。それぞれ必要な処理だけを読み込みます。Linux x86_64 / GCC で定義され、呼び出し側が AVX2（複数の整数をまとめて処理する CPU 命令）対応を確認します。

Montgomery 表現では、係数 $$c$$ を $$c\,2^{32}$$ と法 $$p=998244353$$ で合同な整数として保持します。通常の剰余表現は、$$0$$ 以上 $$p$$ 未満の余りそのものです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_evaluate(f,points)` | 各点での評価値を返す | $$O((n+m)\log^2(n+m))$$ |
| `native_interpolate(points,values)` | 点と値から係数列を返す | $$O(m\log^2 m)$$ |
| `evaluate(arena,f,points,values)` | 通常表現での多点評価 | $$O((n+m)\log^2(n+m))$$ |
| `interpolate(arena,points,values,c)` | 通常表現での補間 | $$O(m\log^2 m)$$ |
| `evaluate_words(n,m)` | 多点評価の作業領域の 32 bit 整数の要素数 | $$O(1)$$ |
| `interpolate_words(m)` | 補間の作業領域の 32 bit 整数の要素数 | $$O(1)$$ |

関数は `fps_detail::evaluation_poly` 内にあります。`n` は多項式の係数数、`m` は評価点数です。

`fps_detail::evaluation_poly::native_evaluate(f,points)` は内部 `Mint` の入力から評価値の `vector<Mint>` を返します。入力は変更せず、評価点の重複を許します。`native_interpolate(points,values)` は相異なる点での値から係数列を返します。こちらの入力長は等しく、非零です。呼び出し側は入力長・点数がそれぞれ $$2^{23}$$ 以下であることを確認します。

共有する積の木は [fps_evaluation_common.hpp](fps_evaluation_common.md)、評価処理は [fps_multipoint.hpp](fps_multipoint.md)、補間処理は [fps_interpolation.hpp](fps_interpolation.md) に分かれています。

内部の 32 bit 整数値を扱う `evaluate(arena,f,points,values)` と `interpolate(arena,points,values,c)` の入出力は通常表現 $$[0,p)$$、木の係数は内部で Montgomery 表現を使います。ここで $$p=998244353$$ です。`evaluate_words(n,m)`・`interpolate_words(m)` は作業領域の必要量を返します。この部品の `Arena` は、確保した領域を 0 で初期化します。公開配列に末尾の余分な領域や先頭アドレスの配置条件は要求しません。

係数数を $$n$$、点数を $$m$$ として、時間は $$O((n+m)\log^2(n+m))$$、追加空間は $$O((n+m)\log(n+m))$$ です。
