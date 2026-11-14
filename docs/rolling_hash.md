---
title: "Rolling Hash"
documentation_of: //string/rolling_hash.hpp
library_category: string/pattern_matching
---

```cpp
#include "lib/string/rolling_hash.hpp"

RollingHash a("abracadabra"), b("abra");
auto hash = a.get(0, 4);
bool same = hash == b.get();
int length = a.lcp(0, b, 0);
auto joined = RollingHash::concat(a.get(0, 4), b.get(), b.size());
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `RollingHash()` / `RollingHash(s)` / `RollingHash(a)` | 空／文字列／整数配列から構築 | 空: $$O(1)$$、配列: $$O(n)$$ |
| `build(s)` / `build(a)` | 文字列／整数配列で再構築 | $$O(n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `get()` / `get(l,r)` | 全体／$$[l,r)$$ のハッシュ | $$O(1)$$ |
| `value()` / `value(l,r)` | 全体／$$[l,r)$$ を長さ付きの `Value` で取得 | $$O(1)$$ |
| `lcp(l,other,j)` | 接尾辞同士の最長共通接頭辞長（LCP） | $$O(\log L)$$ |
| `lcp(l,r,other,j,k)` | $$[l,r)$$ と相手の $$[j,k)$$ の最長共通接頭辞長 | $$O(\log L)$$ |
| `RollingHash::concat(left,right,right_length)` | 生のハッシュ同士を連結 | 準備済み: $$O(1)$$、累乗の拡張: 合計 $$O(N)$$ |
| `RollingHash::reserve(n)` | 基数の累乗を長さ `n` まで準備 | 準備済み: $$O(1)$$、拡張1回の最悪: $$O(N)$$ |
| `RollingHash::single(x)` / `RollingHash::e()` | 1 要素／空列の `Value` | $$O(1)$$ |
| `RollingHash::op(a,b)` | `Value` をこの順で連結 | 準備済み: $$O(1)$$、累乗の拡張: 合計 $$O(N)$$ |
| `RollingHash::base()` / `RollingHash::mod` | 共通の基数／法 | $$O(1)$$ |
| `a == b` / `a != b` | `Value` のハッシュと長さを比較 | $$O(1)$$ |

`get(l,r)` は $$[l,r)$$、`get()` は全体のハッシュです。`lcp(l,b,j)` は両接尾辞の LCP、`lcp(l,r,b,j,k)` は区間 $$[l,r)$$ と $$[j,k)$$ の LCP を返します。`concat(x,y,len)` は `x` の後ろに長さ `len` の `y` を連結します。

`std::string_view`・整数の `std::vector<T>` から構築でき、`build(a)` で再構築できます。整数は法で割った余りを使い、文字列の `char` は符号なしバイトとして扱います。`size()`・`empty()` も使えます。

入力の長さは `INT_MAX` 以下、整数の要素型は64 bit以下の組み込み整数型とします。区間には `0 <= l <= r <= size()` を満たす添字を渡し、LCP の開始位置には各列の末尾も指定できます。空区間のハッシュは 0、長さ付きの値は `e()` と等しく、空区間を含む LCP は 0 です。`reserve(n)` の `n` は非負にします。`concat` のハッシュ値は `mod` 未満、`right_length` は非負、`op` の長さは非負で合計が `INT_MAX` 以下とします。

法は $$2^{61}-1$$、`base()` は実行ごとにランダムな共通の基数です。同じ長さの区間同士でハッシュを比較します。異なる列が同じハッシュ値になること（ハッシュの衝突）があるため、比較で誤判定する可能性があります。

```cpp
#include "lib/string/rolling_hash.hpp"
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

`Value` はハッシュ値 `hash` と要素数 `length` をまとめた型です。`single(x)` は 1 要素、`op(a,b)` はこの順での連結、`e()` は空列です。`value(l,r)`・`value()` は区間・全体を `Value` として取り出します。`Value` の比較は長さも含みます。`reserve(n)` で累乗を長さ `n` まで準備できます。

長さを $$n$$、LCP の比較区間の短い方の長さを $$L$$ として、構築は $$O(n)$$、取得・`single`・`e` は $$O(1)$$、LCP は $$O(\log L)$$ 時間です。基数の累乗は全インスタンスで共有し、これまでに準備を要求した最大長を $$N$$ とすると、一連の拡張は合計 $$O(N)$$ 時間・空間です。配列の再確保で既存の値を移すことがあるため、拡張1回の最悪時間は $$O(N)$$ です。連結・`op` は必要な累乗があれば $$O(1)$$、各インスタンスの空間は $$O(n)$$ です。空区間の LCP は $$O(1)$$ 時間で、それ以外の取得・LCP の追加空間は $$O(1)$$ です。Segtree で `reserve(n)` を済ませた場合、構築は $$O(n)$$、点更新・区間取得は $$O(\log n)$$、空間は $$O(n)$$ です。
