---
title: 矩形加算・矩形和
documentation_of: //data_structure/rectangle_add_rectangle_sum.hpp
library_category: data_structure/rectangle_query
---

整数座標のマスに対して、矩形内への加算と矩形内の総和を交互に行います。初期値は全て 0 です。矩形は `[left,right) × [down,up)` で、`left <= x < right`、`down <= y < up` の整数の組 `(x,y)` が対象です。

更新する矩形を先に `reserve` で登録し、`build` してから使います。問い合わせの座標は事前登録が不要です。加算を全て済ませてから問い合わせる場合は、追加空間が小さい [一括処理版](static_rectangle_add_rectangle_sum.md) も使えます。

```cpp
#include "lib/data_structure/rectangle_add_rectangle_sum.hpp"
#include <cassert>

int main() {
	RectangleAddRectangleSum<long long> grid;
	grid.reserve(0, 0, 4, 3);
	grid.reserve(1, 1, 3, 4);
	grid.build();
	grid.add(0, 0, 4, 3, 2);
	assert(grid.sum(0, 0, 4, 3) == 24);
	grid.add(1, 1, 3, 4, -1);
	assert(grid.sum(0, 0, 4, 4) == 18);
	assert(grid.sum(1, 1, 2, 2) == 1);
	assert(grid.prefix_sum(2, 2) == 7);
	grid.clear_values();
	assert(grid.sum(-10, -10, 10, 10) == 0);
}
```

テンプレート引数は `RectangleAddRectangleSum<T = std::int64_t, Coordinate = std::int64_t>` です。`T` が加算値と答えの型、`Coordinate` が両軸の座標型です。座標は負でも構いません。座標の範囲に比例する領域は確保しません。

`reserve` は最初の `build` より前に呼びます。重複した登録も可能です。`add` では、登録した矩形、または 4 隅が登録済みの矩形を使ってください。`build` 後に新しい更新座標は追加できません。`build` を再度呼ぶと、登録座標を保ったまま値を全て 0 に戻します。

どの矩形も `left <= right`、`down <= up` を満たす必要があります。幅または高さが 0 の矩形は、登録・加算では何もせず、和は 0 です。登録がなくても `build` してから問い合わせれば 0 を返します。

`prefix_sum(x,y)` は `a < x`、`b < y` を満たす全ての整数座標 `(a,b)` の値の和です。原点からの和ではありません。有限個の矩形にしか加算していないため、負の座標を含めても和は有限です。

`Coordinate` は `bool` を除く 64 bit 以下の標準整数型です。`T` は既定構築で 0 となり、座標からの構築と `+`・`-`・`*`・`+=`・`-=` を使える型にします。通常の整数型、`__int128_t`、[mint](mint.md) などの剰余型を使えます。剰余型では答えも同じ法で計算します。演算には通常の加減乗算と同じ分配法則・交換法則が必要です。

整数型では最終的な答えだけでなく、座標と加算値の積、二つの座標と加算値の積、それらの部分和も `T` に収まる必要があります。自動で広い整数型には変換しません。例えば座標や値が大きい場合は `RectangleAddRectangleSum<__int128_t>` を選びます。浮動小数点数を使う場合の丸め誤差は取り除きません。登録する空でない矩形は、重複を含めて `4R <= INT_MAX` を満たしてください。

登録した空でない矩形の数を $$R$$、異なる登録済みの隅の数を $$K$$、内部に保持する値の個数を $$S$$ とします。$$K\le 4R$$、$$S=O(K\log K)$$ です。算術演算・座標の比較・コピーを定数時間とします。

| 操作 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `RectangleAddRectangleSum<T,Coordinate>()` | 空の構造を作る | $$O(1)$$ | $$O(1)$$ |
| `reserve(left,down,right,up)` | 更新用の 4 隅を登録。返り値は `void` | 償却 $$O(1)$$ | 償却 $$O(1)$$ |
| `build()` | 登録座標を準備し、全ての値を 0 にする。返り値は `void` | $$O(R\log R)$$ | $$O(R+S)$$ |
| `add(left,down,right,up,value)` | 矩形の各マスに `T value` を加える。返り値は `void` | $$O(\log^2 K)$$ | $$O(1)$$ |
| `sum(left,down,right,up) const` | 矩形内の総和を `T` で返す | $$O(\log^2 K)$$ | $$O(1)$$ |
| `prefix_sum(x,y) const` | `a < x, b < y` の総和を `T` で返す | $$O(\log^2 K)$$ | $$O(1)$$ |
| `clear_values()` | 登録座標を保ったまま全ての値を 0 にする。返り値は `void` | $$O(S)$$ | $$O(1)$$ |

`build` 前の保持空間は $$O(R)$$、構築後は確保済みの容量を含めて $$O(R+S)$$ です。座標がない場合、利用可能な操作は定数時間・空間です。空の矩形の登録・加算・問い合わせも定数時間です。「償却」は、一連の操作の合計時間から求めた 1 回あたりの計算量です。

コピー構築・コピー代入は保持量に比例する時間・追加空間を使い、コピー後は独立して変更できます。移動構築は定数時間・追加空間で、移動代入には代入先の旧領域を解放する時間が加わります。共有する前計算表はありません。
