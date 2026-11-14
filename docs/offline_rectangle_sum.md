---
title: 静的矩形和（クエリ先読み）
documentation_of: //data_structure/offline_rectangle_sum.hpp
library_category: data_structure/rectangle_query
---

重み付きの点と矩形を登録し、各半開矩形に含まれる点の重みの和をまとめて求めます。

```cpp
#include "lib/data_structure/offline_rectangle_sum.hpp"

OfflineRectangleSum<> rectangles;
rectangles.add_point(1, 2, 10);
rectangles.add_point(3, 4, -2);
int id = rectangles.add_query(0, 0, 3, 5);
auto answers = rectangles.solve(); // answers[id] == 10
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `OfflineRectangleSum<X=int64_t,Y=X,Sum=int64_t>(point_capacity=0,query_capacity=0)` | 空の構造を作り、点数・矩形数の容量を予約 | $$O(1)$$ |
| `add_point(x,y,weight)` | 重み付きの点を追加。同じ座標も別々に数える | 償却 $$O(1)$$ |
| `add_query(left,down,right,up)` | `[left,right) × [down,up)` を追加し、0 始まりの問い合わせ番号を返す | 償却 $$O(1)$$ |
| `solve() const` | 登録した全点を対象に、問い合わせ順の `vector<Sum>` を返す | $$O((n+q)\log(n+q))$$ |

幅または高さが 0 以下の矩形の答えは 0 です。`solve` は登録内容を変更せず、繰り返し呼べます。問い合わせの後に追加した点も、次の `solve` では全て対象になります。

座標型 `X`・`Y` にはコピーと `<` による大小比較が必要です。大小比較では `a < a` を `false` とし、`a < b` かつ `b < c` なら `a < c` とします。どちら向きの `<` も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。デフォルト構築できない座標型にも対応します。`Sum` は 0 の構築・加算・減算に対応する型とし、重みと答えが表現できるように指定します。64 bit 以下の整数型では中間値のオーバーフローを避けるため、必要に応じて 128 bit 整数を使います。それ以外の型では中間の部分和も表現できる必要があります。点数・矩形数はそれぞれ `INT_MAX` 以下とします。

点数を $$n$$、矩形数を $$q$$ として、`solve` は $$O((n+q)\log(n+q))$$ 時間・$$O(n+q)$$ 追加空間です。点数・矩形数の確保容量をそれぞれ $$C_p,C_q$$ として、保持空間は $$O(C_p+C_q)$$ です。構築時の容量指定は非負としてください。比較・算術演算・コピーを定数時間とします。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
