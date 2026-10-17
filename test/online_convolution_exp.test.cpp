#define PROBLEM "https://judge.yosupo.jp/problem/exp_of_formal_power_series"
#include "../convolution/online_convolution.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n;
	if (!in.read(n)) return 0;
	std::vector<mint> a(n), inverse(n, 1);
	unsigned x = 0;
	for (auto &v : a) { in.read(x); v = mint::raw(x); }
	for (int i = 2; i < n; ++i) inverse[i] = -mint(mint::mod() / i) * inverse[mint::mod() % i];
	OnlineConvolution queue(n);
	mint value = 1;
	for (int i = 0; i < n; ++i) {
		if (i) out.write(' ');
		out.write(value.val());
		if (i + 1 < n) value = queue.push(a[i + 1] * (i + 1), value) * inverse[i + 1];
	}
	out.write('\n');
}
