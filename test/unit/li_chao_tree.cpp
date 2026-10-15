#include "data_structure/li_chao_tree.hpp"

template <class T, bool maximize>
void random_test() {
	using Tree = LiChaoTree<T, maximize>;
	using R = typename Tree::value_type;
	mt19937_64 rng(7123);
	struct Line {
		T a, b;
		int l, r;
	};
	rep(trial, 180) {
		vec<T> xs;
		int n = rng() % 70;
		rep(n) xs.push_back(T(int(rng() % 81) - 40));
		Tree tree(xs);
		sort(all(xs));
		xs.erase(unique(all(xs)), xs.end());
		assert(tree.size() == sz(xs));
		rep(i, sz(xs)) assert(tree.coordinate(i) == xs[i] && tree.index(xs[i]) == i);
		vec<Line> lines;
		rep(step, 300) {
			int type = rng() % 20;
			T a = T(int(rng() % 41) - 20), b = T(int(rng() % 101) - 50);
			int l = 0, r = sz(xs);
			if (type == 0) tree.clear(), lines.clear();
			else {
				if (type < 8) tree.add_line(a, b);
				else if (type < 14) {
					T left = T(int(rng() % 121) - 60), right = T(int(rng() % 121) - 60);
					if (right < left) swap(left, right);
					tree.add_segment(left, right, a, b);
					l = lower_bound(all(xs), left) - xs.begin();
					r = lower_bound(all(xs), right) - xs.begin();
				} else {
					l = rng() % (sz(xs) + 1), r = rng() % (sz(xs) + 1);
					if (r < l) swap(l, r);
					tree.add_segment_index(l, r, a, b);
				}
				lines.push_back({a, b, l, r});
			}
			rep(3) {
				if (xs.empty()) break;
				int i = rng() % sz(xs);
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
	vec<T> xs{lo, lo + 1, -1000000000, -1, 0, 1, 1000000000, hi - 1, hi};
	LiChaoTree<T, maximize> tree(xs);
	vec<pair<T, T>> lines;
	for (T a : {lo, lo + 1, T(-1), T(0), T(1), hi - 1, hi})
		for (T b : {lo, lo + 1, T(-1), T(0), T(1), hi - 1, hi}) {
			tree.add_line(a, b);
			lines.emplace_back(a, b);
			rep(i, sz(xs)) {
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
	tree.add_segment_index(sz(xs) - 1, sz(xs), lo, hi);
	assert(tree.query(hi) == R(lo) * hi + hi);
	tree.clear();
	struct Segment {
		T a, b;
		int l, r;
	};
	vec<Segment> segments;
	mt19937_64 rng(137);
	rep(3000) {
		T a = bit_cast<T>(rng()), b = bit_cast<T>(rng());
		int l = rng() % (sz(xs) + 1), r = rng() % (sz(xs) + 1);
		if (r < l) swap(l, r);
		tree.add_segment_index(l, r, a, b);
		segments.push_back({a, b, l, r});
		rep(i, sz(xs)) {
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
	vec<pair<T, T>> lines;
	for (auto [a, b] : vec<pair<T, T>>{{hi, -hi}, {-hi, hi}, {0, hi}, {0, lo}, {hi, hi}, {lo, lo}}) {
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
		rep(n, 1, 17) {
			vec<T> xs(n);
			rep(i, n) xs[i] = start + i;
			tree.build(xs);
			rep(i, n) assert(tree.index(xs[i]) == i);
			tree.add_segment(lo, hi, 0, 7);
			rep(i, n) assert(tree.query_index(i) == (xs[i] < hi ? optional<R>(7) : nullopt));
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
	LiChaoTree<> large(vec<int64_t>(200000));
	vec<int64_t> xs(200000);
	iota(all(xs), -100000);
	large.build(xs);
	rep(i, 200000) large.add_line(-2 * xs[i], xs[i] * xs[i]);
	rep(i, 200000) assert(large.query_index(i) == -__int128_t(xs[i]) * xs[i]);
}
