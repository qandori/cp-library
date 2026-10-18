#include "components/predecessor_set.hpp"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <set>
#include <string>

void require(bool ok, int line = __builtin_LINE()) { if (!ok) { std::fprintf(stderr, "line %d\n", line); std::abort(); } }

void check(const PredecessorSet &set, const std::vector<int> &expected) {
	require(set.size() == int(expected.size()) && set.empty() == expected.empty());
	require(set.to_vector() == expected);
	require(set.min() == (expected.empty() ? set.universe_size() : expected.front()));
	require(set.max() == (expected.empty() ? -1 : expected.back()));
	std::vector<int> actual;
	set.for_each([&, token = std::make_unique<int>(0)](int p) { ++*token; actual.push_back(p); });
	require(actual == expected);
	for (int p : {INT_MIN, -1, 0, set.universe_size() / 2, set.universe_size() - 1, set.universe_size(), INT_MAX}) {
		auto next = std::lower_bound(expected.begin(), expected.end(), p);
		auto prev = std::upper_bound(expected.begin(), expected.end(), p);
		require(set.next(p) == (next == expected.end() ? set.universe_size() : *next));
		require(set.prev(p) == (prev == expected.begin() ? -1 : *--prev));
	}
	auto words = set.words();
	require(words.size() == (std::size_t(set.universe_size()) + 63) / 64);
	if (set.universe_size() & 63) require(!(words.back() >> (set.universe_size() & 63)));
}

void test_bits(std::string_view bits) {
	PredecessorSet set(bits);
	std::vector<int> expected;
	for (int i = 0; i < int(bits.size()); ++i) {
		require(set.contains(i) == (bits[i] == '1'));
		if (bits[i] == '1') expected.push_back(i);
	}
	check(set, expected);
	PredecessorSet copied(set.universe_size(), set.words()); check(copied, expected);
	copied.assign(copied.universe_size(), copied.words()); check(copied, expected);
	copied.clear(); require(copied.empty());
	copied.assign(bits); check(copied, expected);
	std::vector<std::uint64_t> packed(bits.size() / 64);
	predecessor_set_detail::pack_scalar(bits.data(), packed.data(), packed.size());
	for (std::size_t i = 0; i < packed.size(); ++i) require(packed[i] == set.words()[i]);
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	if (__builtin_cpu_supports("avx2")) {
		predecessor_set_detail::pack_avx2(bits.data(), packed.data(), packed.size());
		for (std::size_t i = 0; i < packed.size(); ++i) require(packed[i] == set.words()[i]);
	}
#endif
}

int main() {
	for (int n = 0; n <= 10; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::string bits(n, '0');
		for (int i = 0; i < n; ++i) if (mask >> i & 1) bits[i] = '1';
		test_bits(bits);
	}
	for (int p = 0; p < 256; ++p) {
		std::string source(256 + 32, '0'); source[p + 1] = '1';
		test_bits(std::string_view(source).substr(1, 256));
	}
	std::mt19937_64 rng(403023);
	for (int n : {0, 1, 7, 8, 31, 32, 63, 64, 65, 127, 128, 129, 191, 192, 193, 255, 256, 257, 511, 512, 513, 4095, 4096, 4097, 262143, 262144, 262145}) {
		for (int pattern = 0; pattern < 4; ++pattern) {
			std::string bits(n + 32, 'x');
			for (int i = 0; i < n; ++i) bits[i + 1] = (pattern == 0 ? false : pattern == 1 ? true : pattern == 2 ? rng() % 2 : rng() % 100 == 0) ? '1' : '0';
			test_bits(std::string_view(bits).substr(1, n));
		}
	}
	PredecessorSet set(4097);
	std::set<int> oracle;
	for (int i = 0; i < 20000; ++i) {
		int p = rng() % set.universe_size();
		bool present = rng() & 1;
		bool changed = present ? oracle.insert(p).second : bool(oracle.erase(p));
		require(set.set(p, present) == changed);
		int l = rng() % 4098, r = rng() % 4098;
		if (l > r) std::swap(l, r);
		std::vector<int> actual, expected(oracle.lower_bound(l), oracle.lower_bound(r));
		set.for_each(l, r, [&](int x) { actual.push_back(x); });
		require(actual == expected);
		if (i % 211 == 0) {
			check(set, {oracle.begin(), oracle.end()});
			PredecessorSet copy = set, moved(std::move(copy));
			require(copy.empty() && !copy.universe_size() && copy.words().empty());
			copy = std::move(moved); require(moved.empty());
			check(copy, {oracle.begin(), oracle.end()});
			copy.swap(set); check(set, {oracle.begin(), oracle.end()});
		}
	}
	set.fill(); require(set.size() == 4097 && set.min() == 0 && set.max() == 4096);
	require(!set.set(4096, true) && set.set(4096, false) && set.max() == 4095);
	PredecessorSet mask(65, std::vector<std::uint64_t>{UINT64_MAX, UINT64_MAX});
	require(mask.size() == 65 && mask.words().back() == 1);
	PredecessorSet alias(512, std::vector<std::uint64_t>(8, 0x3030303030303030ULL));
	alias.assign(std::string_view(reinterpret_cast<const char *>(alias.words().data()), 64));
	require(alias.universe_size() == 64 && alias.empty());
	PredecessorSet deep((1 << 24) + 1);
	for (int p : {0, 63, 64, 4095, 4096, 262143, 262144, (1 << 24) - 1, (1 << 24)}) require(deep.insert(p));
	check(deep, {0, 63, 64, 4095, 4096, 262143, 262144, (1 << 24) - 1, (1 << 24)});
	PredecessorSet largest(INT_MAX, true);
	require(largest.size() == INT_MAX && largest.next(INT_MAX - 1) == INT_MAX - 1 && largest.max() == INT_MAX - 1);
	require(largest.erase(INT_MAX - 1) && largest.prev(INT_MAX) == INT_MAX - 2);
	std::vector<int> tail;
	largest.for_each(INT_MAX - 65, INT_MAX, [&](int p) { tail.push_back(p); });
	require(tail.size() == 64 && tail.front() == INT_MAX - 65 && tail.back() == INT_MAX - 2);
	largest.clear(); require(largest.empty() && largest.next(INT_MIN) == INT_MAX && largest.prev(INT_MAX) == -1);
}
