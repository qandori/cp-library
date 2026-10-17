#define PROBLEM "https://judge.yosupo.jp/problem/intersection_of_f2_vector_spaces"
#include "math/xor_basis.hpp"
#include "components/fast_io.hpp"
#include <cstdint>

int main() {
	FastInput input;
	FastOutput output;
	int t = 0; input.read(t);
	while (t--) {
		XorBasis<std::uint32_t, 30> a, b;
		int n = 0, m = 0; input.read(n);
		for (int i = 0; i < n; ++i) { std::uint32_t x = 0; input.read(x); a.insert(x); }
		input.read(m);
		for (int i = 0; i < m; ++i) { std::uint32_t x = 0; input.read(x); b.insert(x); }
		auto result = (a & b).vectors();
		output.write(result.size());
		for (auto x : result) { output.write(' '); output.write(x); }
		output.write('\n');
	}
}
