#include "string/rolling_hash.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

int main() {
	using H = RollingHash;
	using Hash = H::Hash;
	std::vector<Hash> expected{1};
	auto check = [&](int n) {
		while (int(expected.size()) <= n) expected.push_back(Hash(__uint128_t(expected.back()) * H::base() % H::mod));
		H::reserve(n);
		for (int i = 0; i <= n; ++i) {
			assert(H::concat(1, 0, i) == expected[i]);
			assert(H::concat(H::mod - 1, H::mod - 1, i) == Hash((__uint128_t(H::mod - 1) * expected[i] + H::mod - 1) % H::mod));
		}
	};
	for (int n = 0; n <= 32; ++n) check(n);
	int n = 32;
	for (int pass = 0; pass < 8; ++pass)
		for (int growth : {0, 1, 2, 3, 4, 7, 14, 15, 16, 17, 18, 19, 20, 31, 32, 33, 63, 64, 65}) {
			n += growth;
			check(n);
			check(n / 2);
		}
	std::mt19937 random(71239);
	for (int length : {0, 1, 2, 3, 15, 16, 17, 63, 64, 65, n + 15, n + 31, n + 48, n + 65}) {
		std::string s(length, '\0');
		for (char &c : s) c = char(random() % 256);
		H hash(s);
		Hash whole = 0;
		for (char c : s) whole = Hash((__uint128_t(whole) * H::base() + static_cast<unsigned char>(c) + 1) % H::mod);
		assert(hash.get() == whole);
		for (int query = 0; query < 100; ++query) {
			int l = random() % (length + 1), r = random() % (length + 1);
			if (l > r) std::swap(l, r);
			Hash part = 0;
			for (int i = l; i < r; ++i) part = Hash((__uint128_t(part) * H::base() + static_cast<unsigned char>(s[i]) + 1) % H::mod);
			assert(hash.get(l, r) == part);
			int m = (l + r) / 2;
			assert(H::concat(hash.get(l, m), hash.get(m, r), r - m) == part);
		}
	}
}
