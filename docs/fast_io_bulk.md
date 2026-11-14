---
title: "整数の一括入出力"
documentation_of: //components/fast_io_bulk.hpp
library_category: components/util
---

`uint32_t`・`int32_t` の整数をまとめて読み込みます。出力には [非負整数の一括出力](fast_io_output.md) を読み込みます。出力だけが必要なら、その部品を直接 include できます。

```cpp
#include "lib/components/fast_io_bulk.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
	FastInput in;
	int n = 0, m = 0;
	in.read(n);
	std::vector<std::uint32_t> a(n);
	read_unsigned(in, a);
	in.read(m);
	std::vector<std::array<std::uint32_t, 3>> rows;
	rows.reserve(m);
	read_unsigned_records<3>(in, m, [&](auto x, auto y, auto z) {
		rows.push_back({x, y, z});
	});
	FastOutput out;
	write_unsigned_records<3>(out, rows.size(), [&](std::size_t i) {
		return rows[i];
	});
	return out.flush() ? 0 : 1;
}
```

`read_unsigned_records<3>` は 3 個の整数を読んでから、利用者が渡した関数（コールバック）を `emit(x, y, z)` の形で呼びます。上の例では、最初に `n` 個の整数を `a` へ読み、その後の `m` 組を入力順に `rows` へ追加します。`Fields` は 1 以上のコンパイル時定数で、コールバックはその個数の `uint32_t` を受け取ります。

符号付き整数には `read_signed_records<Fields>` を使います。使い方は同じで、コールバックには `Fields` 個の `int32_t` を渡します。

`write_unsigned_records<3>` は各組を空白区切りで 1 行ずつ出力します。コールバックや出力時の条件は [非負整数の一括出力](fast_io_output.md) にまとめています。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `read_unsigned(in, out)` | `FastInput& in` から `span<uint32_t> out` の全要素を埋める。返り値は `void` | $$O(L)$$ |
| `read_unsigned_blocks(in, count, emit)` | `size_t count` 個を順に読み、`emit(const uint32_t* values, size_t n)` に分割して渡す。返り値は `void` | $$O(L)$$ + コールバックの処理時間 |
| `read_unsigned_records<Fields>(in, count, emit)` | `Fields` 個ずつ `size_t count` 組を読み、各組の整数を引数として `emit` を呼ぶ。返り値は `void` | $$O(L)$$ + コールバックの処理時間 |
| `read_signed_blocks(in, count, emit)` | `size_t count` 個を順に読み、`emit(const int32_t* values, size_t n)` に分割して渡す。返り値は `void` | $$O(L)$$ + コールバックの処理時間 |
| `read_signed_records<Fields>(in, count, emit)` | `Fields` 個ずつ `size_t count` 組の符号付き整数を読み、各組の整数を引数として `emit` を呼ぶ。返り値は `void` | $$O(L)$$ + コールバックの処理時間 |
| `write_unsigned_records<Fields>(out, count, read)` | `read(i)` が返す `Fields` 個の整数を 1 行ずつ `count` 組出力する。返り値は `void` | $$O(L)$$ + コールバックの処理時間 |

既定引数はありません。入力のコールバックには、入力された順序で値を渡します。同じ整数や組が複数あっても、そのまま扱います。

空白区切りの十進整数が、指定した個数以上存在することが前提です。符号なし入力は `0` から `4294967295`、符号付き入力は `-2147483648` から `2147483647` に対応します。負数には先頭に `-` を付けます。先頭の 0・連続する空白・タブ・CRLF（Windows 形式の改行）も使えます。途中で入力が終わる場合（EOF）・範囲外・先頭の `+`・数字以外を含む空白区切りの文字列は扱いません。符号なし入力に `-` は指定できません。個数が 0 なら入力を進めず、コールバックも呼びません。指定個数の後にある入力は、続けて `in.read(x)` などで読めます。

`read_unsigned_blocks`・`read_signed_blocks` の分割位置と各回の個数は実行環境で変わります。渡された領域はコールバックの実行中だけ有効です。コールバック内から同じ `in` を読み進めないでください。複数列の入力には、分割位置を気にせず使える `read_unsigned_records`・`read_signed_records` が便利です。

入力では `Fields * count` が `size_t` の範囲を超えると `length_error` を送出します。配列・コールバックが参照するデータの寿命は呼び出しが終わるまで保ってください。

読み書きする文字数を $$L$$ として時間は $$O(L)$$、一括入力用の追加空間は定数です。組単位の入出力にはさらに $$O(\mathrm{Fields})$$ の空間を使います。入力元のバッファ・mmap と出力先の空間は含みません。
