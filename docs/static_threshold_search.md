---
title: Static Threshold Search
documentation_of: //data_structure/static_threshold_search.hpp
library_category: data_structure/range_query
---

固定した整数列で、しきい値を満たす最初・最後の添字を検索します。標準の `next_ge(l,x)` は `l <= i` かつ `A[i] >= x` を満たす最小の `i` を返します。比較条件・検索方向を型で選びます。`strict` の付いた操作は等号を含めずに比較します。

```cpp
#include "lib/data_structure/static_threshold_search.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<int> a{2, 7, 1, 7, 4};
	StaticThresholdSearch<int> above(a);
	assert(above.next_ge(2, 6) == 3);
	assert(above.next_gt(0, 7) == above.size());
	assert(above.first(0, 3, 6) == 1);
	assert(above.next_record(0) == 1);

	StaticThresholdSearch<int, true> below(a);
	assert(below.next_le(1, 2) == 2);
	assert(below.next_lt(0, 2) == 2);

	StaticThresholdSearch<int, false, true> previous(a);
	assert(previous.prev_ge(4, 7) == 3);
	assert(previous.last(0, 3, 7) == 1);
}
```

型は `StaticThresholdSearch<T = std::int64_t, bool Smaller = false, bool Reverse = false>` です。`T` は `bool` を除く 64 bit 以下の標準整数型で、長さは `INT_MAX` 以下とします。符号付き・符号なしの最小値・最大値も使えます。しきい値は同じ `T` を渡し、厳密不等号を実現するために `x+1`・`x-1` は計算しません。

| 型引数 | 条件・方向 |
| --- | --- |
| `Smaller = false` | `A[i] >= x`、strict 版は `A[i] > x` |
| `Smaller = true` | `A[i] <= x`、strict 版は `A[i] < x` |
| `Reverse = false` | 指定位置以降の最小の添字 |
| `Reverse = true` | 指定端点より前の最大の添字 |

| API | 引数と返り値 |
| --- | --- |
| `StaticThresholdSearch()` | 空のデータ構造 |
| `StaticThresholdSearch(std::vector<T> values)` | 列を値で受け取り保持する。変数をそのまま渡すとコピーし、`std::move` で渡すと領域を移す |
| `build(std::vector<T> values)` | 列を置き換えて再構築。`void` |
| `size()` / `empty()` | 列長を `int` / 空かを `bool` で返す |
| `values()` / `operator[](int i)` | 元の順序の列を `std::span<const T>` / 要素を `const T&` で返す |
| `clear()` / `swap(other)` | 空にする / 内容を交換する。`void` |
| `next(l,x)` / `next_strict(l,x)` | `Reverse=false` で使用。`[l,size())` 内の条件を満たす最小の添字を `int` で返す |
| `first(l,r,x)` / `first_strict(l,r,x)` | `Reverse=false` で使用。`[l,r)` 内の最小の添字を `int` で返す |
| `prev(r,x)` / `prev_strict(r,x)` | `Reverse=true` で使用。`[0,r)` 内の条件を満たす最大の添字を `int` で返す |
| `last(l,r,x)` / `last_strict(l,r,x)` | `Reverse=true` で使用。`[l,r)` 内の最大の添字を `int` で返す |
| `next_ge(l,x)` / `next_gt(l,x)` | `Smaller=false, Reverse=false` の非 strict / strict 検索の別名 |
| `next_le(l,x)` / `next_lt(l,x)` | `Smaller=true, Reverse=false` の非 strict / strict 検索の別名 |
| `prev_ge(r,x)` / `prev_gt(r,x)` | `Smaller=false, Reverse=true` の非 strict / strict 検索の別名 |
| `prev_le(r,x)` / `prev_lt(r,x)` | `Smaller=true, Reverse=true` の非 strict / strict 検索の別名 |
| `next_record(int i)` | `i` より検索方向へ進んだ位置で、`A[i]` より厳密に良い値を持つ最も近い添字を `int` で返す。greater なら大きい値、smaller なら小さい値 |

すべての検索で、不在時は **`size()`** を返します。空のデータ構造では結果は 0 です。重複値を含んでいても添字の最小・最大で決まります。`prev` の `r` は含まない端点なので、`i <= p` から探すには `prev_ge(p+1,x)` などを使います。引数の既定値は型引数だけです。

要素取得と `next_record` には `0 <= i < size()`、1端点の検索には `0 <= l,r <= size()`、区間検索には `0 <= l <= r <= size()` を渡します。空区間の検索結果も `size()` です。型引数と異なる方向・条件のメソッドは使用できず、コンパイル時に検査します。

構築後は読み取り専用です。値の変更には `build` を使います。元の列を変更・破棄しても影響しません。`build`・`clear`・代入・破棄後は以前の参照や `span` を使わないでください。コピーは独立し、ムーブ元は空です。ムーブ・`swap` は `noexcept`。`build` の領域確保失敗時は元の列と検索用データを維持します。

列を 64 要素ずつのブロックに分け、ブロックごとの最小値・最大値を持つ木を構築します。検索開始位置を含むブロックは直接調べ、見つからなければ木をたどって条件を満たす最初・最後のブロックを探します。最後にそのブロック内を調べます。ハッシュや乱数は使いません。

列長を $$n$$ とすると、構築は $$O(n)$$ 時間、保持空間・構築時の空間は $$O(n)$$ です。閾値を指定する検索は最悪 $$O(\log n)$$ 時間、追加空間 $$O(1)$$ です。`next_record` は構築時に求めた添字を返すため、最悪 $$O(1)$$ 時間・追加空間です。ブロック内の走査は固定長なので定数時間です。

サイズ・要素・列の取得・`swap` は $$O(1)$$、コピーと旧領域の解放は $$O(n)$$、ムーブ構築は $$O(1)$$ です。空区間・長さ 0・1 の処理は定数時間です。
