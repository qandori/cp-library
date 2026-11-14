---
title: "非負整数列の一括出力"
documentation_of: //components/fast_io_output_values.hpp
library_category: components/util
---

[FastOutput](fast_io.md) を使い、非負整数の列を空白区切りなどでまとめて出力します。64 bit の符号なし整数の最大値まで扱えます。この機能を使うときは `fast_io_output_values.hpp` を include します。

```cpp
#include "lib/components/fast_io_output_values.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

int main() {
	const std::array<std::uint64_t, 4> values{0, 99, 100000000, UINT64_MAX};
	FastOutput out;
	write_unsigned_values(out, values.size(), [&](std::size_t i) {
		return values[i];
	});
	return out.flush() ? 0 : 1;
}
```

出力は次のとおりです。

```text
0 99 100000000 18446744073709551615
```

| 操作 | 内容・返り値 |
| --- | --- |
| `write_unsigned_values<UInt = std::uint64_t>(out,count,read,separator = ' ',ending = '\n')` | `FastOutput& out` へ `read(i)` が返す整数を `size_t count` 個出力する。返り値は `void` |

引数は出力先・個数・値を返す関数・区切り文字・末尾の文字の順です。`read(i)` は `std::size_t` を 1 個受け取る関数で、`i=0,1,...,count-1` の順に 1 回ずつ呼ばれます。返す値は 0 以上かつ `UInt` の最大値以下にしてください。`UInt` には `bool` を除く 64 bit 以下の符号なし整数型を指定できます。

各値を十進数で出力し、値の間に `separator`、最後の値の後に `ending` を出力します。`count=0` は何も出力せず、`read` も呼びません。例えば `separator=','`・`ending=';'` なら `0,3,6;` のような形式を指定できます。

通常の `out.write`・`out.println` と混在できます。`read` の中から同じ `out` を使わないでください。最後に `out.flush()` を呼ぶか、`out` を破棄すると残った出力を書き込みます。バッファを空ける処理で書き込みに失敗すると `std::runtime_error` を送出し、最後の `flush()` は成功時に `true` を返します。同じ `FastOutput` を複数スレッドから同時に使わないでください。

出力文字数を $$L$$ として時間は $$O(L)$$、出力バッファ以外の追加空間は $$O(1)$$ です。`read` 自身の処理時間・空間は別途必要です。
