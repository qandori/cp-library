#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_3_A"
#include "../graph/low_link.hpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n, m;
	cin >> n >> m;
	LowLink g(n);
	for (int repeat_index = 0; repeat_index < m; ++repeat_index) {
		int u, v;
		cin >> u >> v;
		g.add_edge(u, v);
	}
	g.build();
	for (int v : g.articulation) cout << v << '\n';
}
