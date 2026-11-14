---
title: "Fast I/O"
documentation_of: //components/fast_io.hpp
library_category: components/util
---

```cpp
#include "lib/components/fast_io.hpp"
FastInput in;
FastOutput out;
int n;
in.read(n);
out.println(n);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `FastInput(file = stdin)` / `FastInput(s)` | `FILE*`／`string_view` から入力を準備 | $$O(1)$$ |
| `in.read(x)` | 整数・文字列・対応型を読み、成功時に true | 読み進めた文字数に比例 |
| `in.token()` | 次の空白区切りの文字列を `string_view` で返す。入力の終端（EOF）なら空 | 読み進めた文字数に比例 |
| `FastOutput(file = stdout)` | 出力先を指定 | $$O(1)$$ |
| `out.write(x)` | 文字・文字列・整数・対応型を区切りなしで出力 | 出力文字数に比例 |
| `out.println(x)` | 値に続けて改行を出力 | 出力文字数に比例 |
| `out.flush()` | バッファを書き出し、成功時に true。破棄時にも呼ばれる | バッファ内の文字数に比例 |

`FastInput(FILE* = stdin)`・`FastOutput(FILE* = stdout)` は標準の整数型・文字列に対応します。`read(x)` は EOF で `false` を返します。`write(x)` は値だけ、`println(x)` は末尾に改行も出力します。`write(' ')` などで区切りを入れてください。出力は破棄時または `flush()` で書き出します。

出力は `__int128_t`・`__uint128_t` にも対応します。

独自の型は `string_view` から構築できれば入力できます。出力には `decimal_capacity()` と `to_chars(char*)` を用意します。Boost の多倍長整数は演算結果を整数型の変数に受け、`out.write(x.str())` で出力できます。

`token()` は空白で区切られた文字列（トークン）の `string_view` を返し、次の読み込みまで有効です。`FastInput(string_view)` は文字列から読みます。入力元の寿命を保ち、同じ `FILE*` でほかの入力方法と混用しないでください。

多数の 32bit 整数を読む場合は、符号付き・符号なしの[一括入力](https://qandori.github.io/cp-library/components/fast_io_bulk.hpp.html)も使えます。整数列や、始点・終点・重みのような複数項目の組をまとめて処理します。

処理する文字数を $$n$$、最大トークン長を $$m$$ として時間は $$O(n)$$、バッファ空間は $$O(m)$$ です。通常のファイルでは mmap を使い、ファイル全体をメモリ上の連続した領域として参照します。
