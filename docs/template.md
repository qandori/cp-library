# 共通テンプレート

```cpp
#include "lib/template.hpp"
```

| 記法 | 内容 |
| --- | --- |
| `rep(i, n)` / `rep(i, l, r)` | `[0, n)` / `[l, r)` の昇順ループ |
| `per(i, n)` / `per(i, l, r)` | 同じ区間の降順ループ |
| `rep(n)` / `per(n)` | 添字を使わずに繰り返す |
| `vec<T>` / `all(a)` / `sz(a)` | `vector<T>` / イテレータの組 / `int` 型の要素数 |
| `chmin(x, y)` / `chmax(x, y)` | 更新した場合に `true` を返す |
| `UNIQUE(a)` | ソートして重複除去 |

`vector<bool>` の要素に `^=`・`&=`・`|=` を使えます。`DEBUG` を定義しない場合、`cerr` の出力は無効です。
