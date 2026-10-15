#include "string/edit_distance.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;
template <int mode, class W = U>
int candidate(std::string_view s, std::string_view t) {
	using U = W;
	constexpr int bits = sizeof(U) * 8;
	int words = int((t.size() + bits - 1) / bits);
	std::array<int, 256> ids{};
	int alphabet = 1;
	for (unsigned char c : t)
		if (!ids[c]) ids[c] = alphabet++;
	std::vector<U> masks(std::size_t(alphabet) * words), positive(mode == 7 ? 0 : words, ~U(0)), negative(mode == 7 ? 0 : words);
	std::vector<std::array<U, 2>> joined(mode == 7 ? words : 0, {~U(0), 0});
	for (int j = 0; j < int(t.size()); ++j) masks[std::size_t(ids[static_cast<unsigned char>(t[j])]) * words + j / bits] |= U(1) << (j % bits);
	for (unsigned char c : s) {
		const U *mask = masks.data() + std::size_t(ids[c]) * words;
		U carry_p = 1, carry_m = 0, carry_add = 0;
		auto update = [&](int j) {
			U p = mode == 7 ? joined[j][0] : positive[j], m = mode == 7 ? joined[j][1] : negative[j], eq = mask[j], xv = eq | m, xh;
			if constexpr (mode == 1 || mode == 2 || mode == 6 || mode == 7) {
				__uint128_t sum = __uint128_t(eq & p) + p + carry_add;
				carry_add = U(sum >> 64);
				xh = (U(sum) ^ p) | eq;
			} else {
				eq |= carry_m;
				xh = (((eq & p) + p) ^ p) | eq;
			}
			if constexpr (mode == 5 || mode == 6) xv = xh | m;
			U ph = m | ~(xh | p), mh = p & xh;
			U next_p = ph >> (bits - 1), next_m = mh >> (bits - 1);
			ph = (ph << 1) | carry_p;
			mh = (mh << 1) | carry_m;
			if constexpr (mode == 7) joined[j] = {mh | ~(xv | ph), ph & xv};
			else positive[j] = mh | ~(xv | ph), negative[j] = ph & xv;
			carry_p = next_p;
			carry_m = next_m;
		};
		int j = 0;
		if constexpr (mode == 2 || mode == 3)
			for (; j + 2 <= words; j += 2) update(j), update(j + 1);
		if constexpr (mode == 4)
			for (; j + 4 <= words; j += 4) update(j), update(j + 1), update(j + 2), update(j + 3);
		for (; j < words; ++j) update(j);
	}
	if (t.size() % bits) {
		U valid = (U(1) << (t.size() % bits)) - 1;
		if constexpr (mode == 7) joined.back()[0] &= valid, joined.back()[1] &= valid;
		else positive.back() &= valid, negative.back() &= valid;
	}
	int result = int(s.size());
	for (int j = 0; j < words; ++j) {
		if constexpr (mode == 7) result += std::popcount(std::uint64_t(joined[j][0])) - std::popcount(std::uint64_t(joined[j][1]));
		else result += std::popcount(std::uint64_t(positive[j])) - std::popcount(std::uint64_t(negative[j]));
		if constexpr (bits == 128) result += std::popcount(std::uint64_t(positive[j] >> 64)) - std::popcount(std::uint64_t(negative[j] >> 64));
	}
	return result;
}

int scalar(std::string_view s, std::string_view t) {
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

int main(int argc, char **argv) {
	std::string mode = argc > 1 ? argv[1] : "library", kind = argc > 2 ? argv[2] : "random26";
	int n = argc > 3 ? std::atoi(argv[3]) : 100000, repeats = argc > 4 ? std::atoi(argv[4]) : 5;
	std::mt19937 rng(1234);
	std::string s(n, 0), t(n, 0);
	int alphabet = kind == "random2" ? 2 : kind == "random4" ? 4
									   : kind == "random256" ? 256
															 : 26;
	for (char &c : s) c = char(rng() % alphabet);
	for (char &c : t) c = char(rng() % alphabet);
	if (kind == "identical") t = s;
	if (kind == "near1" || kind == "near100" || kind == "near1000") {
		t = s;
		int changes = kind == "near1" ? 1 : kind == "near100" ? 100
															  : 1000;
		for (int j = 0; j < changes; ++j) t[rng() % n] = char(255);
	}
	if (kind == "indel100") {
		t = s;
		for (int j = 0; j < 50; ++j) {
			t.erase(t.begin() + rng() % t.size());
			t.insert(t.begin() + rng() % t.size(), char(255));
		}
	}
	if (kind == "disjoint") s.assign(n, 'a'), t.assign(n, 'b');
	if (kind == "periodic") {
		for (int j = 0; j < n; ++j) s[j] = char(j % 7), t[j] = char((j + 1) % 7);
	}
	if (kind == "blocks") s.assign(n / 2, 'a'), s.append(n - n / 2, 'b'), t.assign(n / 2, 'b'), t.append(n - n / 2, 'a');
	if (kind == "unbalanced") t.resize(std::min(n, 100));
	std::vector<double> times;
	int checksum = -1;
	for (int rep = 0; rep < repeats; ++rep) {
		auto start = std::chrono::steady_clock::now();
		int result;
		if (mode == "library") result = edit_distance(s, t);
		else if (mode == "scalar64") result = edit_distance_detail::bit_parallel<1>(s, t);
#if defined(__x86_64__)
		else if (mode == "avx2" && __builtin_cpu_supports("avx2")) result = edit_distance_detail::bit_parallel<2>(s, t);
		else if (mode == "avx512" && __builtin_cpu_supports("avx512f")) result = edit_distance_detail::bit_parallel<3>(s, t);
#endif
		else if (mode == "block") result = candidate<0>(s, t);
		else if (mode == "add128") result = candidate<1>(s, t);
		else if (mode == "add128_unroll2") result = candidate<2>(s, t);
		else if (mode == "unroll2") result = candidate<3>(s, t);
		else if (mode == "unroll4") result = candidate<4>(s, t);
		else if (mode == "d0_block") result = candidate<5>(s, t);
		else if (mode == "d0_add128") result = candidate<6>(s, t);
		else if (mode == "block128") result = candidate<0, __uint128_t>(s, t);
		else if (mode == "d0_block128") result = candidate<5, __uint128_t>(s, t);
		else if (mode == "interleaved") result = candidate<7>(s, t);
		else if (mode == "scalar") result = scalar(s, t);
		else if (mode == "wave1" || mode == "wave4" || mode == "wave16") {
			int budget = (mode == "wave1" ? 1 : mode == "wave4" ? 4
																: 16)
						 << 20;
			result = edit_distance_detail::wavefront(s, t, 4096, budget);
		} else return 1;
		times.push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
		if (checksum != -1 && result != checksum) return 2;
		checksum = result;
	}
	std::sort(times.begin(), times.end());
	std::cout << mode << ' ' << kind << ' ' << n << ' ' << checksum << ' ' << std::fixed << std::setprecision(3) << times[times.size() / 2] << '\n';
}
