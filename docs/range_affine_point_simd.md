---
title: 区間アフィン変換・一点取得の共通実装
documentation_of: //components/range_affine_point_simd.hpp
library_category: components/data_structure
---

[区間アフィン変換・一点取得](range_affine_point_get.md)が使う内部処理です。通常は公開クラス `RangeAffinePointGet` を使います。

実行中に変わらない奇数の法が $$2^{30}$$ 未満で AVX2（複数の整数を一度に演算する CPU 命令）を使える場合に、各値を `a*x+b` にする更新を管理します。それ以外では公開クラスが汎用の双対セグメント木を使います。

列長を $$n$$ として、構築時間・使用空間は $$O(n)$$、更新・一点取得は $$O(\log n)$$ 時間です。
