#include "string/lcs.hpp"
#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

#if defined(LCS_EMULATE_SIMD)
constexpr bool have_avx2 = true, have_avx512 = true;
#elif defined(__x86_64__)
const bool have_avx2 = __builtin_cpu_supports("avx2"), have_avx512 = __builtin_cpu_supports("avx512f");
#endif

int brute(std::string_view a, std::string_view b) {
	std::vector<int> dp(b.size() + 1);
	for (char x : a) {
		int previous = 0;
		for (std::size_t j = 0; j < b.size(); ++j) {
			int old = dp[j + 1];
			dp[j + 1] = x == b[j] ? previous + 1 : std::max(dp[j], old);
			previous = old;
		}
	}
	return dp.back();
}

bool subsequence(std::string_view a, std::string_view b) {
	std::size_t j = 0;
	for (char c : b)
		if (j < a.size() && a[j] == c) ++j;
	return j == a.size();
}

void check(const std::string &s, const std::string &t, int expected = -1) {
	if (expected == -1) expected = brute(s, t);
	assert(lcs_length(s, t) == expected);
	assert(lcs_length(t, s) == expected);
	for (auto result : {lcs(s, t), lcs(t, s)}) {
		assert(int(result.size()) == expected);
		assert(subsequence(result, s));
		assert(subsequence(result, t));
	}
#if defined(__x86_64__) || defined(LCS_EMULATE_SIMD)
	if (std::max(s.size(), t.size()) <= 4000 && std::max(s.size(), t.size()) > 64) {
		auto forward = lcs_detail::profile<false, 1>(s, t), backward = lcs_detail::profile<true, 1>(s, t);
		assert(lcs_detail::count(forward) == expected && lcs_detail::count(backward) == expected);
		if (have_avx2) {
			assert((lcs_detail::profile<false, 2>(s, t) == forward));
			assert((lcs_detail::profile<true, 2>(s, t) == backward));
		}
		if (have_avx512) {
			assert((lcs_detail::profile<false, 3>(s, t) == forward));
			assert((lcs_detail::profile<true, 3>(s, t) == backward));
		}
		if ((s.size() + t.size()) % 31 == 0 || std::min(s.size(), t.size()) >= 511) {
			for (int backend : {2, 3}) {
				if (backend == 2 && !have_avx2) continue;
				if (backend == 3 && !have_avx512) continue;
				std::string result;
				if (backend == 2) lcs_detail::restore<2>(s, t, result);
				else lcs_detail::restore<3>(s, t, result);
				assert(int(result.size()) == expected && subsequence(result, s) && subsequence(result, t));
			}
		}
	}
#endif
}

void check_carries() {
#if defined(__x86_64__) || defined(LCS_EMULATE_SIMD)
	using U = std::uint64_t;
	std::mt19937_64 rng(6481);
	for (int code = 0; code < 6561; ++code) {
		alignas(64) std::array<U, 24> row, eq;
		int pattern = code;
		for (int j = 0; j < 8; ++j) {
			int mode = pattern % 3;
			pattern /= 3;
			row[j] = mode ? ~U(0) : 0;
			eq[j] = mode == 2 ? ~U(0) : 0;
			row[j + 8] = ~U(0);
			eq[j + 8] = 0;
			row[j + 16] = j ? rng() : 0;
			eq[j + 16] = j ? rng() : 0;
		}
		auto expected = row;
		U carry = 0;
		for (int j = 0; j < 24; ++j) {
			U v = row[j], u = v & eq[j];
			__uint128_t sum = __uint128_t(v) + u + carry;
			expected[j] = U(sum) | (v ^ u);
			carry = U(sum >> 64);
		}
		std::array<const U *, 256> masks;
		masks.fill(eq.data());
		for (int backend : {2, 3}) {
			if (backend == 2 && !have_avx2) continue;
			if (backend == 3 && !have_avx512) continue;
			alignas(64) auto actual = row;
			if (backend == 2) lcs_detail::avx2<false>("x", masks, actual.data(), 24);
			else lcs_detail::avx512<false>("x", masks, actual.data(), 24);
			assert(actual == expected);
		}
	}
#endif
}

int main() {
	check_carries();
	check("", "");
	check("abc", "");
	check("", "abc");
	check("abcde", "ace", 3);
	check("ABCBDAB", "BDCABA", 4);
	check("abc", "def", 0);
	check(std::string("\0\xff\x80\0", 4), std::string("\xff\0\x80", 3));
	std::vector<std::string> binary;
	for (int n = 0; n <= 7; ++n)
		for (int mask = 0; mask < (1 << n); ++mask) {
			std::string s(n, 'a');
			for (int j = 0; j < n; ++j) s[j] += (mask >> j) & 1;
			binary.push_back(s);
		}
	for (const auto &s : binary)
		for (const auto &t : binary) check(s, t);
	std::mt19937 rng(942765);
	auto random_string = [&](int n, int alphabet) {
		std::string s(n, 0);
		for (char &c : s) c = char(rng() % alphabet);
		return s;
	};
	for (int repeat = 0; repeat < 1200; ++repeat) {
		int alphabet = 1 + rng() % 256;
		check(random_string(rng() % 300, alphabet), random_string(rng() % 300, alphabet));
	}
	for (int n : {1, 2, 7, 31, 32, 63, 64, 65, 127, 128, 129, 191, 192, 193, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025})
		for (int alphabet : {2, 4, 26, 256}) {
			check(random_string(n, alphabet), random_string(n + 3, alphabet));
			check(random_string(n, alphabet), random_string(10000, alphabet));
			check(std::string(n, 'a') + std::string(n, 'b'), std::string(n, 'b') + std::string(n, 'a'), n);
		}
	for (int repeat = 0; repeat < 50; ++repeat) {
		auto s = random_string(500, 256), t = random_string(600, 256);
		std::string prefix = random_string(31, 256), suffix = random_string(73, 256);
		check(prefix + s + suffix, prefix + t + suffix, 104 + brute(s, t));
	}
	check(std::string(100000, 'a'), std::string(100000, 'a'), 100000);
	check(std::string(100000, 'a'), std::string(100000, 'b'), 0);
	for (int m : {1, 2, 31, 63, 64, 65, 127, 128, 129}) check(std::string(100000, 'a') + 'b', 'b' + std::string(m, 'a'), m);
	std::string low(10000, 0), high(10000, 0);
	for (int i = 0; i < 10000; ++i) low[i] = char(i % 128), high[i] = char(128 + i % 128);
	check(low, high, 0);
	check(std::string(50000, 'a') + std::string(50000, 'b'), std::string(50000, 'b') + std::string(50000, 'a'), 50000);
	for (int alphabet : {2, 26, 256}) {
		int n = alphabet == 26 ? 200000 : 100000;
		auto s = random_string(n, alphabet), t = random_string(n, alphabet);
		auto scalar = lcs_detail::profile<false, 1>(s, t);
		auto result = lcs(s, t);
		assert(int(result.size()) == lcs_length(s, t) && int(result.size()) == lcs_detail::count(scalar));
		assert(subsequence(result, s) && subsequence(result, t));
#if defined(__x86_64__) || defined(LCS_EMULATE_SIMD)
		if (have_avx2) assert((lcs_detail::profile<false, 2>(s, t) == scalar));
		if (have_avx512) assert((lcs_detail::profile<false, 3>(s, t) == scalar));
		if (alphabet == 26) {
			for (int backend : {2, 3}) {
				if (backend == 2 && !have_avx2) continue;
				if (backend == 3 && !have_avx512) continue;
				std::string restored;
				if (backend == 2) lcs_detail::restore<2>(s, t, restored);
				else lcs_detail::restore<3>(s, t, restored);
				assert(restored.size() == result.size() && subsequence(restored, s) && subsequence(restored, t));
			}
		}
#endif
	}
}
