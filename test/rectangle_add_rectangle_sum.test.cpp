#define PROBLEM "https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum"
#include "../data_structure/rectangle_add_rectangle_sum.hpp"
#include "../math/mint.hpp"
#include "../components/fast_io.hpp"
#include <array>

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	RectangleAddRectangleSum<mint, int> rectangles;
	std::vector<std::array<int, 5>> updates(n);
	for (auto &r : updates) {
		for (int &value : r) input.read(value);
		rectangles.reserve(r[0], r[1], r[2], r[3]);
	}
	rectangles.build();
	for (const auto &r : updates) rectangles.add(r[0], r[1], r[2], r[3], mint(r[4]));
	for (int i = 0; i < q; ++i) {
		int l = 0, d = 0, r = 0, u = 0;
		input.read(l); input.read(d); input.read(r); input.read(u);
		output.println(rectangles.sum(l, d, r, u).val());
	}
}
