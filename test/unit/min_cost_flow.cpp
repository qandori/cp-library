#include "flow/min_cost_flow.hpp"
#include <functional>
#include <map>
#include <random>

template<class Cap, class Cost> void check() {
	std::mt19937_64 rng(3669);
	for (int trial = 0; trial < 800; ++trial) {
		int n = 2 + rng() % 5, m = rng() % 8;
		MinCostFlow<Cap, Cost> graph(n);
		graph.reserve(m);
		std::vector<int> potential(n);
		for (auto& p : potential) p = int(rng() % 21) - 10;
		for (int i = 0; i < m; ++i) {
			int u = rng() % n, v = rng() % n;
			Cap cap = rng() % 3;
			Cost cost = int(rng() % 8) + potential[v] - potential[u];
			assert(graph.add_edge(u, v, cap, cost) == i);
		}
		auto edges = graph.edges();
		std::vector<int> balance(n);
		std::map<int, long long> expected;
		std::function<void(int, long long)> enumerate = [&](int i, long long cost) {
			if (i == m) {
				for (int v = 1; v < n - 1; ++v) if (balance[v]) return;
				int f = balance[0];
				if (f < 0 || balance[n - 1] != -f) return;
				auto it = expected.find(f);
				if (it == expected.end() || cost < it->second) expected[f] = cost;
				return;
			}
			auto e = edges[i];
			for (int f = 0; f <= int(e.cap); ++f) {
				balance[e.from] += f, balance[e.to] -= f;
				enumerate(i + 1, cost + f * e.cost);
				balance[e.from] -= f, balance[e.to] += f;
			}
		};
		enumerate(0, 0);
		auto slope = graph.slope(0, n - 1);
		assert(slope.front().first == 0 && slope.front().second == 0);
		assert(int(slope.back().first) == expected.rbegin()->first && slope.back().second == expected.rbegin()->second);
		for (std::size_t i = 1; i < slope.size(); ++i) {
			auto [l, lc] = slope[i - 1];
			auto [r, rc] = slope[i];
			auto price = (rc - lc) / (r - l);
			for (int f = int(l); f <= int(r); ++f) assert(lc + (f - l) * price == expected[f]);
			if (i > 1) {
				auto [p, pc] = slope[i - 2];
				assert(price > (lc - pc) / (l - p));
			}
		}
		const auto& p = graph.potentials();
		for (auto e : graph.edges()) {
			auto c = e.cost + p[e.from] - p[e.to];
			if (e.flow < e.cap) assert(c >= 0);
			if (e.flow) assert(c <= 0);
		}
		graph.reset_flow();
		for (auto e : graph.edges()) assert(e.flow == 0);
		long long cost = 0;
		int flow = 0;
		while (true) {
			auto [f, c] = graph.flow(0, n - 1, 1);
			if (!f) break;
			cost += c, ++flow;
			assert(cost == expected[flow]);
		}
		assert(flow == expected.rbegin()->first);
		graph.reset_flow();
		graph.add_edge(0, n - 1, 1, 100);
		auto [f, c] = graph.flow(0, n - 1);
		assert(int(f) == flow + 1 && c == cost + 100);
	}
}

int main() {
	cp_acl::mcf_graph<int, long long> acl_graph(2);
	int edge_id = acl_graph.add_edge(0, 1, 4, 2);
	std::pair<int, long long> acl_result = acl_graph.flow(0, 1, 2);
	assert((acl_result == std::pair{2, 4LL}));
	const cp_acl::mcf_graph<int, long long>::edge edge = acl_graph.get_edge(edge_id);
	assert(edge.flow == 2);
	const std::vector<std::pair<int, long long>> slope = acl_graph.slope(0, 1);
	assert((slope == std::vector{std::pair{0, 0LL}, std::pair{2, 4LL}}));
	check<int, std::int8_t>();
	check<int, std::int16_t>();
	check<int, int>();
	check<std::int64_t, std::int64_t>();
	check<std::uint64_t, std::int64_t>();
	std::mt19937 rng(3668);
	for (int trial = 0; trial < 100; ++trial) {
		MinCostFlow<> graph(8), oracle(8);
		for (int i = 0; i < 80; ++i) {
			int u = rng() % 8, v = rng() % 8;
			int cap = rng() % 10000, cost = rng() % 100;
			graph.add_edge(u, v, cap, cost), oracle.add_edge(u, v, cap, cost);
		}
		auto edges = graph.edges();
		flow_detail::Graph<std::int64_t> network(8);
		std::vector<std::int64_t> costs;
		for (auto e : edges) network.add_edge(e.from, e.to, e.cap), costs.push_back(e.cost), costs.push_back(-e.cost);
		network.build();
		std::int64_t limit = rng() % 30000;
		auto scaled = flow_detail::scaling_flow<std::int64_t, std::int64_t, __int128_t>(network, costs, 0, 7, limit);
		assert(scaled == oracle.flow(0, 7, limit));
	}
	MinCostFlow<> many(2);
	for (int i = 0; i < 300; ++i) many.add_edge(0, 1, 1000000000, i);
	auto large = many.flow(0, 1);
	assert(large.first == 300000000000LL && large.second == __int128_t(299) * 300 / 2 * 1000000000);
	const auto& dual = many.potentials();
	for (auto e : many.edges()) assert(e.cost + dual[e.from] - dual[e.to] <= 0);
	MinCostFlow<> huge(2);
	huge.add_edge(0, 1, 1000000000000LL, 1000000000000LL);
	auto result = huge.flow(0, 1);
	assert(result.first == 1000000000000LL && result.second == __int128_t(1000000000000LL) * 1000000000000LL);
	MinCostFlow<int, std::int8_t> boundary(2);
	boundary.add_edge(0, 1, 1, 126);
	assert(boundary.flow(0, 1, 1).second == 126);
	boundary.add_edge(0, 1, 0, 0);
	assert(boundary.flow(1, 0, 1).second == -126);
	boundary.reset_flow();
	assert(boundary.flow(0, 1, 1).second == 126);
	MinCostFlow<int, std::int8_t> appended(6);
	appended.add_edge(0, 1, 1, -30);
	appended.add_edge(1, 2, 1, -30);
	appended.add_edge(2, 5, 1, -20);
	appended.add_edge(3, 4, 1, 20);
	appended.add_edge(4, 5, 1, 20);
	assert(appended.flow(0, 5, 1).second == -80);
	appended.add_edge(0, 3, 1, 50);
	auto appended_result = appended.flow(0, 5, 1);
	assert(appended_result.first == 1 && appended_result.second == 90);
	MinCostFlow<> zero(4);
	for (int i = 2; i >= 0; --i) zero.add_edge(i, i + 1, 2, -2);
	assert((zero.flow(0, 3, 0) == std::pair<std::int64_t, __int128_t>{0, 0}));
	assert((zero.slope(0, 3, 0) == std::vector<std::pair<std::int64_t, __int128_t>>{{0, 0}}));
	for (auto e : zero.edges()) assert(e.flow == 0);
	const auto& zero_potential = zero.potentials();
	for (auto e : zero.edges()) assert(e.cost + zero_potential[e.from] - zero_potential[e.to] >= 0);
	assert(zero.flow(0, 3, 1).second == -6);
	assert(zero.flow(0, 3, 0).first == 0 && zero.get_edge(0).flow == 1);
	zero.reset_flow();
	assert(zero.flow(0, 3, 1).second == -6);
	MinCostFlow<> disconnected(4);
	flow_detail::Graph<std::uint64_t> unsigned_graph(2);
	unsigned_graph.add_edge(0, 1, std::numeric_limits<std::uint64_t>::max());
	unsigned_graph.build();
	auto unsigned_result = flow_detail::scaling_flow<std::uint64_t, std::int64_t, __int128_t>(unsigned_graph, {2, -2}, 0, 1, std::numeric_limits<std::uint64_t>::max());
	assert(unsigned_result.first == std::numeric_limits<std::uint64_t>::max());
	assert(unsigned_result.second == __int128_t(std::numeric_limits<std::uint64_t>::max()) * 2);
	disconnected.add_edge(0, 1, 3, 5);
	disconnected.add_edge(2, 3, 7, -11);
	assert(disconnected.flow(0, 1).second == 15);
	assert(disconnected.flow(2, 3).second == -77);
	MinCostFlow<> empty;
	assert(empty.size() == 0 && empty.edge_count() == 0 && empty.potentials().empty());
}
