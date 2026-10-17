#include "convolution/subset_convolution.hpp"
#include "math/mint.hpp"
#include "subset_convolution_baseline.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

std::uint64_t checksum = 0;
template <class T>
std::uint64_t value(T x) {
	if constexpr (std::is_same_v<T, mint>) return x.val();
	else if constexpr (std::is_same_v<T, double>) return std::bit_cast<std::uint64_t>(x);
	else return std::uint64_t(x);
}

template <class T>
void compare(const char *type, int exponent, int samples) {
	std::size_t n = std::size_t(1) << exponent;
	std::mt19937 rng(20261017);
	std::vector<T> a(n), b(n);
	for (auto &x : a) x = T(rng() % 1001);
	for (auto &x : b) x = T(rng() % 1001);
	auto expected = subset_convolution_baseline::subset_convolution(a, b);
	if (expected != subset_convolution(a, b)) std::abort();
	int repetitions = std::max(1, int(16384 / n));
	std::vector<double> times[2];
	for (int sample = 0; sample < samples; ++sample)
		for (int turn = 0; turn < 2; ++turn) {
			int variant = (sample + turn) % 2;
			double total = 0;
			for (int repetition = 0; repetition < repetitions; ++repetition) {
				auto begin = std::chrono::steady_clock::now();
				auto c = variant ? subset_convolution(a, b) : subset_convolution_baseline::subset_convolution(a, b);
				auto end = std::chrono::steady_clock::now();
				total += std::chrono::duration<double, std::milli>(end - begin).count();
				checksum += value(c[(sample + repetition) % n]);
			}
			times[variant].push_back(total / repetitions);
		}
	for (auto &t : times) std::sort(t.begin(), t.end());
	double before = times[0][samples / 2], after = times[1][samples / 2];
	std::cout << type << ',' << exponent << ',' << before << ',' << after << ',' << before / after << '\n';
}

int main(int argc, char **argv) {
	int exponent = argc > 1 ? std::atoi(argv[1]) : 18;
	int samples = argc > 2 ? std::atoi(argv[2]) : 9;
	std::string type = argc > 3 ? argv[3] : "all";
	if (exponent < 0 || exponent > 20 || samples <= 0) return 1;
	std::cout << std::fixed << std::setprecision(6);
	std::cout << "type,log2_n,baseline_ms,optimized_ms,speedup\n";
	if (type == "mint" || type == "all") compare<mint>("mint", exponent, samples);
	if (type == "uint32" || type == "all") compare<std::uint32_t>("uint32", exponent, samples);
	if (type == "uint64" || type == "all") compare<std::uint64_t>("uint64", exponent, samples);
	if (type == "int128" || type == "all") compare<__int128_t>("int128", exponent, samples);
	if (type == "double" || type == "all") compare<double>("double", exponent, samples);
	std::cerr << "checksum=" << checksum << '\n';
}
