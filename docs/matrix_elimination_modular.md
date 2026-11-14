---
title: "固定法の行列消去"
documentation_of: //components/matrix_elimination_modular.hpp
library_category: components/math
---

[行列](matrix.md) が使う固定法の消去処理です。通常は `Matrix<T>` の行列式・階数・逆行列・連立一次方程式を使います。

`matrix_detail::ModularElimination<Ops>` は、`Ops::V` が `uint32_t`、法が $$1<p<2^{31}$$ の素数の場合に使います。配列の値は $$[0,p)$$ に収めます。配列は各行を左から右へ、上の行から順に並べます。「ピボット」は、消去法で各行の計算の基準に選ぶ 0 でない成分です。

| 操作 | 内容 |
| --- | --- |
| `run(a,h,stride,columns,reduced)` | 行優先の配列 `a` を消去し、ピボット列と交換の符号を含むピボットの積を返す。`reduced` が真なら簡約化も行う |
| `update(dst,src,coeff,count,first,length)` | `dst[j] += sum_k coeff[k] * src[k][first+j]` を法で計算する |

`run` は $$h$$ 行、各行 `stride` 要素を持つ配列の先頭 `columns` 列を対象とします。`0 <= columns <= stride` とします。`update` の `count` は `batch` 以下とし、各領域に `length` 要素を確保します。

消去は時間 $$O(h\cdot\mathrm{stride}\cdot\min(h,\mathrm{columns}))$$、追加空間 $$O(h+\min(h,\mathrm{columns}))$$。`update` は時間 $$O(\mathrm{count}\cdot\mathrm{length})$$、追加空間 $$O(1)$$ です。
