---
title: "有理数（Fraction）"
documentation_of: //math/fraction.hpp
library_category: math
---

```cpp
#include "lib/math/fraction.hpp"
#include <cassert>

int main() {
	using F = Fraction<>;
	F a(2, -6), b(5, 6);
	auto sum = a + b;
	auto n = sum.numerator();
	auto d = sum.denominator();
	auto lower = a.floor();
	auto upper = a.ceil();
	auto inverse = a.inv();
	auto power = a.pow(3);
	double approximation = static_cast<double>(a);
	assert(n == 1 && d == 2);
	assert(lower == -1 && upper == 0);
	assert(inverse == F(-3));
	assert(power == F(-1, 27));
	assert(-1 < approximation && approximation < 0);
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Fraction<>()` / `Fraction<>(n)` / `Fraction<>(n,d)` | 0／整数／分数から構築。`d != 0` | $$O(\log M)$$ |
| `a.numerator()` / `a.denominator()` | 約分後の分子／正の分母 | $$O(1)$$ |
| `+a` / `-a` / `abs(a)` | 単項符号／絶対値 | $$O(1)$$ |
| `a + b` / `a - b` / `a * b` / `a / b` | 四則演算。整数を左右どちらにも置ける。除数は非零 | $$O(\log M)$$ |
| `a += b` / `a -= b` / `a *= b` / `a /= b` | 四則演算の結果を代入 | $$O(\log M)$$ |
| `++a` / `a++` / `--a` / `a--` | 1 を加算／減算 | $$O(\log M)$$ |
| `a == b` / `a != b` / `a < b` / `a <= b` / `a > b` / `a >= b` | 正確な比較 | $$O(1)$$ |
| `a.floor()` / `floor(a)` | 数学的な切り下げ | $$O(1)$$ |
| `a.ceil()` / `ceil(a)` | 数学的な切り上げ | $$O(1)$$ |
| `a.inv()` | 逆数。`a != 0` | $$O(1)$$ |
| `a.pow(k)` / `pow(a,k)` | 非負整数乗。0 乗は 1 | $$O(\log k\log M)$$ |
| `static_cast<bool>(a)` | 0 でないか | $$O(1)$$ |
| `static_cast<Int>(a)` | 0 方向への切り捨て | $$O(1)$$ |
| `static_cast<F>(a)` | 浮動小数点型 `F` への変換 | $$O(1)$$ |
| `in >> a` / `out << a` | 整数または分数の入力／`分子/分母` の出力 | 文字数に比例、入力の約分は別途 $$O(\log M)$$ |

`Fraction<Int = std::int64_t>` は有限の分数です。`Int` は64 bit以下の標準の符号付き整数型とします。
`Fraction<>` の別名は `fraction`。常に約分し、分母は正、0 は `0/1` で保持します。
コンストラクタの引数は `Int` 型です。整数1個からの変換もできます。

四則演算・複合代入・全比較・単項符号・`++`・`--` に対応します。整数を左右どちらにも置けます。
`numerator()`・`denominator()` で分子・分母を取得します。
`floor()`・`ceil()` は数学的な切り下げ・切り上げ、`static_cast<Int>(a)` は0方向への切り捨てです。
`float`・`double`・`long double` には明示的に変換できます。
`abs(a)`・`floor(a)`・`ceil(a)`・`pow(a, k)` も使えます。`k` は `std::uint64_t` で、0乗は1です。
`inv()` は約分済みの分子と分母を入れ替え、符号だけを整えます。

分母0、0による除算・逆数は不可です。**約分後の分子と正の分母が `Int` に収まること**を必要とします。
比較と四則演算の途中の積には幅の広い整数型を使うため、中間値が `Int` を超えても、約分後に収まる結果を扱えます。
`INT64_MIN` を分子に持つ値も扱えますが、その符号反転・絶対値や、正の分母が表現範囲を超える逆数は不可です。

ストリーム出力は常に `分子/分母`、入力は `a/b` または整数です。分子と `/` の間に空白は置きません。
入力の整数は `Int` に収まる必要があり、分母0・表現不能な値・書式エラーでは `failbit` を立て、読み込み先を変更しません。

型の最大絶対値を $$M$$ として、構築と四則演算は $$O(\log M)$$、比較・値取得・丸め・コピー・単項符号・絶対値・`inv()` は $$O(1)$$、`pow(k)` は $$O(\log k)$$ 時間です。指数 0 は定数時間です。追加空間は $$O(1)$$ です。
