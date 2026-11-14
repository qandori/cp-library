---
title: "Static Range Min"
documentation_of: //components/static_range_min.hpp
library_category: components/data_structure
---

```cpp
#include "lib/components/static_range_min.hpp"

StaticRangeMin range(std::vector<int>{5, 2, 7, 1});
auto x = range.prod(0, 3); // 2
auto y = range.all_prod(); // 1
std::vector<std::pair<int, int>> queries{ {0, 2}, {1, 4} };
auto answers = range.prod_many(queries); // {2, 1}
```

更新のない列の区間最小値を求めます。計算量の説明では、32 要素ごとの区画をブロックと呼びます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `StaticRangeMin<T>()` | 空の構造を作る。`T` の既定値は `int` | $$O(1)$$ |
| `StaticRangeMin(values)` | `vector<T>` から構築。型は推論可能 | $$O(n+b\log b)$$ |
| `build(values)` | `vector<T>` から再構築 | $$O(n+b\log b)$$ |
| `size()` | 要素数 | $$O(1)$$ |
| `empty()` | 空か | $$O(1)$$ |
| `operator[](i)` | 元の要素への const 参照。`T=bool` は値で返す | $$O(1)$$ |
| `prod(l,r)` | 非空の半開区間 `[l,r)` の最小値を返す | $$O(1)$$ |
| `prod_many(ranges)` | `span<const pair<int,int>>` の各非空区間の最小値を、入力順の `vector<T>` で返す | $$O(q)$$ |
| `all_prod()` | 列全体の最小値。空列には呼べない | $$O(1)$$ |

`prod_many` は更新のない多数の問い合わせをまとめて処理します。各区間は `0 <= l < r <= size()` とし、空の問い合わせ列には空配列を返します。`q` は問い合わせ数です。

`T` には `<` による順序比較とコピーが必要です。デフォルト構築できない型にも対応します。`values` は値渡しで受け取り、左辺値ならコピー、`std::move(values)` ならムーブして保持します。長さは `INT_MAX` 以下としてください。コピー・ムーブに対応します。

`StaticRangeMin<T,true>` を指定すると、追加の前計算によりブロックをまたぐ問い合わせを高速化します。既定は `false` です。どちらも同じ操作・計算量で使えます。

比較・コピーを $$O(1)$$ として、長さを $$n$$、ブロック数を $$b=\lceil n/32\rceil$$ とすると、構築時間・空間は $$O(n+b\log b)$$、1 区間の参照操作は $$O(1)$$、`prod_many` は $$O(q)$$ 時間・返り値の空間を使います。空列の構築は $$O(1)$$ です。
