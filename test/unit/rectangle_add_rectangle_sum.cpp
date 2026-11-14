#include "data_structure/rectangle_add_rectangle_sum.hpp"
#include "data_structure/static_rectangle_add_rectangle_sum.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
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

template <class T, class Coordinate>
void separated_coordinates(std::vector<Coordinate> axis) {
	struct Rectangle { Coordinate l, d, r, u; T value; };
	std::sort(axis.begin(), axis.end());
	axis.erase(std::unique(axis.begin(), axis.end()), axis.end());
	std::vector<Coordinate> bounds = axis;
	for (std::size_t i = 1; i < axis.size(); ++i) bounds.push_back(std::midpoint(axis[i - 1], axis[i]));
	std::sort(bounds.begin(), bounds.end());
	bounds.erase(std::unique(bounds.begin(), bounds.end()), bounds.end());
	std::mt19937 rng(207395);
	auto rectangle = [&](const std::vector<Coordinate> &coordinates) {
		auto l = coordinates[rng() % coordinates.size()], r = coordinates[rng() % coordinates.size()];
		auto d = coordinates[rng() % coordinates.size()], u = coordinates[rng() % coordinates.size()];
		if (l > r) std::swap(l, r);
		if (d > u) std::swap(d, u);
		return Rectangle{l, d, r, u, T(int(rng() % 21) - 10)};
	};
	std::vector<Rectangle> updates, queries;
	RectangleAddRectangleSum<T, Coordinate> online;
	StaticRectangleAddRectangleSum<T, Coordinate> batch;
	for (int i = 0; i < 80; ++i) {
		auto r = rectangle(axis);
		if (i % 4 == 0) { r.l = axis[1]; r.r = axis[axis.size() - 2]; }
		updates.push_back(r);
		online.reserve(r.l, r.d, r.r, r.u);
	}
	online.build();
	auto expected = [&](const Rectangle &q, std::size_t count) {
		T result{};
		for (std::size_t i = 0; i < count; ++i) {
			const auto &r = updates[i];
			Coordinate l = std::max(q.l, r.l), d = std::max(q.d, r.d);
			Coordinate right = std::min(q.r, r.r), up = std::min(q.u, r.u);
			if (l < right && d < up) result += r.value * (T(right) - T(l)) * (T(up) - T(d));
		}
		return result;
	};
	for (std::size_t i = 0; i < updates.size(); ++i) {
		const auto &r = updates[i];
		online.add(r.l, r.d, r.r, r.u, r.value);
		batch.add_rectangle(r.l, r.d, r.r, r.u, r.value);
		for (int k = 0; k < 2; ++k) {
			auto q = rectangle(bounds);
			assert(online.sum(q.l, q.d, q.r, q.u) == expected(q, i + 1));
			Rectangle prefix{axis.front(), axis.front(), q.r, q.u, T{}};
			assert(online.prefix_sum(q.r, q.u) == expected(prefix, i + 1));
			batch.add_query(q.l, q.d, q.r, q.u);
			queries.push_back(q);
		}
	}
	auto answers = batch.solve();
	for (std::size_t i = 0; i < queries.size(); ++i) assert(answers[i] == expected(queries[i], updates.size()));
}

void extreme_query_ranges() {
	using C = std::int64_t;
	C lowest = std::numeric_limits<C>::min(), highest = std::numeric_limits<C>::max();
	RectangleAddRectangleSum<C, C> online;
	online.reserve(0, -4, 1, -3); online.build();
	online.add(0, -4, 1, -3, 1);
	assert(online.sum(0, lowest, 1, highest) == 1);
	assert(online.sum(0, -2, 1, highest) == 0);
	assert(online.sum(2, lowest, 3, highest) == 0);
	assert(online.sum(2, -2, 3, highest) == 0);
	online.add(0, -4, 1, -3, -1);
	assert(online.sum(0, lowest, 1, highest) == 0);
	for (bool cancel : {false, true}) {
		StaticRectangleAddRectangleSum<C, C> batch;
		for (int i = 0; i < 80; ++i) batch.add_rectangle(0, -4, 1, -3, cancel && i % 2 ? -1 : 1);
		for (int i = 0; i < 80; ++i) batch.add_query(i % 2 ? 2 : 0, i % 4 < 2 ? lowest : -2, i % 2 ? 3 : 1, highest);
		auto answers = batch.solve();
		for (int i = 0; i < 80; ++i) assert(answers[i] == (!cancel && i % 4 == 0 ? 80 : 0));
	}
	separated_coordinates<__int128_t, C>({-1000000000000LL, -100000001, -100003, -19, -4, 0, 1, 2, 3, 11, 999, 131072, 1000000000, 1000000000000LL});
	separated_coordinates<mint, C>({lowest, lowest + 1, lowest / 2, -100000003, -17, -1, 0, 1, 2, 17, 100000003, highest / 2, highest - 1, highest});
	std::vector<std::uint64_t> unsigned_axis{0, 1, 17, std::numeric_limits<std::uint64_t>::max()};
	for (int bit = 0; bit < 64; ++bit) unsigned_axis.push_back(std::uint64_t(1) << bit);
	separated_coordinates<mint>(unsigned_axis);
}

template <class T>
void small_value_types() {
	RectangleAddRectangleSum<T, int> online;
	StaticRectangleAddRectangleSum<T, int> batch;
	int lowest = std::numeric_limits<int>::min(), highest = std::numeric_limits<int>::max();
	std::vector<Rect> updates, queries;
	for (int i = 0; i < 80; ++i) {
		int x = highest - 500 + 3 * i, y = highest - 100 + 2 * (i % 7);
		long long value = std::numeric_limits<T>::is_signed && i % 2 ? -1 : 1;
		updates.push_back({x, y, x + 1, y + 1, value});
		online.reserve(x, y, x + 1, y + 1);
	}
	online.build();
	long long total = 0;
	for (const auto &r : updates) {
		online.add(r.l, r.d, r.r, r.u, T(r.w));
		batch.add_rectangle(r.l, r.d, r.r, r.u, T(r.w));
		total += r.w;
		assert(online.sum(r.l, r.d, r.r, r.u) == T(r.w));
		assert(online.sum(lowest, lowest, highest, highest) == T(total));
		assert(online.sum(lowest, lowest, 0, 0) == T{});
	}
	for (int i = 0; i < 100; ++i) {
		int l = i % 3 ? highest - 500 + 3 * (i % 80) : lowest;
		int r = i % 3 ? highest - 500 + 3 * ((i + 17) % 80) : highest;
		if (l > r) std::swap(l, r);
		queries.push_back({l, lowest, r, highest, 0});
		batch.add_query(l, lowest, r, highest);
	}
	auto answers = batch.solve();
	for (std::size_t i = 0; i < queries.size(); ++i) {
		long long expected = 0;
		for (const auto &r : updates) expected += overlap(r, queries[i]);
		assert(answers[i] == T(expected));
		const auto &q = queries[i];
		assert(online.sum(q.l, q.d, q.r, q.u) == T(expected));
	}
}

int main() {
	randomized(); empty_and_boundaries(); exhaustive_queries(); extreme_query_ranges();
	small_value_types<std::int8_t>(); small_value_types<std::uint8_t>();
	small_value_types<std::int16_t>(); small_value_types<std::uint16_t>();
}
