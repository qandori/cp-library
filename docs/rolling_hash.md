---
title: "Rolling Hash"
documentation_of: //string/rolling_hash.hpp
---

```cpp
#include "lib/string/rolling_hash.hpp"

RollingHash a("abracadabra"), b("abra");
auto hash = a.get(0, 4);
bool same = hash == b.get();
int length = a.lcp(0, b, 0);
auto joined = RollingHash::concat(a.get(0, 4), b.get(), b.size());
```

`get(l,r)` は $$[l,r)$$、`get()` は全体のハッシュです。`lcp(l,b,j)` は両接尾辞の LCP、`lcp(l,r,b,j,k)` は区間 $$[l,r)$$ と $$[j,k)$$ の LCP を返します。`concat(x,y,len)` は `x` の後ろに長さ `len` の `y` を連結します。

`std::string_view`・整数の `std::vector<T>` から構築でき、`build(a)` で再構築できます。整数は法で正規化し、文字列の `char` は符号なし byte として扱います。`size()`・`empty()` も使えます。

法は $$2^{61}-1$$、`base()` は実行ごとにランダムな共通の基数です。同じ長さの区間同士でハッシュを比較します。ハッシュの衝突が起こる可能性があります。

長さを $$n$$、LCP の比較区間の短い方の長さを $$k$$ として、構築は $$O(n)$$、取得は $$O(1)$$、LCP は $$O(\log k)$$ です。基数の累乗は全インスタンスで共有し、長さ $$N$$ まで合計 $$O(N)$$ 時間・空間で拡張します。連結は必要な累乗があれば $$O(1)$$、各インスタンスの空間は $$O(n)$$ です。
