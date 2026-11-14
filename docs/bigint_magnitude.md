---
title: "多倍長整数の絶対値演算"
documentation_of: //components/bigint_magnitude.hpp
library_category: components/math
---

[BigInt](https://qandori.github.io/cp-library/math/bigint.hpp.html) の符号を持たない内部演算です。通常は `BigInt` を使用し、桁配列を直接扱う場合にこの部品を使います。

```cpp
#include "lib/components/bigint_magnitude.hpp"
#include <cassert>

using bigint_detail::Magnitude;
Magnitude a{999999999, 1}, b{2};
auto product = bigint_detail::multiply(a, b);
assert((product == Magnitude{999999998, 3}));
auto [q, r] = bigint_detail::divide(a, b);
assert((q == Magnitude{999999999}));
assert((r == Magnitude{1}));
assert((a == Magnitude{999999999, 1}));
```

`bigint_detail::Word` は `std::uint32_t`、`Magnitude` は `std::vector<Word>`、`base` は `1000000000` です。下位桁から並べ、各要素を `[0,base)` に収めます。零は空配列とし、非零では末尾の要素を非零にしてください。`trim` 以外の操作は、この正規化された表現を前提とします。

以下の関数は全て `bigint_detail` 名前空間にあります。引数に既定値はありません。

| 関数 | 内容・返り値 | 入力の更新 |
| --- | --- | --- |
| `trim(a)` | 末尾の零を除く。返り値は `void` | `a` |
| `compare(a,b)` | 大小に応じて `-1`・`0`・`1` を `int` で返す | なし |
| `multiply_word(a,b)` | 配列と 1 桁の積を `Magnitude` で返す。`0 <= b < base` | なし |
| `divide_word(a,b)` | 1 桁で除算し、余りを `Word` で返す。`0 < b < base` | `a` を商にする |
| `add(a,b)` | `a += b` とする。返り値は `void` | `a` |
| `subtract(a,b)` | `a >= b` として減算する。返り値は `void` | `a` |
| `increment(a)` | 1 増加させる。返り値は `void` | `a` |
| `multiply(a,b)` | 積を `Magnitude` で返す | なし |
| `divide_schoolbook(a,b)` | 通常の長除算で商・余りの `std::pair<Magnitude,Magnitude>` を返す。`b` は非零 | なし |
| `reciprocal(a)` | 下記の整数逆数を `Magnitude` で返す | なし |
| `divide(a,b)` | 商・余りの `std::pair<Magnitude,Magnitude>` を返す | なし |

`add`・`multiply`・`divide` は同じ配列を両方の引数に指定できます。`add(a,a)` は元の値の 2 倍にし、`subtract(a,a)` は空配列にします。`divide` で除数が空配列なら `std::domain_error` を送出します。`divide_schoolbook` の非零条件と、1 桁演算の基数条件などは `assert` の前提です。

`reciprocal(a)` は、`a` が非空で最上位桁が `base / 2` 以上であることを前提とします。桁数を $$n$$、配列が表す非負整数を $$A$$、基数を $$B=10^9$$ とすると、返す値は次です。

$$
\displaystyle\left\lfloor\frac{B^{2n}}{A}\right\rfloor.
$$

乗除算は誤差なく求めます。大きな入力の内部乗算には NTT（素数を法とする離散フーリエ変換）を使います。その乗算ごとに、繰り上げ前の係数列の長さが $$2^{24}$$ 以下である必要があります。これを超える場合は `std::length_error` を送出します。`reciprocal`・`divide` も内部乗算について同じ条件が必要です。

基数桁数を $$n,m$$、$$n\ge m$$、長さ $$k$$ の乗算時間を $$M(k)$$ とします。`trim`・`compare`・1 桁演算・`add`・`subtract`・`increment` は最悪 $$O(n)$$ 時間です。小さい `multiply` は $$O(nm)$$、NTT 版は $$O((n+m)\log(n+m))$$ 時間です。`divide_schoolbook` は被除数が除数以上の場合、商の桁数を $$q$$ として $$O(qm)$$ 時間で、$$q\le n-m+1$$ です。被除数が除数未満の除算は、余りのコピーを含め $$O(n)$$ 時間です。`reciprocal` は $$O(M(n))$$、逆数を使う `divide` は $$O(\displaystyle\frac{n}{m}M(m))$$ 時間です。乗除算の追加空間は $$O(n+m)$$、`reciprocal` は $$O(n)$$、`trim`・`compare`・`subtract` は $$O(1)$$ です。`add` は結果を保存する `a` の拡張分を除き、$$O(1)$$ 空間です。
