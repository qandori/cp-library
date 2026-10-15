#include "data_structure/li_chao_tree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <utility>
#include <vector>

using namespace std;

template <class T, bool maximize>
void random_test() {
	using Tree = LiChaoTree<T, maximize>;
	using R = typename Tree::value_type;
	mt19937_64 rng(7123);
	struct Line {
		T a, b;
		int l, r;
	};
	for (int trial = 0; trial < 180; ++trial) {
		vector<T> xs;
		int n = rng() % 70;
		for (int repeat_index = 0; repeat_index < n; ++repeat_index) xs.push_back(T(int(rng() % 81) - 40));
		Tree tree(xs);
		sort(xs.begin(), xs.end());
		xs.erase(unique(xs.begin(), xs.end()), xs.end());
		assert(tree.size() == static_cast<int>(xs.size()));
		for (int i = 0; i < (static_cast<int>(xs.size())); ++i) assert(tree.coordinate(i) == xs[i] && tree.index(xs[i]) == i);
		vector<Line> lines;
		for (int step = 0; step < 300; ++step) {
			int type = rng() % 20;
			T a = T(int(rng() % 41) - 20), b = T(int(rng() % 101) - 50);
			int l = 0, r = static_cast<int>(xs.size());
			if (type == 0) tree.clear(), lines.clear();
			else {
				if (type < 8) tree.add_line(a, b);
				else if (type < 14) {
					T left = T(int(rng() % 121) - 60), right = T(int(rng() % 121) - 60);
					if (right < left) swap(left, right);
					tree.add_segment(left, right, a, b);
					l = lower_bound(xs.begin(), xs.end(), left) - xs.begin();
					r = lower_bound(xs.begin(), xs.end(), right) - xs.begin();
				} else {
					l = rng() % (static_cast<int>(xs.size()) + 1), r = rng() % (static_cast<int>(xs.size()) + 1);
					if (r < l) swap(l, r);
					tree.add_segment_index(l, r, a, b);
				}
				lines.push_back({a, b, l, r});
			}
			for (int repeat_index = 0; repeat_index < 3; ++repeat_index) {
				if (xs.empty()) break;
				int i = rng() % static_cast<int>(xs.size());
				optional<R> expected;
				for (auto f : lines)
					if (f.l <= i && i < f.r) {
						R value = R(f.a) * R(xs[i]) + R(f.b);
						if (!expected || (maximize ? value > *expected : value < *expected)) expected = value;
					}
				assert(tree.query_index(i) == expected);
				assert(tree.query(xs[i]) == expected);
			}
		}
		tree.build({T(2), T(-1), T(2)});
		assert(tree.size() == 2 && !tree.query(T(2)));
		tree.add_segment(T(-1), T(2), T(3), T(1));
		assert(tree.query(T(-1)) == R(-2) && !tree.query(T(2)));
	}
}

template <bool maximize>
void extreme_test() {
	using T = int64_t;
	using R = __int128_t;
	const T lo = numeric_limits<T>::min(), hi = numeric_limits<T>::max();
	vector<T> xs{lo, lo + 1, -1000000000, -1, 0, 1, 1000000000, hi - 1, hi};
	LiChaoTree<T, maximize> tree(xs);
	vector<pair<T, T>> lines;
	for (T a : {lo, lo + 1, T(-1), T(0), T(1), hi - 1, hi})
		for (T b : {lo, lo + 1, T(-1), T(0), T(1), hi - 1, hi}) {
			tree.add_line(a, b);
			lines.emplace_back(a, b);
			for (int i = 0; i < (static_cast<int>(xs.size())); ++i) {
				R expected = R(lines[0].first) * xs[i] + lines[0].second;
				for (auto [c, d] : lines) {
					R value = R(c) * xs[i] + d;
					if (maximize ? value > expected : value < expected) expected = value;
				}
				assert(tree.query_index(i) == expected);
			}
		}
	tree.clear();
	tree.add_segment(lo, hi, hi, lo);
	assert(!tree.query(hi));
	assert(tree.query(lo) == R(hi) * lo + lo);
	tree.add_segment_index(static_cast<int>(xs.size()) - 1, static_cast<int>(xs.size()), lo, hi);
	assert(tree.query(hi) == R(lo) * hi + hi);
	tree.clear();
	struct Segment {
		T a, b;
		int l, r;
	};
	vector<Segment> segments;
	mt19937_64 rng(137);
	for (int repeat_index = 0; repeat_index < 3000; ++repeat_index) {
		T a = bit_cast<T>(rng()), b = bit_cast<T>(rng());
		int l = rng() % (static_cast<int>(xs.size()) + 1), r = rng() % (static_cast<int>(xs.size()) + 1);
		if (r < l) swap(l, r);
		tree.add_segment_index(l, r, a, b);
		segments.push_back({a, b, l, r});
		for (int i = 0; i < (static_cast<int>(xs.size())); ++i) {
			optional<R> expected;
			for (auto f : segments)
				if (f.l <= i && i < f.r) {
					R value = R(f.a) * xs[i] + f.b;
					if (!expected || (maximize ? value > *expected : value < *expected)) expected = value;
				}
			assert(tree.query_index(i) == expected);
		}
	}
}

template <bool maximize>
void arithmetic_test() {
	using T = int64_t;
	using R = __int128_t;
	const T lo = numeric_limits<T>::min(), hi = numeric_limits<T>::max();
	LiChaoTree<T, maximize> tree({1, 2});
	vector<pair<T, T>> lines;
	for (auto [a, b] : vector<pair<T, T>>{{hi, -hi}, {-hi, hi}, {0, hi}, {0, lo}, {hi, hi}, {lo, lo}}) {
		tree.add_line(a, b);
		lines.emplace_back(a, b);
		for (T x : {1, 2}) {
			R expected = R(lines[0].first) * x + lines[0].second;
			for (auto [c, d] : lines) {
				R value = R(c) * x + d;
				if (maximize ? value > expected : value < expected) expected = value;
			}
			assert(tree.query(x) == expected);
		}
	}
	tree.clear();
	assert(!tree.query(1));
	tree.add_segment(2, 3, hi, -hi);
	assert(!tree.query(1) && tree.query(2) == R(hi));
	tree.add_segment(2, 3, -hi, hi);
	assert(tree.query(2) == (maximize ? R(hi) : -R(hi)));
	for (T start : {lo, T(-5), hi - 15}) {
		for (int n = 1; n < 17; ++n) {
			vector<T> xs(n);
			for (int i = 0; i < n; ++i) xs[i] = start + i;
			tree.build(xs);
			for (int i = 0; i < n; ++i) assert(tree.index(xs[i]) == i);
			tree.add_segment(lo, hi, 0, 7);
			for (int i = 0; i < n; ++i) assert(tree.query_index(i) == (xs[i] < hi ? optional<R>(7) : nullopt));
		}
	}
}

int main() {
	random_test<int, false>();
	random_test<int, true>();
	random_test<int64_t, false>();
	random_test<int64_t, true>();
	random_test<double, false>();
	random_test<double, true>();
	extreme_test<false>();
	extreme_test<true>();
	arithmetic_test<false>();
	arithmetic_test<true>();
	LiChaoTree<> empty;
	empty.add_line(1, 2);
	empty.add_segment(-1, 1, 3, 4);
	empty.add_segment_index(0, 0, 1, 2);
	assert(empty.size() == 0);
	LiChaoTree<> singleton({7, 7});
	assert(!singleton.query(7));
	singleton.add_line(3, 4);
	singleton.add_line(3, 2);
	assert(singleton.query(7) == 23);
	auto copy = singleton;
	singleton.clear();
	assert(!singleton.query(7) && copy.query(7) == 23);
	LiChaoTree<int64_t, false, int64_t> narrow({-10, 0, 10});
	narrow.add_line(3, 4);
	assert(narrow.query(-10) == -26);
	LiChaoTree<double> floating({-0.5, 0, 0.5});
	floating.add_line(0.25, 0.125);
	assert(floating.query(-0.5) == 0 && floating.query(0.5) == 0.25);
	LiChaoTree<> large(vector<int64_t>(200000));
	vector<int64_t> xs(200000);
	iota(xs.begin(), xs.end(), -100000);
	large.build(xs);
	for (int i = 0; i < 200000; ++i) large.add_line(-2 * xs[i], xs[i] * xs[i]);
	for (int i = 0; i < 200000; ++i) assert(large.query_index(i) == -__int128_t(xs[i]) * xs[i]);
}
