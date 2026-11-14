---
title: "非負整数の一括出力"
documentation_of: //components/fast_io_output.hpp
library_category: components/util
---

`uint32_t` の整数を、決まった個数ずつまとめて出力します。[Fast I/O](fast_io.md) の `FastOutput` を使い、Mac を含むすべての対応環境で利用できます。出力する組は、利用者が渡した関数（コールバック）で指定します。入力処理も必要なら [一括入出力](fast_io_bulk.md) を include すると、この部品も読み込まれます。

```cpp
#include "lib/components/fast_io_output.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

int main() {
	const std::array<std::uint32_t, 2> rows[] = {
		{0, 7}, {12, 34}, {4294967295u, 0}
	};
	FastOutput out;
	write_unsigned_records<2>(out, 3, [&](std::size_t i) {
		return rows[i];
	});
	return out.flush() ? 0 : 1;
}
```

出力は次のとおりです。

```text
0 7
12 34
4294967295 0
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `write_unsigned_records<Fields>(out,count,read)` | `FastOutput& out` へ、`read(i)` が返す `Fields` 個の整数を `size_t count` 組出力する。返り値は `void` | $$O(L)$$ ＋コールバックの処理時間 |

引数の順序は出力先・組数・コールバックです。既定引数はありません。`Fields` はコンパイル時の定数で、`1 <= Fields <= numeric_limits<size_t>::max()/11` とします。

`read(i)` は `i=0,1,...,count-1` の順に 1 回ずつ呼ばれます。`std::array<uint32_t, Fields>` や `span` など、添字 `0` 以上 `Fields` 未満を参照できる値を返してください。各要素は `uint32_t` に変換でき、値が `0` 以上 `4294967295` 以下であることを前提とします。参照するデータは、その組の出力が終わるまで有効にしてください。

各組を十進整数の空白区切りで出力し、組の末尾に改行を付けます。値の順序と重複はそのまま保ちます。`count=0` は何も出力せず、`read` も呼びません。`read` 内から同じ `out` を使わないでください。

通常の `out.write`・`out.println` と混在できます。出力はバッファに残る場合があるため、最後に `out.flush()` を呼ぶか、`out` を破棄します。バッファを空ける処理で書き込みに失敗すると `std::runtime_error` を送出します。最後の `flush()` は成功時に `true` を返します。同じ `FastOutput` を複数スレッドから同時に使わないでください。

出力文字数を $$L$$ として時間は $$O(L)$$、追加空間は一組の戻り値と出力バッファを含めて $$O(\mathrm{Fields})$$ です。コールバック自身の処理時間・空間は別途必要です。
