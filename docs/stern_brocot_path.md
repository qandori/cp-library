---
title: Stern–Brocot Tree の圧縮経路
documentation_of: //components/stern_brocot_path.hpp
library_category: components/math
---

[Stern–Brocot Tree](https://qandori.github.io/cp-library/math/stern_brocot_tree.hpp.html) の内部部品です。同じ方向への連続移動を、その方向と移動回数にまとめて順に取り出します。

| 関数・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `stern_brocot_detail::Path<U>` | 符号なし整数の分子・分母から、根からの移動方向・回数を順に取り出す型 | —（型） |
| `Path(numerator,denominator)` | 正の分子・分母から構築。約分されていなくてもよい | $$O(1)$$ |
| `empty()` | 経路を最後まで取り出したか | $$O(1)$$ |
| `right()` | 次の区間が右への移動であるか。`!empty()` が必要 | $$O(1)$$ |
| `pop()` | 次の区間の移動回数を返し、残りの経路へ進む。`!empty()` が必要。方向は呼ぶ前に `right()` で取得 | $$O(1)$$ |

構築・各操作は $$O(1)$$ 時間、経路全体は分子・分母の最大値を $$M$$ として $$O(\log M)$$ 時間、追加空間は $$O(1)$$ です。
