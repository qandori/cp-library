#include "string/rolling_hash.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

RollingHash global_hash("global");

template <class T>
std::uint64_t symbol(T value) {
	__int128 x;
	if constexpr (std::is_same_v<T, char>) x = static_cast<unsigned char>(value);
	else x = value;
	return std::uint64_t((x % RollingHash::mod + RollingHash::mod + 1) % RollingHash::mod);
}

template <class Range>
std::uint64_t naive(const Range &a, int l, int r) {
	std::uint64_t result = 0;
	for (int i = l; i < r; ++i) result = std::uint64_t((__uint128_t(result) * RollingHash::base() + symbol(typename Range::value_type(a[i]))) % RollingHash::mod);
	return result;
}

template <class Range>
void check(const RollingHash &hash, const Range &a) {
	assert(hash.size() == int(a.size()) && hash.empty() == a.empty());
	assert(hash.get() == naive(a, 0, int(a.size())));
	for (int l = 0; l <= hash.size(); ++l)
		for (int r = l; r <= hash.size(); ++r) {
			assert(hash.get(l, r) < RollingHash::mod);
			assert(hash.get(l, r) == naive(a, l, r));
			int m = (l + r) / 2;
			assert(RollingHash::concat(hash.get(l, m), hash.get(m, r), r - m) == hash.get(l, r));
		}
}

template <class Range>
void check_lcp(const RollingHash &a, const Range &s, const RollingHash &b, const Range &t, std::mt19937_64 &random) {
	for (int i = 0; i < 300; ++i) {
		int l = random() % (s.size() + 1), r = random() % (s.size() + 1);
		int j = random() % (t.size() + 1), k = random() % (t.size() + 1);
		if (l > r) std::swap(l, r);
		if (j > k) std::swap(j, k);
		int expected = 0;
		while (l + expected < r && j + expected < k && s[l + expected] == t[j + expected]) ++expected;
		assert(a.lcp(l, r, b, j, k) == expected);
		expected = 0;
		while (l + expected < int(s.size()) && j + expected < int(t.size()) && s[l + expected] == t[j + expected]) ++expected;
		assert(a.lcp(l, b, j) == expected);
	}
}

template <class T>
void integer_test(std::mt19937_64 &random) {
	std::vector<T> a{std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), T(0), T(1), T(-1)};
	RollingHash hash(a);
	check(hash, a);
	for (int i = 0; i < 50; ++i) {
		a.resize(random() % 60);
		for (auto &x : a) x = T(random());
		hash.build(a);
		check(hash, a);
		auto b = a;
		if (!b.empty()) b[random() % b.size()] = T(random());
		RollingHash other(b);
		check_lcp(hash, a, other, b, random);
	}
}

int main() {
	std::mt19937_64 random(713);
	assert(RollingHash::base() >= 256 && RollingHash::base() < RollingHash::mod - 256);
	assert(global_hash.get() == RollingHash("global").get());
	RollingHash empty;
	check(empty, std::string{});
	assert(empty.lcp(0, empty, 0) == 0);
	assert(RollingHash::concat(0, 0, 0) == 0);
	for (int i = 0; i < 200; ++i) {
		std::string s(random() % 80, '\0');
		for (char &c : s) c = char(random() % 256);
		RollingHash hash(s);
		check(hash, s);
		auto t = s;
		if (!t.empty()) t[random() % t.size()] ^= char(1);
		RollingHash other(t);
		check_lcp(hash, s, other, t, random);
		check_lcp(hash, s, hash, s, random);
		std::vector<unsigned char> bytes(s.begin(), s.end());
		assert(hash.get() == RollingHash(bytes).get());
		auto copy = hash;
		check(copy, s);
		auto moved = std::move(copy);
		check(moved, s);
		check(copy, std::string{});
		copy.build(t);
		check(copy, t);
		empty.build(std::string_view(s).substr(s.size() / 2));
		check(empty, s.substr(s.size() / 2));
	}
	integer_test<std::int64_t>(random);
	integer_test<std::uint64_t>(random);
	integer_test<int>(random);
	integer_test<unsigned>(random);
	integer_test<signed char>(random);
	integer_test<unsigned char>(random);
	std::vector<bool> flags{true, false, false, true};
	check(RollingHash(flags), flags);
	std::int64_t p = std::int64_t(RollingHash::mod);
	std::vector<std::int64_t> edges{-p - 1, -p, -p + 1, -1, 0, 1, p - 1, p, p + 1, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()};
	check(RollingHash(edges), edges);
	RollingHash big(std::string(1000000, 'a'));
	assert(big.lcp(0, big, 1) == big.size() - 1);
	assert(big.lcp(123, 45678, big, 999999, 1000000) == 1);
	assert(big.get(0, 1000) == RollingHash(std::string(1000, 'a')).get());
	std::string long_prefix(200000, 'a'), changed = long_prefix;
	changed.back() = 'b';
	assert(RollingHash(long_prefix).lcp(0, RollingHash(changed), 0) == 199999);
	for (int i = 0; i < 5000; ++i) {
		std::uint64_t left = i < 4 ? RollingHash::mod - 1 - i : random() % RollingHash::mod;
		std::uint64_t right = i < 4 ? RollingHash::mod - 1 - i : random() % RollingHash::mod;
		int length = random() % 20000;
		std::uint64_t power = 1;
		for (std::uint64_t b = RollingHash::base(), e = length; e; e >>= 1, b = std::uint64_t(__uint128_t(b) * b % RollingHash::mod))
			if (e & 1) power = std::uint64_t(__uint128_t(power) * b % RollingHash::mod);
		assert(RollingHash::concat(left, right, length) == std::uint64_t((__uint128_t(left) * power + right) % RollingHash::mod));
	}
}
