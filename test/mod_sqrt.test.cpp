#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_mod"
#include "math/mod_sqrt.hpp"
#include "components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int t = 0;
	input.read(t);
	while (t--) {
		std::uint64_t a = 0, p = 0;
		input.read(a), input.read(p);
		auto root = mod_sqrt(a, p);
		if (root) output.println(*root);
		else output.println(-1);
	}
}
