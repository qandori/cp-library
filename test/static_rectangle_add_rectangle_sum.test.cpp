#define PROBLEM "https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum"
#include "../data_structure/static_rectangle_add_rectangle_sum.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	StaticRectangleAddRectangleSum<mint, int> rectangles;
	for (int i = 0; i < n; ++i) {
		int l = 0, d = 0, r = 0, u = 0, w = 0;
		input.read(l); input.read(d); input.read(r); input.read(u); input.read(w);
		rectangles.add_rectangle(l, d, r, u, mint(w));
	}
	for (int i = 0; i < q; ++i) {
		int l = 0, d = 0, r = 0, u = 0;
		input.read(l); input.read(d); input.read(r); input.read(u);
		rectangles.add_query(l, d, r, u);
	}
	for (auto answer : rectangles.solve()) output.println(answer.val());
}
