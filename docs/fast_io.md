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

`FastInput(FILE* = stdin)`・`FastOutput(FILE* = stdout)` は標準の整数型・文字列・`BigInt` に対応します。`read(x)` は EOF で `false` を返します。`write(x)` は値だけ、`println(x)` は末尾に改行も出力します。`write(' ')` などで区切りを入れてください。出力は破棄時または `flush()` で書き出します。

`token()` は空白区切りの `string_view` を返し、次の読み込みまで有効です。`FastInput(string_view)` は文字列から読みます。入力元の寿命を保ち、同じ `FILE*` でほかの入力方法と混用しないでください。通常のファイルは mmap、パイプなどは fread を使います。

処理する文字数を $$n$$、最大トークン長を $$m$$ として時間は $$O(n)$$、バッファ空間は $$O(m)$$ です。mmap では入力ファイル全体を仮想アドレス空間にマップします。
