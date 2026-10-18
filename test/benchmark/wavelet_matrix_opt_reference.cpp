#ifdef WM_OPT_HEADER
#include WM_OPT_HEADER
#else
#ifndef WM_OPT_REFERENCE_HEADER
#define WM_OPT_REFERENCE_HEADER ".verify-helper/cache/wm-opt/reference/402999_core.hpp"
#endif
#include WM_OPT_REFERENCE_HEADER
#endif
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

struct Query {
	std::uint32_t l, r, k;
};

int main(int argc, char **argv) {
	FastInput input;
	std::uint32_t n, q;
	if (!input.read(n) || !input.read(q)) return 2;
	std::vector<std::uint32_t> a(n), answers(q);
	for (auto &x : a) input.read(x);
	std::vector<Query> queries(q);
	for (auto &z : queries) input.read(z.l), input.read(z.r), input.read(z.k);
	auto start = std::chrono::steady_clock::now();
#ifdef WM_OPT_HEADER
	WaveletMatrix<std::uint32_t> wm(a);
#else
#ifdef WM_OPT_REF_SCALAR
	canard::wavelet_matrix wm(a, canard::execution::scalar{});
#else
	canard::wavelet_matrix wm(a, canard::execution::avx2{});
#endif
#endif
	auto built = std::chrono::steady_clock::now();
	for (std::size_t i = 0; i < queries.size(); ++i) {
		auto z = queries[i];
		answers[i] = wm.kth_smallest(z.l, z.r, z.k);
	}
	auto finished = std::chrono::steady_clock::now();
	if (argc == 2 && !std::strcmp(argv[1], "--answers")) {
		FastOutput output;
		for (auto x : answers) output.write(x), output.write('\n');
		return 0;
	}
	std::uint64_t checksum = 0;
	for (auto x : answers) checksum = (checksum ^ x) * 0x9e3779b97f4a7c15;
	double build_ms = std::chrono::duration<double, std::milli>(built - start).count();
	double query_ms = std::chrono::duration<double, std::milli>(finished - built).count();
	std::printf("{\"ms\":%.12f,\"build_ms\":%.12f,\"query_ms\":%.12f,\"checksum\":\"%llu\",\"answers\":%u}\n",
			build_ms + query_ms, build_ms, query_ms, static_cast<unsigned long long>(checksum), q);
}
