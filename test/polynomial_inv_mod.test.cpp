#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_polynomials"
#include "fps/polynomial_gcd.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, m = 0;
	input.read(n), input.read(m);
	fps a(n), b(m);
	for (auto& x : a) { unsigned v = 0; input.read(v); x = v; }
	for (auto& x : b) { unsigned v = 0; input.read(v); x = v; }
	auto result = polynomial_inv_mod(std::move(a), std::move(b));
	if (!result) { output.println(-1); return 0; }
	output.println(result->size());
	for (int i = 0; i < int(result->size()); ++i) {
		if (i) output.write(' ');
		output.write((*result)[i].val());
	}
	output.write('\n');
}
