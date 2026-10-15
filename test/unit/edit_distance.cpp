#include "string/edit_distance.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#if defined(EDIT_DISTANCE_EMULATE_SIMD)
constexpr bool have_avx2 = true, have_avx512 = true;
#elif defined(__x86_64__)
const bool have_avx2 = __builtin_cpu_supports("avx2"), have_avx512 = __builtin_cpu_supports("avx512f");
#endif

int brute(std::string_view s, std::string_view t) {
	std::vector<int> row(t.size() + 1);
	std::iota(row.begin(), row.end(), 0);
	for (int i = 0; i < int(s.size()); ++i) {
		int diagonal = row[0]++;
		for (int j = 0; j < int(t.size()); ++j) {
			int previous = row[j + 1];
			row[j + 1] = std::min({previous + 1, row[j] + 1, diagonal + (s[i] != t[j])});
			diagonal = previous;
		}
	}
	return row.back();
}

void check(const std::string &s, const std::string &t, int expected = -1) {
	if (expected == -1) expected = brute(s, t);
	assert(edit_distance(s, t) == expected);
	assert(edit_distance(t, s) == expected);
	if (std::max(s.size(), t.size()) <= 4000) {
		assert(edit_distance_detail::bit_parallel(s, t) == expected);
		assert(edit_distance_detail::bit_parallel(t, s) == expected);
#if defined(__x86_64__) || defined(EDIT_DISTANCE_EMULATE_SIMD)
		if (have_avx2) assert(edit_distance_detail::bit_parallel<2>(s, t) == expected);
		if (have_avx512) assert(edit_distance_detail::bit_parallel<3>(s, t) == expected);
#endif
	}
	if (std::max(s.size(), t.size()) <= 2000) {
		int bound = std::min(expected + 2, 40);
		for (int cap : {std::max(0, expected - 1), std::min(expected, 40), bound}) {
			if (cap > 100) continue;
			assert(edit_distance_detail::wavefront(s, t, cap, 10000000) == (expected <= cap ? expected : -1));
		}
		assert(edit_distance_detail::wavefront(s, t, bound, 0) == (s.empty() && t.empty() ? 0 : -1));
	}
}

void check_carries() {
#if defined(__x86_64__) || defined(EDIT_DISTANCE_EMULATE_SIMD)
	using U = std::uint64_t;
	std::mt19937_64 rng(165781);
	for (int code = 0; code < 6561; ++code) {
		alignas(64) std::array<U, 16> p, m, eq;
		int pattern = code;
		for (int j = 0; j < 8; ++j) {
			int mode = pattern % 3;
			pattern /= 3;
			p[j] = mode ? ~U(0) : 0;
			eq[j] = mode == 2 ? ~U(0) : 0;
			m[j] = ~p[j] & rng();
			p[j + 8] = p[j];
			eq[j + 8] = eq[j];
			m[j + 8] = m[j];
		}
		auto expected_p = p, expected_m = m;
		U add = 0, cp = 1, cm = 0;
		for (int j = 0; j < 16; ++j) {
			U positive = p[j], negative = m[j], xv = eq[j] | negative;
			__uint128_t sum = __uint128_t(positive & eq[j]) + positive + add;
			add = U(sum >> 64);
			U xh = (U(sum) ^ positive) | eq[j], ph = negative | ~(xh | positive), mh = positive & xh;
			U next_cp = ph >> 63, next_cm = mh >> 63;
			ph = (ph << 1) | cp;
			mh = (mh << 1) | cm;
			expected_p[j] = mh | ~(xv | ph);
			expected_m[j] = ph & xv;
			cp = next_cp;
			cm = next_cm;
		}
		std::array<const U *, 256> masks;
		masks.fill(eq.data());
		for (int backend : {2, 3}) {
			if (backend == 2 && !have_avx2) continue;
			if (backend == 3 && !have_avx512) continue;
			alignas(64) auto actual_p = p, actual_m = m;
			if (backend == 2) edit_distance_detail::avx2("x", masks, actual_p.data(), actual_m.data(), 16);
			else edit_distance_detail::avx512("x", masks, actual_p.data(), actual_m.data(), 16);
			assert(actual_p == expected_p && actual_m == expected_m);
		}
	}
#endif
}

int main() {
	check_carries();
	check("", "", 0);
	check("kitten", "sitting", 3);
	check("sunday", "saturday", 3);
	check("abc", "acb", 2);
	check(std::string("\0\xff\x80\0", 4), std::string("\xff\0\x80", 3));
	std::vector<std::string> binary;
	for (int n = 0; n <= 6; ++n)
		for (int mask = 0; mask < (1 << n); ++mask) {
			std::string s(n, 'a');
			for (int j = 0; j < n; ++j) s[j] += (mask >> j) & 1;
			binary.push_back(s);
		}
	for (const auto &s : binary)
		for (const auto &t : binary) check(s, t);
	std::mt19937 rng(75423);
	auto random_string = [&](int n, int alphabet) {
		std::string s(n, 0);
		for (char &c : s) c = char(rng() % alphabet);
		return s;
	};
	for (int repeat = 0; repeat < 1000; ++repeat) {
		int alphabet = 1 + rng() % 256;
		check(random_string(rng() % 300, alphabet), random_string(rng() % 300, alphabet));
	}
	for (int n : {1, 2, 31, 63, 64, 65, 127, 128, 129, 191, 192, 193, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025})
		for (int alphabet : {2, 4, 26, 256}) {
			check(random_string(n, alphabet), random_string(n + 3, alphabet));
			check(std::string(n, 'a') + 'b', 'b' + std::string(n, 'a'), 2);
		}
	for (int repeat = 0; repeat < 200; ++repeat) {
		auto s = random_string(100 + rng() % 1000, 4), t = s;
		for (int j = 0; j < 1 + repeat % 12; ++j) {
			int pos = rng() % t.size();
			switch (rng() % 3) {
			case 0:
				t[pos] = char(rng() % 4);
				break;
			case 1:
				t.insert(t.begin() + pos, char(rng() % 4));
				break;
			case 2:
				t.erase(t.begin() + pos);
				break;
			}
		}
		check(s, t);
	}
	for (int alphabet : {2, 26, 256}) check(random_string(3000, alphabet), random_string(3000, alphabet));
	for (int repeat = 0; repeat < 10; ++repeat) {
		auto s = random_string(3000, 4), t = s;
		for (int j = 0; j < 20; ++j) {
			int pos = rng() % t.size();
			t[pos] = char(255);
			if (j % 2) t.erase(t.begin() + pos);
			else t.insert(t.begin() + pos, char(254));
		}
		check(s, t);
	}
	check(std::string(100000, 'a'), std::string(100000, 'a'), 0);
	check(std::string(100000, 'a'), std::string(100000, 'b'), 100000);
	check(std::string(100000, 'a'), std::string(99999, 'a'), 1);
	for (int m : {1, 31, 63, 64, 65, 127, 128, 129}) check(std::string(100000, 'a') + 'b', 'b' + std::string(m, 'a'), 100001 - m);
	auto s = random_string(100000, 26), t = s;
	for (int j = 0; j < 100; ++j) t[500 + 999 * j] = char(255);
	check(s, t, 100);
	for (int distance : {1023, 1024, 1025}) {
		t = s;
		for (int j = 0; j < distance; ++j) t[100 + 97 * j] = char(255);
		check(s, t, distance);
	}
	for (int alphabet : {2, 26, 256}) {
		int n = alphabet == 26 ? 200000 : 100000;
		auto a = random_string(n, alphabet), b = random_string(n, alphabet);
		int expected = edit_distance_detail::bit_parallel<1>(a, b);
		assert(edit_distance(a, b) == expected);
#if defined(__x86_64__) || defined(EDIT_DISTANCE_EMULATE_SIMD)
		if (have_avx2) assert(edit_distance_detail::bit_parallel<2>(a, b) == expected);
		if (have_avx512) assert(edit_distance_detail::bit_parallel<3>(a, b) == expected);
#endif
	}
}
