#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_3_A"
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
	for (int v : g.articulation) cout << v << '\n';
}
