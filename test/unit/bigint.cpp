#include "math/bigint.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using I = __int128_t;
std::string decimal(I value) {
	if (!value) return "0";
	bool negative = value < 0;
	__uint128_t x = value;
	if (negative) x = -x;
	std::string s;
	while (x) s += '0' + x % 10, x /= 10;
	if (negative) s += '-';
	std::reverse(s.begin(), s.end());
	return s;
}
std::string reference_product(const std::string &a, const std::string &b) {
	auto parse = [](const std::string &s) {
		std::vector<std::uint64_t> v;
		for (int end = s.size(); end > 0; end -= 4) v.push_back(std::stoul(s.substr(std::max(0, end - 4), std::min(4, end))));
		return v;
	};
	auto x = parse(a), y = parse(b);
	std::vector<std::uint64_t> z(x.size() + y.size());
	for (std::size_t i = 0; i < x.size(); ++i)
		for (std::size_t j = 0; j < y.size(); ++j) z[i + j] += x[i] * y[j];
	for (std::size_t i = 0; i + 1 < z.size(); ++i) z[i + 1] += z[i] / 10000, z[i] %= 10000;
	while (z.size() > 1 && !z.back()) z.pop_back();
	std::string result = std::to_string(z.back());
	for (std::size_t i = z.size() - 1; i--;) {
		auto part = std::to_string(z[i]);
		result += std::string(4 - part.size(), '0') + part;
	}
	return result;
}
std::string random_decimal(std::mt19937_64 &random, int length) {
	std::string s(length, '0');
	for (char &c : s) c += random() % 10;
	s[0] = '1' + random() % 9;
	return s;
}
int main() {
	for (auto s : {"0", "00", "+000", "-000"}) {
		BigInt zero(s);
		assert(zero == 0 && zero.sign() == 0 && !zero && zero.str() == "0");
	}
	for (I x : {I(0), I(-1), I(1), I(std::numeric_limits<std::int64_t>::min()), I(std::numeric_limits<std::uint64_t>::max()), -(I(1) << 126) * 2, (I(1) << 126) - 1}) {
		BigInt a(x);
		assert(a.str() == decimal(x));
		assert(BigInt(a.str()) == a && I(a) == x);
		std::stringstream stream;
		stream << a;
		BigInt b;
		stream >> b;
		assert(a == b);
	}
	for (I x : {I(-999999999999999999LL), I(-1), I(0), I(1), I(999999999999999999LL), I(1000000000000000000LL)})
		for (I y : {I(-999999999999999999LL), I(-1), I(0), I(1), I(999999999999999999LL), I(1000000000000000000LL)}) {
			BigInt a(x), b(y);
			assert((a + b).str() == decimal(x + y));
			assert((a - b).str() == decimal(x - y));
			a += a;
			assert(a.str() == decimal(2 * x));
			a -= a;
			assert(a == 0 && a.sign() == 0);
		}
	__uint128_t umax = ~__uint128_t(0);
	assert(__uint128_t(BigInt(umax)) == umax);
	std::mt19937_64 random(9187);
	for (int i = 0; i < 10000; ++i) {
		I x = I(random() % 1000000000000000ULL) - 500000000000000LL;
		I y = I(random() % 1000000000000000ULL) - 500000000000000LL;
		if (!y) y = 1;
		BigInt a(x), b(y);
		assert((a + b).str() == decimal(x + y));
		assert((a - b).str() == decimal(x - y));
		assert((a * b).str() == decimal(x * y));
		assert((a / b).str() == decimal(x / y));
		assert((a % b).str() == decimal(x % y));
		assert((a <=> b) == (x <=> y));
		assert(-a == BigInt(-x) && +a == a);
		assert(abs(a) == BigInt(x < 0 ? -x : x));
		a += a; assert(a == BigInt(2 * x));
		a -= a; assert(a == 0 && a.sign() == 0);
		a = b; a *= a; assert(a == BigInt(y * y));
		a = b; a /= a; assert(a == 1);
		a = b; a %= a; assert(a == 0);
		a = b; a *= std::int64_t(-73); assert(a == BigInt(-73 * y));
		a = b; assert(a++ == b && a == b + 1);
		assert(a-- == b + 1 && a == b);
		assert(++a == b + 1 && --a == b);
	}
	for (int length : {1, 17, 18, 19, 143, 144, 145, 287, 288, 289, 575, 576, 577, 1000, 2304, 2305, 4607, 4608, 4609, 5000, 10000}) {
		for (int repetition = 0; repetition < 4; ++repetition) {
			auto x = random_decimal(random, length), y = random_decimal(random, length - repetition % length);
			BigInt a(x), b(y), product(reference_product(x, y));
			assert(a * b == product);
			assert((-a) * b == -product && (-a) * (-b) == product);
			assert(a * a == BigInt(reference_product(x, x)));
			auto [q, r] = divmod(product, a);
			assert(q == b && r == 0);
			assert((product + a - 1) / a == b && (product + a - 1) % a == a - 1);
			assert((-(product + a - 1)) / a == -b && (-(product + a - 1)) % a == -(a - 1));
			assert(product / (-a) == -b && product % (-a) == 0);
			assert((a + b) - a == b && (a - b) + b == a);
		}
	}
	for (int n : {1, 17, 18, 19, 288, 4608, 4609, 100000, 203700, 203710, 210000}) {
		BigInt a(std::string(n, '9'));
		assert(a + 1 == BigInt("1" + std::string(n, '0')));
		assert(a * a == BigInt(std::string(n - 1, '9') + "8" + std::string(n - 1, '0') + "1"));
		BigInt b = a + 1;
		assert(--b == a && ++b == a + 1);
		assert((b - 1) / b == 0 && (b - 1) % b == a);
		assert((a * a) / a == a && (a * a) % a == 0);
	}
	for (auto [n, m] : {std::pair(10000, 300), {50000, 287}, {30000, 5000}, {20000, 1170}, {20000, 1200}}) {
		auto x = random_decimal(random, n), y = random_decimal(random, m);
		BigInt a(x), b(y), product(reference_product(x, y));
		assert(a * b == product);
		assert(divmod(product + 123, a) == std::pair(b, BigInt(123)));
		assert(divmod(product + 123, b) == std::pair(a, BigInt(123)));
	}
	for (int n : {257, 511, 1024}) for (int m : {8, 9, 16, 32, 33}) {
		auto x = random_decimal(random, 18 * n), y = random_decimal(random, 18 * m);
		BigInt a(x), b(y), product(reference_product(x, y));
		assert(a * b == product && b * a == product);
		assert((-a) * b == -product && a * (-b) == -product);
		x.assign(18 * n, '9'), y.assign(18 * m, '9');
		a = BigInt(x), b = BigInt(y), product = BigInt(reference_product(x, y));
		assert(a * b == product && b * a == product);
	}
	BigInt source(-123), moved(std::move(source));
	assert(source == 0 && moved == -123);
	source = std::move(moved);
	assert(source == -123 && moved == 0);
	BigInt *self = &source;
	source = std::move(*self);
	assert(source == -123);
	assert(pow(BigInt(12), 0) == 1);
	assert(pow(BigInt(-12), 13).str() == "-106993205379072");
	assert(pow(BigInt(10), 10000).str() == "1" + std::string(10000, '0'));
}
