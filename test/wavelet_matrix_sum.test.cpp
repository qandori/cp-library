#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum_with_upper_bound"
#include "../data_structure/wavelet_matrix.hpp"
#include "../components/fast_io.hpp"
#include <cstdint>
#include <vector>

using namespace std;

int main() {
	FastInput input;
	FastOutput output;
	uint32_t n = 0, q = 0;
	input.read(n); input.read(q);
	vector<int64_t> a(n);
	for (auto &value : a) input.read(value);
	WaveletMatrix wm(a, true);
	for (uint32_t i = 0; i < q; ++i) {
		uint32_t l = 0, r = 0;
		int64_t x = 0;
		input.read(l); input.read(r); input.read(x);
		int k = wm.range_freq(l, r, x) + wm.count(l, r, x);
		output.write(uint32_t(k)); output.write(' ');
		output.println(wm.sum_smallest(l, r, k));
	}
}
