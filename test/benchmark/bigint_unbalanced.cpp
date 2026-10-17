#ifndef BIGINT_BASELINE_HEADER
#error "Use bigint_unbalanced.py to prepare the baseline"
#endif
#define BigInt BaselineBigInt
#include BIGINT_BASELINE_HEADER
#undef BigInt
#undef BIGINT_HPP
#include "math/bigint.hpp"
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

double cpu_time() {
	timespec value;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &value);
	return value.tv_sec + value.tv_nsec * 1e-9;
}
struct Timing {
	double cpu, wall;
	std::size_t count;
};
std::uint64_t sink = 0;
template<class T>
Timing batch(const std::vector<T>& a, const std::vector<T>& b, std::size_t repetitions, double minimum = 0) {
	auto begin = cpu_time();
	auto wall = std::chrono::steady_clock::now();
	std::uint64_t checksum = 0;
	std::size_t count = 0;
	do {
		for (std::size_t i = 0; i < repetitions; ++i) {
			T result = a[i & 3] * b[i & 3];
			checksum = checksum * 713 + static_cast<std::uint64_t>(result);
		}
		count += repetitions;
	} while (cpu_time() - begin < minimum);
	sink ^= checksum;
	return {cpu_time() - begin, std::chrono::duration<double>(std::chrono::steady_clock::now() - wall).count(), count};
}
int main(int argc, char** argv) {
	if (argc < 3) return 1;
	int n = std::stoi(argv[1]), m = std::stoi(argv[2]);
	int trials = argc > 3 ? std::stoi(argv[3]) : 7;
	if (n < 1 || m < 1 || trials < 1) return 1;
	std::mt19937_64 random(982341);
	auto make = [&](int limbs) {
		std::string result(std::size_t(limbs) * 18, '0');
		for (char& digit : result) digit = '0' + random() % 10;
		result[0] = '1' + random() % 9;
		return result;
	};
	std::vector<BigInt> a, b;
	std::vector<BaselineBigInt> x, y;
	for (int i = 0; i < 4; ++i) {
		auto first = make(n), second = make(m);
		a.emplace_back(first), b.emplace_back(second);
		x.emplace_back(first), y.emplace_back(second);
		if ((a.back() * b.back()).str() != (x.back() * y.back()).str()) return 2;
	}
	std::size_t old_repetitions = 1, new_repetitions = 1;
	while (batch(x, y, old_repetitions).cpu < .20) old_repetitions *= 2;
	while (batch(a, b, new_repetitions).cpu < .20) new_repetitions *= 2;
	std::cout << std::setprecision(12);
	for (int trial = 0; trial < trials; ++trial) {
		Timing old, current;
		if ((trial + n + m) & 1) {
			current = batch(a, b, new_repetitions, .20);
			old = batch(x, y, old_repetitions, .20);
		} else {
			old = batch(x, y, old_repetitions, .20);
			current = batch(a, b, new_repetitions, .20);
		}
		std::cout << n << ' ' << m << ' ' << trial << ' ' << old.count << ' ' << current.count << ' '
			<< old.cpu << ' ' << current.cpu << ' ' << (old.cpu / old.count) / (current.cpu / current.count) << ' '
			<< old.wall << ' ' << current.wall << '\n';
	}
	std::cerr << sink << '\n';
}
