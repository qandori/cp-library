---
title: 矩形和の四つの係数
documentation_of: //components/rectangle_sum_polynomial.hpp
library_category: components/data_structure
---

[矩形加算・矩形和](rectangle_add_rectangle_sum.md) と [一括処理版](static_rectangle_add_rectangle_sum.md) の内部部品です。通常は公開ヘッダーを使います。

`rectangle_sum_detail::Polynomial<T>` は `T weight, x, y, xy` を保持し、座標 `(a,b)` における `(weight*a-x)*b-y*a+xy` を表します。`T` の既定構築は 0、加減乗算は通常の整数と同じ規則を満たす必要があります。整数型では全ての中間値が収まる型を使ってください。

| 操作 | 内容・返り値 | 時間計算量 | 使用空間 |
| --- | --- | --- | --- |
| `Storage<T>::type` | 公開ライブラリの内部計算に使う型。組み込み整数型は 32 bit 以上で十分な幅の符号なし整数型、`__int128_t` は `__uint128_t`、それ以外は `T` | —（型） | —（型） |
| `Polynomial<T>()` | 四つの係数を 0 にする | $$O(1)$$ | $$O(1)$$ |
| `Polynomial<T>::corner(a,b,value)` | `value*(x-a)*(y-b)` を表す係数を返す。引数の型は全て `T` | $$O(1)$$ | $$O(1)$$ |
| `operator+=` / `operator-=` | 各係数を加減し、自身への参照を返す | $$O(1)$$ | $$O(1)$$ |
| `evaluate(a,b) const` | 座標 `(a,b)` における値を `T` で返す | $$O(1)$$ | $$O(1)$$ |

`T` の演算・コピーを定数時間とします。オブジェクトの保持空間は $$O(1)$$ で、共有表や動的な領域確保はありません。
