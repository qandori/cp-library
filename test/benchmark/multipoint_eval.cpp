#include "fps/multipoint_eval.hpp"
#include MULTIPOINT_BASELINE
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <random>
#include <string>

using T = fps_detail::Mint;
using F = FormalPowerSeries<T>;
using Fn = std::vector<T> (*)(const F &, const std::vector<T> &);
volatile std::uint64_t multipoint_sink = 0;

std::uint64_t checksum(const std::vector<T> &values) {
	std::uint64_t result = 0;
	for (T x : values) result = result * 1000003 + x.val();
	return result;
}

struct Measurement { double seconds, cpu; std::size_t iterations; };

Measurement measure(Fn fn, const F &f, const std::vector<T> &xs, double seconds) {
	auto start = std::chrono::steady_clock::now();
	const std::clock_t cpu_start = std::clock();
	std::size_t count = 0;
	double elapsed, cpu;
	do {
		for (int batch = 0; batch < 16; ++batch) {
			auto values = fn(f, xs);
			multipoint_sink = multipoint_sink + checksum(values);
			++count;
		}
		cpu = double(std::clock() - cpu_start) / CLOCKS_PER_SEC;
		elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	} while (cpu < seconds);
	return {elapsed / count, cpu, count};
}

std::vector<T> forced_direct(const F &f, const std::vector<T> &xs) {
#if defined(__x86_64__) || defined(__i386__)
	if (fps_detail::NTT::vectorized()) {
		std::vector<T> result(xs.size());
		using O = fps_detail::Operations<T>;
		fps_detail::evaluate_direct(O::words(result), O::words(f), f.size(), O::words(xs), xs.size());
		return result;
	}
#endif
	return baseline_multipoint_eval(f, xs);
}

int main(int argc, char **argv) {
	const int pairs = argc > 1 ? std::atoi(argv[1]) : 8;
	const double seconds = argc > 2 ? std::atof(argv[2]) : 0.15;
	if (pairs < 3 || seconds < 0.15) return 1;
	std::mt19937 random(731);
	std::vector<std::pair<int, int>> cases;
	for (int n : {2, 4, 7, 8, 16, 31, 32})
		for (int m : {7, 8, 16, 31, 32, 33, 64, 4096}) cases.push_back({n, m});
	const std::vector<std::pair<int, int>> larger = {
		{4, 4095}, {4, 4097}, {7, 4095}, {7, 4097}, {8, 63}, {8, 65},
		{32, 9}, {32, 15}, {32, 63}, {32, 65}, {32, 131072},
		{33, 8}, {256, 8}, {4096, 7}, {4096, 8}, {4096, 31}, {4096, 32}, {131072, 32},
		{33, 33}, {1024, 33}, {32768, 129}, {32768, 32768}
	};
	cases.insert(cases.end(), larger.begin(), larger.end());
	Fn candidate = multipoint_eval<T>;
	if (argc > 3 && std::string(argv[3]) == "grid") {
		cases.clear();
		for (int n : {2, 4, 7, 8, 16, 31, 32})
			for (int m : {7, 8, 16, 31, 32, 33, 64, 4096}) cases.push_back({n, m});
		candidate = forced_direct;
	}
	std::puts("n,m,pair,order,baseline_seconds,candidate_seconds,speedup,checksum,baseline_batch_cpu,candidate_batch_cpu,baseline_iterations,candidate_iterations");
	for (auto [n, m] : cases) {
		F f(n);
		std::vector<T> xs(m);
		for (T &x : f) x = random();
		for (T &x : xs) x = random();
		auto before = baseline_multipoint_eval(f, xs), after = candidate(f, xs);
		if (before != after) return 2;
		const auto expected = checksum(before);
		for (int pair = 0; pair < pairs; ++pair) {
			Measurement a, b;
			if (pair % 2 == 0) {
				a = measure(baseline_multipoint_eval<T>, f, xs, seconds);
				b = measure(candidate, f, xs, seconds);
			} else {
				b = measure(candidate, f, xs, seconds);
				a = measure(baseline_multipoint_eval<T>, f, xs, seconds);
			}
			std::printf("%d,%d,%d,%s,%.12g,%.12g,%.8g,%llu,%.8g,%.8g,%zu,%zu\n", n, m, pair,
				pair % 2 ? "CB" : "BC", a.seconds, b.seconds, a.seconds / b.seconds,
				static_cast<unsigned long long>(expected), a.cpu, b.cpu, a.iterations, b.iterations);
			std::fflush(stdout);
		}
	}
}
