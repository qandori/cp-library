#include "string/lcs.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;
template <int mode, int batch = 1>
int candidate(std::string_view a, std::string_view b) {
	if (b.empty()) return 0;
	int w = int((b.size() + 63) / 64);
	std::array<int, 256> ids;
	ids.fill(-1);
	int alphabet = 0;
	for (unsigned char c : b)
		if (ids[c] == -1) ids[c] = alphabet++;
	std::vector<U> masks(std::size_t(alphabet) * w), row(w, mode == 3 ? 0 : ~U(0));
	for (int j = 0; j < int(b.size()); ++j) masks[std::size_t(ids[static_cast<unsigned char>(b[j])]) * w + j / 64] |= U(1) << (j % 64);
	std::vector<const U *> rows;
	for (unsigned char c : a)
		if (ids[c] != -1) rows.push_back(masks.data() + std::size_t(ids[c]) * w);
	for (std::size_t i = 0; i < rows.size();) {
		int size = std::min<std::size_t>(batch, rows.size() - i);
		std::array<U, batch> carries{}, previous{};
		if constexpr (mode == 3) carries.fill(1);
		auto update = [&](int j) {
			U v = row[j];
#pragma GCC unroll 8
			for (int k = 0; k < batch; ++k) {
				if (k >= size) break;
				U u = v & rows[i + k][j], sum;
				if constexpr (mode == 6) {
					sum = v + u + carries[k];
					carries[k] = (u | (v & ~sum)) >> 63;
				} else if constexpr (mode == 0 || mode >= 4) {
					__uint128_t z = __uint128_t(v) + u + carries[k];
					sum = U(z);
					carries[k] = U(z >> 64);
				} else if constexpr (mode == 1) {
					U t;
					bool c1 = __builtin_add_overflow(v, u, &t), c2 = __builtin_add_overflow(t, carries[k], &sum);
					carries[k] = c1 | c2;
				} else if constexpr (mode == 2) {
					U t = v + u;
					sum = t + carries[k];
					carries[k] = (t < v) | (sum < t);
				} else if constexpr (mode == 3) {
					U x = v | rows[i + k][j], y = (v << 1) | carries[k], d;
					bool c1 = __builtin_sub_overflow(x, y, &d), c2 = __builtin_sub_overflow(d, previous[k], &sum);
					previous[k] = c1 | c2;
					carries[k] = v >> 63;
					v = x & ~sum;
					continue;
				}
				v = sum | (v ^ u);
			}
			row[j] = v;
		};
		int j = 0;
		if constexpr (mode == 4)
			for (; j + 4 <= w; j += 4) update(j), update(j + 1), update(j + 2), update(j + 3);
		if constexpr (mode == 5)
			for (; j + 2 <= w; j += 2) update(j), update(j + 1);
		for (; j < w; ++j) update(j);
		i += size;
	}
	int count = 0;
	for (U v : row) count += std::popcount(mode == 3 ? v : ~v);
	return count;
}

int main(int argc, char **argv) {
	std::string implementation = argc > 1 ? argv[1] : "length", kind = argc > 2 ? argv[2] : "random26";
	int n = argc > 3 ? std::atoi(argv[3]) : 100000, repeats = argc > 4 ? std::atoi(argv[4]) : 5;
	std::mt19937 rng(1234);
	std::string a(n, 0), b(n, 0);
	int alphabet = kind == "random2" ? 2 : kind == "random4" ? 4
									   : kind == "random256" ? 256
															 : 26;
	for (char &c : a) c = char(rng() % alphabet);
	for (char &c : b) c = char(rng() % alphabet);
	if (kind == "identical") b = a;
	if (kind == "near") {
		b = a;
		for (int j = 0; j < n / 100; ++j) b[rng() % n] = char(rng() % alphabet);
	}
	if (kind == "disjoint") a.assign(n, 'a'), b.assign(n, 'b');
	if (kind == "blocks") a.assign(n / 2, 'a'), a.append(n - n / 2, 'b'), b.assign(n / 2, 'b'), b.append(n - n / 2, 'a');
	if (kind == "unbalanced") b.resize(std::min(n, 100));
	std::vector<double> times;
	int checksum = -1;
	for (int rep = 0; rep < repeats; ++rep) {
		auto start = std::chrono::steady_clock::now();
		int value;
		if (implementation == "length") value = lcs_length(a, b);
		else if (implementation == "restore") value = int(lcs(a, b).size());
		else if (implementation == "scalar64") value = lcs_detail::count(lcs_detail::profile<false, 1>(a, b));
#if defined(__x86_64__)
		else if (implementation == "avx2" && __builtin_cpu_supports("avx2")) value = lcs_detail::count(lcs_detail::profile<false, 2>(a, b));
		else if (implementation == "avx512" && __builtin_cpu_supports("avx512f")) value = lcs_detail::count(lcs_detail::profile<false, 3>(a, b));
		else if ((implementation == "restore_avx2" && __builtin_cpu_supports("avx2")) || (implementation == "restore_avx512" && __builtin_cpu_supports("avx512f"))) {
			std::string result;
			if (implementation == "restore_avx2") lcs_detail::restore<2>(a, b, result);
			else lcs_detail::restore<3>(a, b, result);
			value = int(result.size());
		}
#endif
		else if (implementation == "add128")
			value = candidate<0>(a, b);
		else if (implementation == "overflow") value = candidate<1>(a, b);
		else if (implementation == "compare") value = candidate<2>(a, b);
		else if (implementation == "subtract") value = candidate<3>(a, b);
		else if (implementation == "unroll4") value = candidate<4>(a, b);
		else if (implementation == "unroll2") value = candidate<5>(a, b);
		else if (implementation == "batch2") value = candidate<0, 2>(a, b);
		else if (implementation == "batch4") value = candidate<0, 4>(a, b);
		else if (implementation == "batch8") value = candidate<0, 8>(a, b);
		else if (implementation == "bitcarry") value = candidate<6>(a, b);
		else return 1;
		times.push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
		if (checksum != -1 && checksum != value) return 2;
		checksum = value;
	}
	std::sort(times.begin(), times.end());
	std::cout << implementation << ' ' << kind << ' ' << n << ' ' << checksum << ' ' << std::fixed << std::setprecision(3) << times[times.size() / 2] << '\n';
}
