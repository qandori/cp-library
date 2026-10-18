#define PROBLEM "https://judge.yosupo.jp/problem/range_kth_smallest"
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
	for (uint32_t i = 0; i < q; ++i) {
		uint32_t l = 0, r = 0, k = 0;
		input.read(l); input.read(r); input.read(k);
		output.println(wm.kth_smallest(l, r, k));
	}
}
