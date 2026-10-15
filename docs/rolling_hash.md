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

```cpp
#include <atcoder/segtree>

using RH = RollingHash;
RH::reserve(11);
std::vector<RH::Value> values;
for (char c : std::string_view("abracadabra")) values.push_back(RH::single(c));
atcoder::segtree<RH::Value, RH::op, RH::e> seg(values);
seg.set(0, RH::single('x'));
auto part = seg.prod(7, 11);
bool same = part == RH("abra").value();
```

`Value` は `hash` と `length` を持つモノイドです。`single(x)` は 1 要素、`op(a,b)` はこの順での連結、`e()` は空列です。`value(l,r)`・`value()` は区間・全体を `Value` として取り出します。`Value` の比較は長さも含みます。`reserve(n)` で累乗を長さ `n` まで準備できます。

長さを $$n$$、LCP の比較区間の短い方の長さを $$k$$ として、構築は $$O(n)$$、取得・`single`・`e` は $$O(1)$$、LCP は $$O(\log k)$$ です。基数の累乗は全インスタンスで共有し、長さ $$N$$ まで合計 $$O(N)$$ 時間・空間で拡張します。連結・`op` は必要な累乗があれば $$O(1)$$、各インスタンスの空間は $$O(n)$$ です。Segtree で `reserve(n)` を済ませた場合、構築は $$O(n)$$、点更新・区間取得は $$O(\log n)$$、空間は $$O(n)$$ です。
