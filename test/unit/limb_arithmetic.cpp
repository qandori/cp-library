#include "../../components/limb_arithmetic.hpp"
#include <array>
#include <cassert>
#include <random>
#include <vector>

template <std::uint64_t base>
void check() {
	using U = std::uint64_t;
	std::mt19937_64 rng(1234);
	for (int n = 0; n <= 257; ++n) for (int trial = 0; trial < 24; ++trial) {
		std::vector<U> a(n), b(n);
		for (int i = 0; i < n; ++i) {
			a[i] = trial < 8 ? (rng() & 1) * (base - 1) : rng() % base;
			b[i] = trial < 8 ? (trial & 1 ? a[i] : base - 1 - a[i]) : rng() % base;
		}
		for (U initial = 0; initial < 2; ++initial) for (bool subtract : {false, true}) {
			auto x = a, expected = a;
			U carry = initial;
			for (int i = 0; i < n; ++i) {
				if (subtract) {
					U value = b[i] + carry;
					carry = expected[i] < value;
					expected[i] = expected[i] + carry * base - value;
				} else {
					U value = expected[i] + b[i] + carry;
					carry = value >= base;
					expected[i] = value - carry * base;
				}
			}
			U actual = subtract ? limb_arithmetic::subtract<base>(x.data(), b.data(), n, initial)
							   : limb_arithmetic::add<base>(x.data(), b.data(), n, initial);
			assert(actual == carry && x == expected);
		}
		auto x = a, y = a;
		U c = limb_arithmetic::add<base>(x.data(), x.data(), n);
		U d = limb_arithmetic::add<base>(y.data(), a.data(), n);
		assert(x == y && c == d);
		assert(limb_arithmetic::subtract<base>(x.data(), x.data(), n) == 0);
		for (auto v : x) assert(v == 0);
	}
}

int main() {
	for (unsigned pattern = 0; pattern < 6561; ++pattern) for (unsigned carry = 0; carry < 2; ++carry) {
		unsigned g = 0, p = 0, x = pattern, expected = carry;
		for (int i = 0; i < 8; ++i, x /= 3) {
			unsigned state = x % 3;
			g |= (state == 2) << i;
			p |= (state == 1) << i;
			expected |= ((state == 2) || ((state == 1) && ((expected >> i) & 1))) << (i + 1);
		}
		assert(((p + (g << 1) + carry) ^ p) == expected);
	}
	check<2>();
	check<10000>();
	check<1000000000000000000ULL>();
	check<(1ULL << 62)>();
}
