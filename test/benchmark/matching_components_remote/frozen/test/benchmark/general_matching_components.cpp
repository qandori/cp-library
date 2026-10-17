#include "graph/general_matching.hpp"
#include "baseline.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

struct Case {
	std::string name;
	int n;
	std::vector<std::pair<int, int>> edges, extra;
	long long reset_bound = 0;
};
std::uint64_t checksum = 0;
double cpu() { return double(std::clock()) / CLOCKS_PER_SEC; }
template<class T> std::vector<int> solve(const Case &c) {
	T g(c.n);
	g.reserve(c.edges.size() + c.extra.size());
	for (auto [u, v] : c.edges) g.add_edge(u, v);
	g.build();
	if (!c.extra.empty()) {
		for (auto [u, v] : c.extra) g.add_edge(u, v);
		g.build();
	}
	return std::move(g.mate);
}
void valid(const Case &c, const std::vector<int> &mate) {
	std::set<std::pair<int, int>> edges;
	for (auto [u, v] : c.edges) edges.emplace(std::min(u, v), std::max(u, v));
	for (auto [u, v] : c.extra) edges.emplace(std::min(u, v), std::max(u, v));
	for (int u = 0; u < c.n; ++u) if (mate[u] != -1) {
		int v = mate[u];
		assert(v >= 0 && v < c.n && u != v && mate[v] == u);
		assert(edges.count({std::min(u, v), std::max(u, v)}));
	}
}
template<class T> std::pair<double, int> measure(const Case &c, const std::vector<int> &expected) {
	double elapsed = 0;
	int reps = 0;
	do {
		double begin = cpu();
		auto result = solve<T>(c);
		elapsed += cpu() - begin;
		assert(result == expected);
		std::uint64_t hash = 1469598103934665603ULL;
		for (int v : result) hash = (hash ^ std::uint64_t(v + 1)) * 1099511628211ULL;
		checksum += hash;
		++reps;
	} while (elapsed < 0.15);
	return {elapsed, reps};
}
Case stars(int n, int k, bool interleaved, bool nonbipartite = false) {
	int size = nonbipartite ? k : 2 * k, count = n / size;
	Case c{std::string(nonbipartite ? "deficient" : "double_star") + "_k" + std::to_string(k) + (interleaved ? "_interleaved" : ""), n, {}, {}};
	auto id = [&](int b, int j) { return interleaved ? b + j * count : b * size + j; };
	for (int b = 0; b < count; ++b) {
		if (nonbipartite) {
			for (int j = 1; j < size; ++j) c.edges.emplace_back(id(b, 0), id(b, j));
			c.edges.emplace_back(id(b, 1), id(b, 2));
		} else {
			for (int j = 0; j < k; ++j) c.edges.emplace_back(id(b, 0), id(b, k + j));
			for (int j = 1; j < k; ++j) c.edges.emplace_back(id(b, j), id(b, k));
		}
	}
	c.reset_bound = 1LL * n * count * (nonbipartite ? size - 4 : k - 2);
	return c;
}
std::vector<Case> cases(bool pilot) {
	std::vector<Case> all;
	for (int n : {1000, 10000, 100000}) for (int k : {4, 8, 32}) all.push_back(stars(n, k, false));
	all.push_back(stars(10000, 8, true));
	for (int size : {9, 10}) all.push_back(stars(10000, size, true, true));
	std::mt19937 rng(547139);
	for (std::string kind : {"triangles", "sparse", "dense", "exposed_path", "star", "tree", "bipartite_skew", "odd_cliques", "mixed", "rebuild"}) {
		int n = kind == "dense" ? 1000 : 10000;
		Case c{kind, n, {}, {}};
		if (kind == "triangles" || kind == "odd_cliques") {
			int size = kind == "triangles" ? 3 : 9;
			for (int b = 0; b + size <= n; b += size)
				for (int u = b; u < b + size; ++u) for (int v = u + 1; v < b + size; ++v) c.edges.emplace_back(u, v);
		} else if (kind == "sparse" || kind == "tree" || kind == "dense") {
			for (int u = 1; u < n; ++u) c.edges.emplace_back(u, rng() % u);
			if (kind == "sparse") for (int e = 0; e < 3 * n; ++e) c.edges.emplace_back(rng() % n, rng() % n);
			if (kind == "dense") for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) if (rng() & 1) c.edges.emplace_back(u, v);
		} else if (kind == "exposed_path") {
			for (int u = 1; u + 1 < n; u += 2) c.edges.emplace_back(u, u + 1);
			for (int u = 0; u + 1 < n; u += 2) c.edges.emplace_back(u, u + 1);
		} else if (kind == "star") {
			for (int u = 1; u < n; ++u) c.edges.emplace_back(0, u);
		} else if (kind == "bipartite_skew") {
			for (int u = n / 8; u < n; ++u) for (int j = 0; j < 3; ++j) c.edges.emplace_back(rng() % (n / 8), u);
		} else if (kind == "mixed") {
			c = stars(n / 2, 8, true); c.name = kind; c.n = n;
			for (int u = n / 2 + 1; u < n; ++u) c.edges.emplace_back(u, n / 2 + rng() % (u - n / 2));
		} else {
			c = stars(n, 8, true); c.name = kind;
			for (int u = 1; u < n; ++u) c.extra.emplace_back(u - 1, u);
		}
		all.push_back(std::move(c));
	}
	if (pilot) all.erase(std::remove_if(all.begin(), all.end(), [](const Case &c) {
		return c.n == 1000 && c.name != "dense";
	}), all.end());
	return all;
}
int main(int argc, char **argv) {
	bool pilot = argc > 1 && std::string(argv[1]) == "pilot";
	int pairs = pilot ? 2 : 6;
	std::cout << "case,n,edges,pair,order,variant,cpu_seconds,reps,seconds_per_solve,reset_bound\n" << std::setprecision(10);
	for (auto &c : cases(pilot)) {
		auto expected = solve<GeneralMatching>(c);
		valid(c, expected);
		bool capped = c.reset_bound > 1000000000LL;
		if (!capped) {
			double begin = cpu();
			auto reference = solve<BaselineMatching>(c);
			double elapsed = cpu() - begin;
			assert(reference == expected); valid(c, reference);
			if (elapsed > 5) { std::cerr << "CENSORED baseline >5 CPU seconds " << c.name << ' ' << c.n << '\n'; continue; }
		} else std::cerr << "OMITTED baseline reset bound >1e9 " << c.name << ' ' << c.n << ' ' << c.reset_bound << '\n';
		for (int pair = 0; pair < pairs; ++pair) {
			for (int leg = 0; leg < 2; ++leg) {
				bool candidate = (pair + leg) % 2;
				if (capped && !candidate) continue;
				auto [elapsed, reps] = candidate ? measure<GeneralMatching>(c, expected) : measure<BaselineMatching>(c, expected);
				std::cout << c.name << ',' << c.n << ',' << c.edges.size() << ',' << pair << ',' << (pair % 2 ? "BA" : "AB") << ',' << (candidate ? "candidate" : "baseline") << ',' << elapsed << ',' << reps << ',' << elapsed / reps << ',' << c.reset_bound << std::endl;
			}
		}
	}
	std::cerr << "checksum " << checksum << '\n';
}
