#include <array>
#include <cstdint>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "components/fast_io.hpp"

inline std::uint64_t dc_benchmark_seed = 713;
#ifdef DC_PROFILE
inline std::uint64_t dc_splays = 0, dc_rotations = 0, dc_visits = 0, dc_promotions = 0;
inline std::array<std::uint64_t, 32> dc_levels{};
#endif
#ifndef DC_BENCH_HEADER
#define DC_BENCH_HEADER "data_structure/dynamic_connectivity.hpp"
#endif
#include DC_BENCH_HEADER

std::int64_t add(std::int64_t a, std::int64_t b) { return a + b; }
std::int64_t zero() { return 0; }

struct Query { int type, u, v; std::int64_t x; };

int main(int argc, char **argv) {
	if (argc > 1) dc_benchmark_seed = std::stoull(argv[1]);
	FastInput input;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	std::vector<std::int64_t> values(n), answers;
	for (auto &x : values) input.read(x);
	std::vector<Query> queries(q);
	for (auto &query : queries) {
		input.read(query.type); input.read(query.u);
		if (query.type < 2) input.read(query.v);
		else if (query.type == 2) input.read(query.x);
	}
	answers.reserve(q);
	auto start = std::chrono::steady_clock::now();
	DynamicConnectivity<std::int64_t, add, zero> graph(values);
	graph.reserve(q);
	for (const auto &query : queries) {
		if (query.type == 0) graph.link(query.u, query.v);
		else if (query.type == 1) graph.cut(query.u, query.v);
		else if (query.type == 2) graph.set(query.u, graph.get(query.u) + query.x);
		else answers.push_back(graph.prod(query.u));
	}
	double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	std::uint64_t checksum = answers.size();
	for (auto x : answers) checksum = (checksum ^ std::uint64_t(x)) * 0x9e3779b97f4a7c15ULL;
	std::printf("{\"ms\":%.9f,\"checksum\":\"%llu\",\"answers\":%zu", ms, static_cast<unsigned long long>(checksum), answers.size());
#ifdef DC_PROFILE
	std::printf(",\"splays\":%llu,\"rotations\":%llu,\"flag_visits\":%llu,\"promotions\":%llu,\"levels\":[", static_cast<unsigned long long>(dc_splays), static_cast<unsigned long long>(dc_rotations), static_cast<unsigned long long>(dc_visits), static_cast<unsigned long long>(dc_promotions));
	for (int i = 0; i < 32; ++i) std::printf("%s%llu", i ? "," : "", static_cast<unsigned long long>(dc_levels[i]));
	std::printf("]");
#endif
	std::puts("}");
}
