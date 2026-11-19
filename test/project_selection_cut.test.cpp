#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_6_A"
#include "flow/project_selection.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	ProjectSelection<> problem(n);
	problem.force(0, true);
	problem.force(n - 1, false);
	for (int i = 0; i < m; ++i) {
		int u = 0, v = 0;
		std::int64_t cap = 0;
		input.read(u), input.read(v), input.read(cap);
		problem.add_cost_if(u, v, true, false, cap);
	}
	output.println(problem.solve()->cost);
}
