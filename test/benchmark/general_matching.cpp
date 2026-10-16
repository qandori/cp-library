#include "plain.hpp"
#include "greedy.hpp"
#include "reverse.hpp"
#include "short.hpp"
#include "short2.hpp"
#include "free_first.hpp"
#include "bitset.hpp"
#include "bitset_short.hpp"
#include "bitset_short2.hpp"
#include "current.hpp"
#include "components.hpp"
#include "components_short.hpp"
#include "components_short2.hpp"
#if __has_include("reference.hpp")
#include "reference.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Clock = std::chrono::steady_clock;
std::uint64_t checksum;
template <class Solver>
void run(const char *name, const std::string &kind, int n, const std::vector<std::pair<int, int>> &edges, int trial, int repeats) {
	int words = (n + 63) / 64;
	std::vector<std::uint64_t> actual(std::size_t(n) * words);
	for (auto [u, v] : edges) {
		actual[std::size_t(u) * words + (v >> 6)] |= std::uint64_t(1) << (v & 63);
		actual[std::size_t(v) * words + (u >> 6)] |= std::uint64_t(1) << (u & 63);
	}
	double construction = 0, solving = 0;
	int result = -1;
	for (int repeat = 0; repeat < repeats; ++repeat) {
		auto t0 = Clock::now();
		Solver g(n);
		g.reserve(edges.size());
		for (auto [u, v] : edges) g.add_edge(u, v);
		auto t1 = Clock::now();
		int count = g.build();
		auto t2 = Clock::now();
		int matched = 0;
		for (int u = 0; u < n; ++u) if (g.mate[u] != -1) {
			int v = g.mate[u];
			if (v < 0 || v >= n || v == u || g.mate[v] != u || !(actual[std::size_t(u) * words + (v >> 6)] >> (v & 63) & 1)) std::abort();
			++matched;
		}
		if (matched != 2 * count || (result != -1 && result != count)) std::abort();
		result = count;
		construction += std::chrono::duration<double, std::micro>(t1 - t0).count();
		solving += std::chrono::duration<double, std::micro>(t2 - t1).count();
		checksum += count;
	}
	std::cout << name << ',' << kind << ',' << n << ',' << edges.size() << ',' << trial << ',' << construction / repeats << ',' << solving / repeats << ',' << (construction + solving) / repeats << ',' << result << '\n';
}
void benchmark(const std::string &kind, int n, const std::vector<std::pair<int, int>> &edges, int trials, int repeats, std::mt19937 &rng) {
	for (int trial = 0; trial < trials; ++trial) {
		std::vector<int> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
		std::shuffle(order.begin(), order.end(), rng);
		for (int id : order) {
			if (id == 0) run<Matching_plain>("plain", kind, n, edges, trial, repeats);
			if (id == 1) run<Matching_greedy>("greedy", kind, n, edges, trial, repeats);
			if (id == 2) run<Matching_reverse>("reverse", kind, n, edges, trial, repeats);
			if (id == 3) run<Matching_short>("short", kind, n, edges, trial, repeats);
			if (id == 4) run<Matching_short2>("short2", kind, n, edges, trial, repeats);
			if (id == 5) run<Matching_free_first>("free_first", kind, n, edges, trial, repeats);
			if (id == 6) run<Matching_bitset>("bitset", kind, n, edges, trial, repeats);
			if (id == 7) run<Matching_bitset_short>("bitset_short", kind, n, edges, trial, repeats);
			if (id == 8) run<Matching_bitset_short2>("bitset_short2", kind, n, edges, trial, repeats);
			if (id == 9) run<Matching_current>("current", kind, n, edges, trial, repeats);
			if (id == 11) run<Matching_components>("components", kind, n, edges, trial, repeats);
			if (id == 12) run<Matching_components_short>("components_short", kind, n, edges, trial, repeats);
			if (id == 13) run<Matching_components_short2>("components_short2", kind, n, edges, trial, repeats);
#if __has_include("reference.hpp")
			if (id == 10) run<Matching_reference>("reference_404900", kind, n, edges, trial, repeats);
#endif
		}
	}
}

int main(int argc, char **argv) {
	if (argc > 2 && std::string(argv[1]) == "--cases") {
		int trials = argc > 3 ? std::atoi(argv[3]) : 5, repeats = argc > 4 ? std::atoi(argv[4]) : 20;
		std::vector<std::filesystem::path> paths;
		for (auto &entry : std::filesystem::directory_iterator(argv[2])) if (entry.path().extension() == ".in") paths.push_back(entry.path());
		std::sort(paths.begin(), paths.end());
		std::cout << "variant,kind,n,m,trial,add_us,solve_us,total_us,size\n";
		std::mt19937 rng(612341);
		for (auto &path : paths) {
			std::ifstream in(path);
			int n, m;
			in >> n >> m;
			std::vector<std::pair<int, int>> edges(m);
			for (auto &[u, v] : edges) in >> u >> v;
			benchmark(path.stem().string(), n, edges, trials, repeats, rng);
		}
		return 0;
	}
	int n = argc > 1 ? std::atoi(argv[1]) : 500, trials = argc > 2 ? std::atoi(argv[2]) : 5, repeats = argc > 3 ? std::atoi(argv[3]) : 20;
	std::cout << "variant,kind,n,m,trial,add_us,solve_us,total_us,size\n";
	for (std::string kind : {"sparse", "dense", "complete", "odd_cliques", "star", "path", "tree", "bipartite", "bipartite_skew", "blossom_chain", "exposed_path"}) {
		std::mt19937 rng(846931);
		std::vector<std::pair<int, int>> edges;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) {
			bool add = false;
			if (kind == "sparse") add = rng() % n < 6;
			if (kind == "dense") add = rng() % 2;
			if (kind == "complete") add = true;
			if (kind == "odd_cliques") add = u / 9 == v / 9;
			if (kind == "star") add = u == 0;
			if (kind == "path" || kind == "exposed_path") add = v == u + 1;
			if (kind == "bipartite") add = u < n / 2 && v >= n / 2 && rng() % n < 12;
			if (kind == "bipartite_skew") add = u < n / 8 && v >= n / 8 && rng() % n < 32;
			if (kind == "blossom_chain") add = (u / 3 == v / 3) || (u % 3 == 2 && v == u + 1);
			if (add) edges.emplace_back(u, v);
		}
		if (kind == "tree") for (int u = 1; u < n; ++u) edges.emplace_back(u, rng() % u);
		std::vector<int> permutation(n);
		std::iota(permutation.begin(), permutation.end(), 0);
		std::shuffle(permutation.begin(), permutation.end(), rng);
		for (auto &[u, v] : edges) u = permutation[u], v = permutation[v];
		if (kind == "exposed_path") {
			std::stable_partition(edges.begin(), edges.end(), [&](auto e) {
				int pos = std::find(permutation.begin(), permutation.end(), e.first) - permutation.begin();
				return (pos & 1) && pos + 1 < n - 1;
			});
		} else std::shuffle(edges.begin(), edges.end(), rng);
		benchmark(kind, n, edges, trials, repeats, rng);
	}
	std::cerr << checksum << '\n';
}
