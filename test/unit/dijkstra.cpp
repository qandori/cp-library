#include "graph/dijkstra.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using namespace std;

int main() {
	using I = int64_t;
	using G = vector<vector<pair<int, I>>>;
	mt19937_64 random(123);
	for (int trial = 0; trial < 500; ++trial) {
		int n = 1 + random() % 20, edges = random() % (n * n + 1);
		I inf = trial % 2 ? numeric_limits<I>::max() : DIJKSTRA_INF;
		G g(n);
		vector<vector<I>> expected(n, vector<I>(n, inf));
		for (int i = 0; i < n; ++i) expected[i][i] = 0;
		for (int repeat_index = 0; repeat_index < edges; ++repeat_index) {
			int v = random() % n, to = random() % n;
			I c = trial % 3 == 0 ? random() % 2 : trial % 3 == 1 ? random() % 1000
																 : random() >> 1;
			g[v].emplace_back(to, c);
			expected[v][to] = min(expected[v][to], c);
		}
		for (int k = 0; k < n; ++k)
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < n; ++j) {
					__int128_t sum = __int128_t(expected[i][k]) + expected[k][j];
					if (sum < expected[i][j]) expected[i][j] = I(sum);
				}
		for (int st = 0; st < n; ++st) {
			for (I &v : expected[st])
				if (v == inf) v = -1;
			assert(dijkstra(g, st, inf) == expected[st]);
		}
	}
	const vector<vector<pair<int, int>>> g = {{{1, 7}}, {}, {}};
	assert((dijkstra(g, 0) == vector<int>{0, 7, -1}));
	const vector<vector<pair<int, double>>> f = {{{1, 1.5}}, {{2, 2.25}}, {}, {}};
	assert((dijkstra(f, 0, 100.) == vector<double>{0, 1.5, 3.75, -1}));
	I maximum = numeric_limits<I>::max();
	const G huge = {{{1, maximum - 1}, {2, maximum}}, {{2, maximum}}, {}};
	assert((dijkstra(huge, 0, maximum) == vector<I>{0, maximum - 1, -1}));
	using Wide = __int128_t;
	using WideGraph = vector<vector<pair<int, Wide>>>;
	for (int trial = 0; trial < 150; ++trial) {
		int n = 1 + random() % 15;
		Wide inf = Wide(1) << 125;
		WideGraph graph(n);
		vector<vector<Wide>> expected(n, vector<Wide>(n, inf));
		for (int i = 0; i < n; ++i) expected[i][i] = 0;
		for (int e = random() % (n * n + 1); e--;) {
			int from = random() % n, to = random() % n;
			Wide weight = (Wide(random() >> 4) << 64) | random();
			if (trial % 3 == 0) weight = random() % 2;
			graph[from].emplace_back(to, weight);
			expected[from][to] = min(expected[from][to], weight);
		}
		for (int k = 0; k < n; ++k)
			for (int i = 0; i < n; ++i)
				for (int j = 0; j < n; ++j)
					expected[i][j] = min(expected[i][j], expected[i][k] + expected[k][j]);
		for (int source = 0; source < n; ++source) {
			for (auto &value : expected[source]) if (value == inf) value = -1;
			assert(dijkstra(graph, source, inf) == expected[source]);
		}
	}
	Wide inf = Wide(1) << 125;
	WideGraph wide = {{{1, inf - 1}, {2, inf}}, {{2, inf}}, {}, {}};
	assert((dijkstra(wide, 0, inf) == vector<Wide>{0, inf - 1, -1, -1}));
}
