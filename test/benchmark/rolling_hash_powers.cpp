#include "Baseline.hpp"
#include "Candidate.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

std::uint64_t rolling_benchmark_base;
volatile std::uint64_t sink;

void require(bool ok) {
	if (!ok) std::abort();
}

template <class H>
void reset() {
	std::vector<std::uint64_t>{1}.swap(H::powers);
}

void check_powers() {
	require(Baseline::powers.size() == Candidate::powers.size());
	std::uint64_t oracle = 1;
	for (std::size_t i = 0; i < Baseline::powers.size(); ++i) {
		require(Baseline::powers[i] == oracle);
		require(Candidate::powers[i] == oracle);
		oracle = std::uint64_t(__uint128_t(oracle) * rolling_benchmark_base % Baseline::mod);
	}
}

void check_reserve(int n) {
	Baseline::reserve(n);
	Candidate::reserve(n);
	check_powers();
}

void verify_boundaries() {
	for (int start = 0; start <= 64; ++start) {
		for (int growth = 0; growth <= 64; ++growth) {
			reset<Baseline>();
			reset<Candidate>();
			check_reserve(start);
			check_reserve(start + growth);
			check_reserve(start);
		}
	}
	for (int growth : {1, 2, 3, 4, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65}) {
		reset<Baseline>();
		reset<Candidate>();
		for (int n = 0; n < 4096; n += growth) check_reserve(n);
		check_reserve(4096);
	}
	std::mt19937 rng(71239);
	for (int n = 0; n <= 257; ++n) {
		std::string s(n, '\0');
		for (char &c : s) c = char(rng() % 256);
		reset<Baseline>();
		reset<Candidate>();
		Baseline a(s);
		Candidate b(s);
		check_powers();
		require(a.prefix == b.prefix);
		for (int l = 0; l <= n; ++l) {
			std::uint64_t oracle = 0;
			for (int r = l; r <= n; ++r) {
				require(a.get(l, r) == oracle && b.get(l, r) == oracle);
				if (r < n) oracle = std::uint64_t((__uint128_t(oracle) * rolling_benchmark_base + static_cast<unsigned char>(s[r]) + 1) % Baseline::mod);
			}
		}
	}
}

template <class H>
__attribute__((noinline)) std::uint64_t run(int n, int mode, int reps, const std::string &s) {
	std::uint64_t sum = 0;
	for (int r = 0; r < reps; ++r) {
		if (mode != 2) reset<H>();
		if (mode == 0) {
			H::reserve(n);
			sum += H::powers.back();
		} else if (mode == 3) {
			for (int i = 1; i < n; i += 7) H::reserve(i);
			H::reserve(n);
			sum += H::powers.back();
		} else {
			H hash(s);
			sum += hash.get();
		}
	}
	return sum;
}

struct Timing {
	double cpu, wall;
	std::uint64_t checksum;
};

template <class H>
Timing timed(int n, int mode, int reps, const std::string &s) {
	auto wall_start = std::chrono::steady_clock::now();
	auto cpu_start = std::clock();
	auto checksum = run<H>(n, mode, reps, s);
	auto cpu_stop = std::clock();
	auto wall_stop = std::chrono::steady_clock::now();
	sink = checksum;
	return {double(cpu_stop - cpu_start) / CLOCKS_PER_SEC, std::chrono::duration<double>(wall_stop - wall_start).count(), checksum};
}

int main(int argc, char **argv) {
	if (argc < 2) return 1;
	rolling_benchmark_base = argc > 6 ? std::strtoull(argv[6], nullptr, 10) : 12345678901234567ULL;
	if (rolling_benchmark_base < 256 || rolling_benchmark_base >= Baseline::mod - 256) return 1;
	if (std::string(argv[1]) == "verify") {
		verify_boundaries();
		std::cout << "PASS boundary, all powers, prefix, substring oracle base=" << rolling_benchmark_base << '\n';
		return 0;
	}
	int n = std::atoi(argv[1]), mode = argc > 2 ? std::atoi(argv[2]) : 0, pairs = argc > 3 ? std::atoi(argv[3]) : 10;
	double min_seconds = argc > 4 ? std::atof(argv[4]) : .35;
	int start_order = argc > 5 ? std::atoi(argv[5]) : 0;
	if (n < 1 || mode < 0 || mode > 3 || pairs < 1 || min_seconds <= 0) return 1;
	std::string s(n, 'a');
	std::mt19937 rng(71239);
	for (char &c : s) c = char('a' + rng() % 26);
	for (int m = 0; m < 4; ++m) {
		auto a = run<Baseline>(n, m, 1, s), b = run<Candidate>(n, m, 1, s);
		require(a == b);
		check_powers();
	}
	int reps = 1;
	while (true) {
		auto a = timed<Baseline>(n, mode, reps, s), b = timed<Candidate>(n, mode, reps, s);
		require(a.checksum == b.checksum);
		if (std::min(a.cpu, b.cpu) >= min_seconds) break;
		if (reps > 100000000) return 2;
		reps *= 2;
	}
	std::cout << "n,mode,pair,reps,baseline_cpu,candidate_cpu,ratio,baseline_wall,candidate_wall,checksum\n" << std::setprecision(9);
	for (int i = 0; i < pairs; ++i) {
		Timing a, b;
		if ((i + start_order) % 2) {
			b = timed<Candidate>(n, mode, reps, s);
			a = timed<Baseline>(n, mode, reps, s);
		} else {
			a = timed<Baseline>(n, mode, reps, s);
			b = timed<Candidate>(n, mode, reps, s);
		}
		require(a.checksum == b.checksum);
		std::cout << n << ',' << mode << ',' << i << ',' << reps << ',' << a.cpu << ',' << b.cpu << ',' << a.cpu / b.cpu << ',' << a.wall << ',' << b.wall << ',' << a.checksum << std::endl;
	}
}
