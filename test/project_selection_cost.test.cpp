#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=3058"
#include "flow/project_selection.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	std::string direction;
	input.read(n), input.read(m), input.read(direction);
	ProjectSelection<> problem(n);
	for (int v = 0; v < n; ++v) {
		int cost = 0;
		input.read(cost);
		problem.add_cost(v, direction[v] == 'R' ? cost : 0, direction[v] == 'L' ? cost : 0);
	}
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0, cost = 0;
		input.read(u), input.read(v), input.read(cost);
		--u, --v;
		if (u > v) std::swap(u, v);
		problem.add_cost_if(u, v, true, false, cost);
	}
	output.println(problem.solve()->cost);
}
