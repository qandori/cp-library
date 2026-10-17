#include "../../convolution/ordered_convolution.hpp"
#include "ordered_convolution_baseline.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

template <class T>
void run(const char *type, std::size_t n, std::size_t length, int repeat, int samples) {
	std::mt19937 rng(20261017);
	std::vector<T> a(n), b(length);
	for (auto &x : a) {
		if constexpr (std::is_same_v<T, mint>) x = mint::raw(rng() % mint::mod());
		else x = T(rng() % 3);
	}
	for (auto &x : b) {
		if constexpr (std::is_same_v<T, mint>) x = mint::raw(rng() % mint::mod());
		else x = T(rng() % 3);
	}
	if (ordered_convolution(a, b) != ordered_convolution_baseline(a, b))
		throw std::runtime_error("result mismatch");
	std::vector<double> times[2], cpu[2];
	std::uint64_t checksums[2]{};
	for (int trial = 0; trial < samples; ++trial)
		for (int order = 0; order < 2; ++order) {
			int variant = order ^ (trial & 1);
			auto begin = std::chrono::steady_clock::now();
			auto cpu_begin = std::clock();
			std::uint64_t checksum = 0;
			for (int iteration = 0; iteration < repeat; ++iteration) {
				auto c = variant ? ordered_convolution(a, b) : ordered_convolution_baseline(a, b);
				for (const auto &x : c) {
					if constexpr (requires { x.val(); }) checksum += x.val();
					else checksum += std::uint64_t(x);
				}
			}
			auto cpu_end = std::clock();
			if (cpu_begin == std::clock_t(-1) || cpu_end == std::clock_t(-1)) throw std::runtime_error("CPU clock unavailable");
			cpu[variant].push_back(1000.0 * (cpu_end - cpu_begin) / CLOCKS_PER_SEC);
			times[variant].push_back(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count());
			checksums[variant] = checksum;
		}
	if (checksums[0] != checksums[1]) throw std::runtime_error("checksum mismatch");
	std::vector<double> ratios;
	for (int i = 0; i < samples; ++i) ratios.push_back(cpu[0][i] / cpu[1][i]);
	std::sort(ratios.begin(), ratios.end());
	for (auto &sample : times) std::sort(sample.begin(), sample.end());
	for (auto &sample : cpu) std::sort(sample.begin(), sample.end());
	std::cout << type << ',' << n << ',' << length << ',' << repeat << ','
		<< times[0][samples / 2] << ',' << times[1][samples / 2] << ',' << times[0][samples / 2] / times[1][samples / 2] << ',' << cpu[0][samples / 2] << ',' << cpu[1][samples / 2] << ',' << ratios[samples / 2] << ',' << checksums[0] << '\n';
}

int main(int argc, char **argv) {
	std::size_t n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 31;
	std::size_t length = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 100000;
	int repeat = argc > 3 ? std::atoi(argv[3]) : 20;
	std::string type = argc > 4 ? argv[4] : "all";
	int samples = argc > 5 ? std::atoi(argv[5]) : 15;
	if (!n || repeat <= 0 || samples <= 0 || (type != "all" && type != "mint" && type != "int64")) return 1;
	std::cout << "type,n,length,repeat,before_ms,after_ms,wall_speedup,before_cpu_ms,after_cpu_ms,paired_cpu_speedup,checksum\n";
	if (type == "all" || type == "mint") {
		run<mint>("mint", n, length, repeat, samples);
	}
	if (type == "all" || type == "int64") {
		run<std::int64_t>("int64", n, length, repeat, samples);
	}
}
