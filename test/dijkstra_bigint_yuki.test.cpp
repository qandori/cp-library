#define PROBLEM "https://yukicoder.me/problems/no/3669"
#include "graph/dijkstra.hpp"
#include "components/fast_io.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <numeric>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	struct Edge { int u, v, a, b; };
	std::vector<Edge> edges(m);
	int max_b = 1;
	for (auto& e : edges) input.read(e.u), input.read(e.v), input.read(e.a), input.read(e.b), max_b = std::max(max_b, e.b);
	using Integer = boost::multiprecision::cpp_int;
	Integer denominator = 1;
	for (int b = 2; b <= max_b; ++b) denominator *= b / std::gcd(b, int(Integer(denominator % b)));
	std::vector<std::vector<std::pair<int, Integer>>> graph(n);
	std::vector<Integer> unit(max_b + 1);
	for (int b = 1; b <= max_b; ++b) unit[b] = denominator / b;
	for (auto e : edges) {
		Integer weight = unit[e.b] * e.a;
		graph[e.u - 1].emplace_back(e.v - 1, weight);
		graph[e.v - 1].emplace_back(e.u - 1, std::move(weight));
	}
	auto distance = dijkstra(graph, 0, Integer(denominator * (300LL * n + 1)));
	for (int v = 1; v < n; ++v) {
		Integer divisor = gcd(distance[v], denominator);
		output.write(Integer(distance[v] / divisor).str());
		output.write(' ');
		output.println(Integer(denominator / divisor).str());
	}
}
