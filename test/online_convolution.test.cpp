#define PROBLEM "https://judge.yosupo.jp/problem/convolution_mod"
#include "../convolution/online_convolution.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput in;
	FastOutput out;
	int n, m = 0;
	if (!in.read(n)) return 0;
	in.read(m);
	std::vector<mint> a(n), b(m);
	unsigned x = 0;
	for (auto &v : a) { in.read(x); v = mint::raw(x); }
	for (auto &v : b) { in.read(x); v = mint::raw(x); }
	OnlineConvolution queue(n + m - 1);
	for (int i = 0; i < n + m - 1; ++i) {
		if (i) out.write(' ');
		out.write(queue.push(i < n ? a[i] : mint(0), i < m ? b[i] : mint(0)).val());
	}
	out.write('\n');
}
