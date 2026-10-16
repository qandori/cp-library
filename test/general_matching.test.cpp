#define PROBLEM "https://judge.yosupo.jp/problem/general_matching"
#include "../graph/general_matching.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n, m;
	in.read(n);
	in.read(m);
	GeneralMatching g(n);
	g.reserve(m);
	for (int i = 0, u, v; i < m; ++i) {
		in.read(u);
		in.read(v);
		g.add_edge(u, v);
	}
	out.write(g.build());
	out.write('\n');
	for (auto [u, v] : g.edges()) {
		out.write(u);
		out.write(' ');
		out.write(v);
		out.write('\n');
	}
}
