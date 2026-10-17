#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_formal_power_series"
#include "../convolution/online_convolution.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n;
	if (!in.read(n)) return 0;
	std::vector<mint> a(n);
	unsigned x = 0;
	for (auto &v : a) { in.read(x); v = mint::raw(x); }
	mint scale = a[0].inv(), value = scale;
	OnlineConvolution queue(n);
	for (int i = 0; i < n; ++i) {
		if (i) out.write(' ');
		out.write(value.val());
		if (i + 1 < n) value = -queue.push(a[i + 1], value) * scale;
	}
	out.write('\n');
}
