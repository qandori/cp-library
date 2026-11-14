#include "data_structure/rectangle_add_rectangle_sum.hpp"
#include "data_structure/static_rectangle_add_rectangle_sum.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

struct Rect { int l, d, r, u; long long w; };

long long overlap(const Rect &a, const Rect &b) {
	return a.w * std::max(0, std::min(a.r, b.r) - std::max(a.l, b.l)) * std::max(0, std::min(a.u, b.u) - std::max(a.d, b.d));
}

void randomized() {
	std::mt19937 rng(817302);
	for (int trial = 0; trial < 220; ++trial) {
		RectangleAddRectangleSum<> online;
		StaticRectangleAddRectangleSum<> batch;
		StaticRectangleAddRectangleSum<mint, int> modular;
		std::vector<Rect> updates(5 + rng() % 110), queries;
		for (auto &r : updates) {
			r = {int(rng() % 19) - 9, int(rng() % 19) - 9, int(rng() % 19) - 9, int(rng() % 19) - 9, int(rng() % 101) - 50};
			if (r.l > r.r) std::swap(r.l, r.r);
			if (r.d > r.u) std::swap(r.d, r.u);
			online.reserve(r.l, r.d, r.r, r.u);
			online.reserve(r.l, r.d, r.r, r.u);
		}
		online.build();
		std::array<std::array<long long, 20>, 20> cells{};
		for (const auto &r : updates) {
			online.add(r.l, r.d, r.r, r.u, r.w);
			batch.add_rectangle(r.l, r.d, r.r, r.u, r.w);
			modular.add_rectangle(r.l, r.d, r.r, r.u, r.w);
			for (int x = r.l; x < r.r; ++x) for (int y = r.d; y < r.u; ++y) cells[x + 10][y + 10] += r.w;
			for (int k = 0; k < 3; ++k) {
				Rect q{int(rng() % 21) - 10, int(rng() % 21) - 10, int(rng() % 21) - 10, int(rng() % 21) - 10, 0};
				if (q.l > q.r) std::swap(q.l, q.r);
				if (q.d > q.u) std::swap(q.d, q.u);
				long long expected = 0;
				for (int x = q.l; x < q.r; ++x) for (int y = q.d; y < q.u; ++y) expected += cells[x + 10][y + 10];
				assert(online.sum(q.l, q.d, q.r, q.u) == expected);
				assert(online.prefix_sum(q.r, q.u) == online.sum(-20, -20, q.r, q.u));
				assert(batch.add_query(q.l, q.d, q.r, q.u) == int(queries.size()));
				modular.add_query(q.l, q.d, q.r, q.u);
				queries.push_back(q);
			}
		}
		auto answers = batch.solve();
		auto residues = modular.solve();
		assert(batch.solve() == answers);
		for (std::size_t i = 0; i < queries.size(); ++i) {
			long long expected = 0;
			for (const auto &r : updates) expected += overlap(r, queries[i]);
			assert(answers[i] == expected && residues[i] == mint(expected));
			const auto &q = queries[i];
			assert(online.sum(q.l, q.d, q.r, q.u) == expected);
		}
		auto copy = online;
		online.clear_values();
		assert(online.sum(-20, -20, 20, 20) == 0);
		for (const auto &r : updates) online.add(r.l, r.d, r.r, r.u, r.w);
		assert(online.sum(-20, -20, 20, 20) == copy.sum(-20, -20, 20, 20));
		online.build();
		assert(online.sum(-20, -20, 20, 20) == 0);
	}
}

void empty_and_boundaries() {
	RectangleAddRectangleSum<> empty;
	empty.reserve(0, 0, 0, 10); empty.build();
	assert(empty.sum(-10, -10, 10, 10) == 0);
	empty.add(3, 2, 3, 5, 100);
	empty.clear_values(); empty.build();
	StaticRectangleAddRectangleSum<> batch;
	assert(batch.solve().empty());
	assert(batch.add_query(0, 0, 1, 1) == 0);
	assert(batch.solve() == std::vector<std::int64_t>{0});
	batch.add_rectangle(0, 0, 1, 1, 9);
	assert(batch.solve() == std::vector<std::int64_t>{9});
	batch.add_rectangle(0, 0, 1, 1, -9);
	assert(batch.solve() == std::vector<std::int64_t>{0});
	RectangleAddRectangleSum<__int128_t> wide;
	long long l = std::numeric_limits<long long>::min();
	wide.reserve(l, -3, l + 7, 5); wide.build();
	wide.add(l, -3, l + 7, 5, __int128_t(1000000000000LL));
	assert(wide.sum(l, -3, l + 7, 5) == __int128_t(56000000000000LL));
	assert(wide.sum(l + 1, -2, l + 6, 4) == __int128_t(30000000000000LL));
	using U = std::uint64_t;
	U last = std::numeric_limits<U>::max();
	RectangleAddRectangleSum<mint, U> modular;
	StaticRectangleAddRectangleSum<mint, U> static_modular;
	modular.reserve(0, 0, last, last); modular.build();
	for (int i = 0; i < 100; ++i) {
		modular.add(0, 0, last, last, mint(i - 7));
		static_modular.add_rectangle(0, 0, last, last, mint(i - 7));
		static_modular.add_query(1, 2, last, last);
	}
	mint expected = mint(4250) * (mint(last) - 1) * (mint(last) - 2);
	assert(modular.sum(1, 2, last, last) == expected);
	for (mint answer : static_modular.solve()) assert(answer == expected);
}

void exhaustive_queries() {
	RectangleAddRectangleSum<> tree;
	std::vector<Rect> rectangles;
	for (int l = -2; l <= 2; ++l) for (int r = l; r <= 2; ++r)
		for (int d = -2; d <= 2; ++d) for (int u = d; u <= 2; ++u) {
			tree.reserve(l, d, r, u);
			rectangles.push_back({l, d, r, u, (l + r + d + u) % 3});
		}
	tree.build();
	for (const auto &r : rectangles) tree.add(r.l, r.d, r.r, r.u, r.w);
	for (int l = -3; l <= 3; ++l) for (int r = l; r <= 3; ++r)
		for (int d = -3; d <= 3; ++d) for (int u = d; u <= 3; ++u) {
			long long expected = 0;
			for (const auto &a : rectangles) expected += overlap(a, Rect{l, d, r, u, 0});
			assert(tree.sum(l, d, r, u) == expected);
		}
}

int main() { randomized(); empty_and_boundaries(); exhaustive_queries(); }
