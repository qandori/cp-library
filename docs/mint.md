---
title: "modint998244353"
documentation_of: //math/mint.hpp
library_category: math
---

整数を 998244353 で割った余りとして保持し、その法で四則演算する型です。`val()` は 0 以上 998244353 未満の整数を返します。

```cpp
#include "lib/math/mint.hpp"
#include <cassert>

int main() {
	mint x = 3;
	auto power = x.pow(5);
	auto inverse = x.inv();
	auto value = x.val();
	assert(power == mint(243));
	assert(inverse * x == mint(1));
	assert(value == 3);
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `mint()` / `mint(x)` | 0／整数を 998244353 で割った余りから構築 | $$O(1)$$ |
| `mint::mod()` / `a.val()` | 法／0 以上法未満の値 | $$O(1)$$ |
| `mint::raw(x)` | 法未満の非負整数から構築 | $$O(1)$$ |
| `+a` / `-a` | 単項符号 | $$O(1)$$ |
| `a + b` / `a - b` / `a * b` / `a / b` | 四則演算。整数との演算も利用可。除数は非零 | $$O(1)$$ |
| `a += b` / `a -= b` / `a *= b` / `a /= b` | 四則演算の結果を代入 | $$O(1)$$ |
| `++a` / `a++` / `--a` / `a--` | 1 を加算／減算 | $$O(1)$$ |
| `a == b` / `a != b` | 等値／非等値 | $$O(1)$$ |
| `a.pow(k)` / `a.inv()` | 非負整数乗／逆数。逆数は非零のみ | pow: $$O(\log k)$$、inv: $$O(1)$$（法固定） |
| `in >> a` / `out << a` | 整数の入力／0 以上法未満の整数の出力 | 入出力文字数に比例 |
| `mint::word(x)` / `mint::reduce(x)` | 内部表現からの構築／内部表現に使う剰余計算。通常は `mint(x)`・`val()` を使う | $$O(1)$$ |

`pow(n)` の指数は非負、`inv()` と除算の除数は非零とします。

法 998244353 の整数として四則演算します。`mint(-1).val()` は 998244352、`mint(998244354).val()` は 1 です。計算中は `mint` のまま扱い、通常の整数が必要になったときに `val()` を呼びます。`std::cout << x` でも 0 以上法未満の整数を出力できます。

FPS に依存せず、このヘッダーだけで利用できます。

`pow(k)` は $$O(\log k)$$、その他の演算は mod 固定のため $$O(1)$$ 時間です。追加空間は $$O(1)$$ です。
