#define PROBLEM "https://judge.yosupo.jp/problem/static_range_frequency"
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
	vector<uint32_t> a(n);
	for (auto &value : a) input.read(value);
	WaveletMatrix wm(a);
	wm.prepare_count();
	for (uint32_t i = 0; i < q; ++i) {
		uint32_t l = 0, r = 0, x = 0;
		input.read(l); input.read(r); input.read(x);
		output.println(uint32_t(wm.count(l, r, x)));
	}
}
