---
title: "Fast I/O"
documentation_of: //components/fast_io.hpp
---

```cpp
#include "lib/components/fast_io.hpp"
FastInput in;
FastOutput out;
int n;
in.read(n);
out.println(n);
```

| 操作 | 内容・返り値 |
| --- | --- |
| `FastInput(file = stdin)` / `FastInput(s)` | `FILE*`／`string_view` から入力を準備 |
| `in.read(x)` | 整数・文字列・対応型を読み、成功時に true |
| `in.token()` | 次の空白区切りトークンの `string_view`。EOF なら空 |
| `FastOutput(file = stdout)` | 出力先を指定 |
| `out.write(x)` | 文字・文字列・整数・対応型を区切りなしで出力 |
| `out.println(x)` | 値に続けて改行を出力 |
| `out.flush()` | バッファを書き出し、成功時に true。破棄時にも呼ばれる |

`FastInput(FILE* = stdin)`・`FastOutput(FILE* = stdout)` は標準の整数型・文字列に対応します。`read(x)` は EOF で `false` を返します。`write(x)` は値だけ、`println(x)` は末尾に改行も出力します。`write(' ')` などで区切りを入れてください。出力は破棄時または `flush()` で書き出します。

出力は `__int128_t`・`__uint128_t` にも対応します。

独自の型は `string_view` から構築できれば入力できます。出力には `decimal_capacity()` と `to_chars(char*)` を用意します。Boost の多倍長整数は演算結果を整数型の変数に受け、`out.write(x.str())` で出力できます。

`token()` は空白区切りの `string_view` を返し、次の読み込みまで有効です。`FastInput(string_view)` は文字列から読みます。入力元の寿命を保ち、同じ `FILE*` でほかの入力方法と混用しないでください。通常のファイルは mmap、パイプなどは fread を使います。

32 bit の符号なし整数は、十分な入力が残る場合に 8 桁をまとめて解析します。範囲と区切りを確認し、バッファ境界やほかの形式は通常の解析へ戻します。出力では 4 桁の表を使います。

処理する文字数を $$n$$、最大トークン長を $$m$$ として時間は $$O(n)$$、バッファ空間は $$O(m)$$ です。mmap では入力ファイル全体を仮想アドレス空間にマップします。
