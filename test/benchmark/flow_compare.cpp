#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "graph/max_flow.hpp"
#include "components/fast_io.hpp"

int main(int argc, char** argv) {
	FastInput input;
	FastOutput output;
	int repetitions = argc > 1 ? std::atoi(argv[1]) : 1;
	bool residual = argc > 2 && std::string(argv[2]) == "residual";
	int n = 0, m = 0, s = 0, t = 0;
	input.read(n), input.read(m), input.read(s), input.read(t);
	struct Edge { int u, v; std::int64_t cap; };
	std::vector<Edge> edges(m);
	for (auto& e : edges) input.read(e.u), input.read(e.v), input.read(e.cap);
	long long solver_ns = 0;
	for (int repeat = 0; repeat < repetitions; ++repeat) {
		MaxFlow<> graph(n);
		graph.reserve(m);
		for (auto e : edges) graph.add_edge(e.u - 1, e.v - 1, e.cap);
		auto start = std::chrono::steady_clock::now();
		auto result = graph.flow(s - 1, t - 1);
		solver_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
		output.println(result);
		std::uint64_t hash = 1469598103934665603ULL;
		for (int i = 0; i < graph.edge_count(); ++i) {
			auto e = graph.get_edge(i);
			for (std::uint64_t x : {std::uint64_t(e.from), std::uint64_t(e.to), std::uint64_t(e.cap), std::uint64_t(e.flow)}) hash = (hash ^ x) * 1099511628211ULL;
			if (residual) { output.write(e.from); output.write(' '); output.write(e.to); output.write(' '); output.write(e.cap); output.write(' '); output.println(e.flow); }
		}
		output.println(hash);
	}
	std::fprintf(stderr, "solver_ns=%lld\n", solver_ns);
}
