#include "components/limb_arithmetic.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using U = std::uint64_t;
constexpr U base = 1000000000000000000ULL;
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
template <bool subtract>
U scalar(U *a, const U *b, std::size_t n, U carry) {
	for (std::size_t i = 0; i < n; ++i) {
		if constexpr (subtract) {
			U value = b[i] + carry;
			carry = a[i] < value;
			a[i] = a[i] + carry * base - value;
		} else {
			U value = a[i] + b[i] + carry;
			carry = value >= base;
			a[i] = value - carry * base;
		}
	}
	return carry;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

template <bool subtract>
void measure(int n, int trial) {
	std::mt19937_64 rng(123);
	std::vector<U> a(n), b(n), x, y;
	for (int i = 0; i < n; ++i) a[i] = rng() % base, b[i] = rng() % base;
	x = a; y = a;
	int repetitions = std::max(32, 10000000 / n);
	U check_x = 0, check_y = 0;
	auto run = [&](auto operation, std::vector<U> &v, U &checksum) {
		auto start = std::chrono::steady_clock::now();
		for (int i = 0; i < repetitions; ++i) checksum += operation(v.data(), b.data(), n, i & 1);
		auto end = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::nano>(end - start).count() / (n * double(repetitions));
	};
	auto fast = [](U *a, const U *b, std::size_t n, U c) {
		if constexpr (subtract) return limb_arithmetic::subtract<base>(a, b, n, c);
		else return limb_arithmetic::add<base>(a, b, n, c);
	};
	double optimized, baseline;
	if (trial & 1) {
		baseline = run(scalar<subtract>, y, check_y);
		optimized = run(fast, x, check_x);
	} else {
		optimized = run(fast, x, check_x);
		baseline = run(scalar<subtract>, y, check_y);
	}
	if (x != y || check_x != check_y) std::abort();
	std::cout << (subtract ? "subtract" : "add") << ',' << n << ',' << trial << ','
			  << optimized << ',' << baseline << ',' << baseline / optimized << '\n';
}

int main() {
#if defined(__x86_64__) || defined(__i386__)
	std::cout << "avx2=" << bool(__builtin_cpu_supports("avx2")) << '\n';
#else
	std::cout << "avx2=0\n";
#endif
	std::cout << "operation,limbs,trial,optimized_ns_per_limb,scalar_ns_per_limb,speedup\n" << std::fixed << std::setprecision(4);
	for (int n : {16, 32, 64, 256, 4096, 65536}) for (int trial = 0; trial < 5; ++trial) {
		measure<false>(n, trial);
		measure<true>(n, trial);
	}
}
