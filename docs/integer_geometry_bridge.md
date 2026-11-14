---
title: "整数幾何の座標変換"
documentation_of: //components/integer_geometry_bridge.hpp
library_category: components/geometry
---

`integer_geometry_detail` 名前空間の内部部品です。整数幾何の各ヘッダーから使われるため、通常は直接 include する必要はありません。

| 操作 | 内容・返り値 |
| --- | --- |
| `to_geometry(p)` | `integer_geometry::BasicPoint<T>` を `geometry::Point<T>` へ変換 |
| `from_geometry(p)` | `geometry::Point<T>` を `integer_geometry::BasicPoint<T>` へ変換 |
| `to_geometry(points)` / `from_geometry(points)` | 対応する `std::vector` の点列を変換 |
| `from_geometry(location)` | `geometry::Location` を `integer_geometry::Location` へ変換 |

座標型 `T` は64 bit以下の符号付き組み込み整数型です。座標の値と型を保ち、返り値は入力を参照しません。空の点列も扱えます。

単点の変換は $$O(1)$$ 時間・追加空間、長さ $$n$$ の点列の変換は $$O(n)$$ 時間・追加空間です。
