#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_3_B"
#include "../graph/low_link.hpp"

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	cin >> n >> m;
	LowLink g(n);
	rep(m) {
		int u, v;
		cin >> u >> v;
		g.add_edge(u, v);
	}
	g.build();
	vec<pair<int, int>> answer;
	for (int id : g.bridges) {
		auto [u, v] = g.edges[id];
		answer.emplace_back(min(u, v), max(u, v));
	}
	sort(all(answer));
	for (auto [u, v] : answer) cout << u << ' ' << v << '\n';
}
