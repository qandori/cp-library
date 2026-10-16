#include "graph/general_matching.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

int brute(const std::vector<std::uint32_t> &adj) {
	int n = int(adj.size());
	std::vector<signed char> dp(1 << n, -1);
	dp[0] = 0;
	auto solve = [&](auto &&self, unsigned mask) -> int {
		if (dp[mask] != -1) return dp[mask];
		int u = std::countr_zero(mask);
		unsigned rest = mask ^ (1U << u), neighbors = adj[u] & rest;
		int answer = self(self, rest);
		while (neighbors) {
			unsigned bit = neighbors & -neighbors;
			neighbors ^= bit;
			answer = std::max(answer, 1 + self(self, rest ^ bit));
		}
		return dp[mask] = static_cast<signed char>(answer);
	};
	return solve(solve, (1U << n) - 1);
}
void check(GeneralMatching &g, const std::vector<std::vector<unsigned char>> &adj, int expected) {
	int n = int(adj.size());
	assert(g.build() == expected);
	assert(int(g.mate.size()) == n);
	int matched = 0;
	for (int u = 0; u < n; ++u) if (g.mate[u] != -1) {
		int v = g.mate[u];
		assert(0 <= v && v < n && u != v && adj[u][v] && g.mate[v] == u);
		++matched;
	}
	assert(matched == 2 * expected);
	auto pairs = g.edges();
	assert(int(pairs.size()) == expected);
	for (auto [u, v] : pairs) assert(u < v && g.mate[u] == v);
	assert(g.build() == expected);
}
int main() {
	for (int n = 0; n <= 6; ++n) {
		int m = n * (n - 1) / 2;
		for (unsigned mask = 0; mask < (1U << m); ++mask) {
			GeneralMatching g(n);
			std::vector<std::uint32_t> bits(n);
			std::vector<std::vector<unsigned char>> adj(n, std::vector<unsigned char>(n));
			int k = 0;
			for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v, ++k) if (mask >> k & 1) {
				g.add_edge(u, v);
				adj[u][v] = adj[v][u] = 1;
				bits[u] |= 1U << v;
				bits[v] |= 1U << u;
			}
			check(g, adj, brute(bits));
		}
	}
	std::mt19937 rng(917323);
	for (int n = 1; n <= 18; ++n) for (int trial = 0; trial < 120; ++trial) {
		GeneralMatching g(n);
		g.reserve(std::size_t(n) * n);
		std::vector<std::uint32_t> bits(n);
		std::vector<std::vector<unsigned char>> adj(n, std::vector<unsigned char>(n));
		std::vector<std::pair<int, int>> edges;
		unsigned density = rng() % 101;
		for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) if (rng() % 100 < density) edges.emplace_back(u, v);
		std::shuffle(edges.begin(), edges.end(), rng);
		for (auto [u, v] : edges) {
			g.add_edge(u, v);
			adj[u][v] = adj[v][u] = 1;
			bits[u] |= 1U << v;
			bits[v] |= 1U << u;
			if (trial < 8) check(g, adj, brute(bits));
			if (rng() % 5 == 0) g.add_edge(v, u);
		}
		for (int u = 0; u < n; ++u) g.add_edge(u, u);
		check(g, adj, brute(bits));
		auto copy = g;
		check(copy, adj, brute(bits));
	}
	for (int n : {31, 63, 64, 65, 127, 128, 129, 499, 500, 501, 1023, 1024, 1025, 4097}) {
		GeneralMatching g(n);
		std::vector<std::vector<unsigned char>> adj(n, std::vector<unsigned char>(n));
		for (int i = 1; i < n; ++i) {
			g.add_edge(i - 1, i);
			adj[i - 1][i] = adj[i][i - 1] = 1;
		}
		check(g, adj, n / 2);
	}
	for (int n : {32, 64, 128, 500, 1024, 2048, 4096}) {
		GeneralMatching g(n);
		std::vector<std::vector<unsigned char>> adj(n, std::vector<unsigned char>(n));
		auto add = [&](int u, int v) { g.add_edge(u, v); adj[u][v] = adj[v][u] = 1; };
		for (int i = 1; i + 1 < n; i += 2) add(i, i + 1);
		for (int i = 0; i + 1 < n; i += 2) add(i, i + 1);
		for (int i = 0; i + 2 < n; i += 3) add(i, i + 2);
		check(g, adj, n / 2);
	}

}
