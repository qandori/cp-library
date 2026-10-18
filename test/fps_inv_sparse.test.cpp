#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_formal_power_series_sparse"
#include "fps_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, k = 0;
	input.read(n); input.read(k);
	fps a(n);
	for (int i = 0; i < k; ++i) {
		int degree = 0;
		unsigned value = 0;
		input.read(degree); input.read(value);
		a[degree] = fps_mint::raw(value);
	}
	write_fps(output, a.inv());
}
