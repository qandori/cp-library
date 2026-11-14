---
title: 静的な矩形加算・矩形和
documentation_of: //data_structure/static_rectangle_add_rectangle_sum.hpp
library_category: data_structure/rectangle_query
---

整数座標のマスに矩形加算を行った後、複数の矩形和をまとめて求めます。初期値は全て 0 です。矩形は `[left,right) × [down,up)` で、条件を満たす整数座標のマスが対象です。加算と問い合わせを交互に実行する場合は [矩形加算・矩形和](rectangle_add_rectangle_sum.md) を使います。

```cpp
#include "lib/data_structure/static_rectangle_add_rectangle_sum.hpp"
#include <cassert>

int main() {
	StaticRectangleAddRectangleSum<long long> grid;
	grid.add_rectangle(0, 0, 4, 3, 2);
	grid.add_rectangle(1, 1, 3, 4, -1);
	int id = grid.add_query(0, 0, 4, 4);
	auto answers = grid.solve();
	assert(answers[id] == 18);
}
```

テンプレート引数は `StaticRectangleAddRectangleSum<T = std::int64_t, Coordinate = std::int64_t>` です。引数の順序は左・下・右・上です。加算値を指定する場合は最後に渡します。`add_query` は 0 始まりの問い合わせ番号を返し、`solve` は同じ順序の `std::vector<T>` を返します。重複する加算は全て加算し、重複する問い合わせも別々の答えを返します。

加算と問い合わせは任意の順序で登録できます。`solve` は、問い合わせを登録した時点に関係なく、登録済みの**全ての加算**を反映します。`solve` は登録内容を変更せず、繰り返し呼べます。その後に登録内容を追加してから再度呼ぶこともできます。

矩形は `left <= right`、`down <= up` を満たす必要があります。幅または高さが 0 の加算は何もせず、そのような問い合わせの答えは 0 です。加算がなければ全て 0、問い合わせがなければ空配列を返します。

`Coordinate` は `bool` を除く 64 bit 以下の標準整数型で、負の座標も使えます。`T` は既定構築で 0 となり、座標からの構築、通常の整数と同じ規則の加減乗算、`+=`・`-=` が必要です。整数型、`__int128_t`、[mint](mint.md) などの剰余型を使えます。剰余型ではその法で計算します。`T` が `bool` を除く組み込みの整数型または `__int128_t` の場合、加算値と返す和が `T` に収まる範囲で使います。答えが 64 bit に収まらない場合は、例えば `StaticRectangleAddRectangleSum<__int128_t>` を選びます。それ以外の型では、座標との積や部分和などの計算途中もその型で扱える必要があります。浮動小数点数の場合の丸め誤差は取り除きません。

空でない加算の登録数を $$N$$、問い合わせの登録数を $$Q$$ とし、$$N+Q\le\lfloor\mathrm{INT\_MAX}/2\rfloor$$ を満たしてください。

| 操作 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `StaticRectangleAddRectangleSum<T,Coordinate>()` | 空の構造を作る | $$O(1)$$ | $$O(1)$$ |
| `add_rectangle(left,down,right,up,value)` | 各マスに `T value` を加える矩形を登録。返り値は `void` | 償却 $$O(1)$$ | 償却 $$O(1)$$ |
| `add_query(left,down,right,up)` | 問い合わせを登録し、番号を `int` で返す | 償却 $$O(1)$$ | 償却 $$O(1)$$ |
| `solve() const` | 全問い合わせの答えを `std::vector<T>` で返す | $$O((N+Q)\log(N+Q))$$ | $$O(N+Q)$$ |

保持空間は確保済みの容量に比例する $$O(N+Q)$$、`solve` の返り値を含む追加空間も $$O(N+Q)$$ です。算術演算・比較・コピーを定数時間とします。加算がない場合の `solve` は $$O(Q)$$ 時間・空間、問い合わせがない場合は定数時間・空間です。空の加算矩形の登録は定数時間です。登録数が定数個の場合の対数は定数として扱います。「償却」は、一連の操作の合計時間から求めた 1 回あたりの計算量です。

コピー構築・コピー代入は保持量に比例する時間・追加空間を使い、コピー後は独立して変更できます。移動構築は定数時間・追加空間で、移動代入には代入先の旧領域を解放する時間が加わります。共有する前計算表はありません。
