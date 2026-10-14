#include "data_structure/wavelet_matrix.hpp"
#include "data_structure/wavelet_matrix.hpp"

void check(const vec<int64_t> &a) {
	WaveletMatrix wm(a);
	assert(wm.size() == sz(a) && wm.empty() == a.empty());
	rep(i, sz(a)) assert(wm[i] == a[i] && wm.access(i) == a[i]);
	map<int64_t, vec<int>> positions;
	rep(i, sz(a)) positions[a[i]].push_back(i);
	for (const auto &[x, indices] : positions) {
		rep(k, sz(indices)) assert(wm.select(x, k) == indices[k]);
		assert(wm.select(x, -1) == -1 && wm.select(x, sz(indices)) == -1);
	}
	mt19937_64 random(123);
	rep(200) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (l > r) swap(l, r);
		int64_t x = int(random() % 41) - 20, y = int(random() % 41) - 20;
		if (random() % 10 == 0) x = numeric_limits<int64_t>::min();
		if (random() % 10 == 0) x = numeric_limits<int64_t>::max();
		vec<int64_t> sorted(a.begin() + l, a.begin() + r);
		sort(all(sorted));
		int below = lower_bound(all(sorted), x) - sorted.begin();
		int exact = std::count(all(sorted), x);
		assert(wm.count(l, r, x) == exact);
		assert(wm.rank(x, r) == std::count(a.begin(), a.begin() + r, x));
		assert(wm.range_freq(l, r, x) == below);
		int between = 0;
		for (int64_t v : sorted) between += x <= v && v < y;
		assert(wm.range_freq(l, r, x, y) == between);
		optional<int64_t> previous, next;
		if (below) previous = sorted[below - 1];
		if (below < sz(sorted)) next = sorted[below];
		assert(wm.prev_value(l, r, x) == previous && wm.next_value(l, r, x) == next);
		if (!sorted.empty()) {
			int k = random() % sorted.size();
			assert(wm.kth_smallest(l, r, k) == sorted[k]);
			assert(wm.kth_largest(l, r, k) == sorted[sorted.size() - 1 - k]);
		}
	}
	wm.build({}); assert(wm.empty() && wm.count(0, 0, 1) == 0);
	wm.build(a); rep(i, sz(a)) assert(wm[i] == a[i]);
}

int main() {
	WaveletMatrix<> empty;
	assert(empty.empty() && empty.select(1, 0) == -1);
	assert(!empty.prev_value(0, 0, 0) && !empty.next_value(0, 0, 0));
	check({}); check({7}); check(vec<int64_t>(129, -3));
	check({numeric_limits<int64_t>::min(), numeric_limits<int64_t>::max(), -1, 0, 1});
	mt19937_64 random(456);
	for (int n : {2, 3, 63, 64, 65, 127, 128, 129, 1000}) {
		vec<int64_t> a(n);
		rep(i, n) a[i] = int(random() % 21) - 10;
		check(a);
		iota(all(a), -500); shuffle(all(a), random); check(a);
	}
	rep(200) {
		vec<int64_t> a(random() % 80);
		for (auto &v : a) v = int64_t(random());
		check(a);
	}
	vec<uint64_t> unsigned_values{0, numeric_limits<uint64_t>::max(), 1};
	WaveletMatrix u(unsigned_values);
	assert(u.kth_smallest(0, 3, 2) == numeric_limits<uint64_t>::max());
	assert(u.range_freq(0, 3, numeric_limits<uint64_t>::max()) == 2);
	WaveletMatrix<string> words(vec<string>{"b", "a", "b", "z"});
	assert(words.kth_smallest(0, 4, 1) == "b" && words.select("b", 1) == 2);
	assert(words.prev_value(0, 4, "b") == optional<string>("a"));
}
