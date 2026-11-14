---
title: 一般化 CRT
documentation_of: //math/crt.hpp
library_category: math/number_theory
redirect_from:
- /math/util/crt.hpp.html
- /lightweight/math/util/crt.hpp.html
---

連立合同式 $$x\equiv r_i\pmod {m_i}$$ を解きます。法が互いに素でなくても使え、剰余は負でも構いません。

```cpp
#include "lib/math/crt.hpp"
#include <cassert>

const auto a = crt(2, 6, 5, 9);
assert(a && a->residue == 14 && a->modulus == 18);

const auto b = crt({2, 5, 14}, {6, 9, 18});
assert(b == a);
assert(!crt(1, 2, 0, 4));
```

以下では合同式の数を $$n$$、入力する法の最大値を $$M$$ とします。

| 操作・型 | 引数・返り値 | 計算量 |
| --- | --- | --- |
| `crt(r0, m0, r1, m1)` | `std::int64_t` の 2 組の剰余・法を渡す。`std::optional<CRTSolution>` を返す。`constexpr` 対応 | $$O(\log\min(m_0,m_1))$$ |
| `crt(residues, moduli)` | `const std::vector<std::int64_t>&` の剰余列と法の列を渡す。同じ添字の組を合同式とし、`std::optional<CRTSolution>` を返す | $$O(n\log M)$$ |
| `solution.residue` | 最小非負解。型は `std::int64_t` | $$O(1)$$ |
| `solution.modulus` | 全解の周期で、全ての法の最小公倍数。型は `std::int64_t` | $$O(1)$$ |

各法は $$1\le m_i\le\mathrm{INT64\_MAX}$$、剰余は符号付き 64 bit 整数の全範囲で指定できます。解がなければ `std::nullopt` を返します。解があれば、全ての整数解は

$$x=\mathrm{residue}+k\,\mathrm{modulus}\quad(k\in\mathbb{Z}),\qquad 0\le\mathrm{residue}<\mathrm{modulus}$$

です。空の連立式は `{0, 1}`、すなわち全ての整数を表します。同じ合同式を複数指定しても構いません。

配列長の不一致と正でない法は `std::invalid_argument` を送出します。合同式を入力順に併合する途中で周期が `INT64_MAX` を超えた場合は `std::overflow_error` を送出します。この場合、それ以降の合同式は処理しません。途中の積には 128 bit 整数を使います。

[一次合同式](linear_congruence.md) を使って 2 つずつ併合します。合同式の数を $$n$$、最大の法を $$M$$ として、時間は $$O(n\log M)$$、追加空間は $$O(1)$$ です。2 式版は $$O(\log\min(m_0,m_1))$$ 時間です。法 1 の処理は $$O(1)$$ 時間です。
