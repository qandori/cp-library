#ifndef LI_CHAO_TREE_HPP
#define LI_CHAO_TREE_HPP

#include "../template.hpp"

template <class T = int64_t, bool maximize = false, class R = conditional_t<is_integral_v<T>, conditional_t<(sizeof(T) < 8), int64_t, __int128_t>, T>>
class LiChaoTree {
	static_assert((is_integral_v<T> && is_signed_v<T> && sizeof(T) <= 8) || is_floating_point_v<T>);
	struct Line {
		T a, b;
		R eval(T x) const { return R(a) * R(x) + R(b); }
	};
	int n = 0, base = 1;
	vec<T> xs;
	vec<Line> lines;
	vec<uint8_t> state;
	template <class V>
	static bool better(V a, V b) {
		if constexpr (maximize) return a > b;
		else return a < b;
	}
	void insert(int k, int l, int width, Line f) {
		for (;;) {
			Line &g = lines[k];
			if (!(state[k] & 1)) {
				g = f;
				state[k] = 3;
				return;
			}
			if (f.a == g.a) {
				if (better(f.b, g.b)) g = f;
				return;
			}
			int half = width >> 1, m = l + half;
			if (better(f.eval(xs[m]), g.eval(xs[m]))) swap(f, g);
			if (!half) return;
			bool right = better(f.a, g.a);
			int edge = right ? l + width - 1 : l;
			if (!better(f.eval(xs[edge]), g.eval(xs[edge]))) return;
			k = k * 2 + right;
			if (right) l = m;
			width = half;
		}
	}
	void insert_segment(int k, int l, int r, int ql, int qr, Line f) {
		if (ql <= l && r <= qr) {
			insert(k, l, r - l, f);
			return;
		}
		if (state[k] & 1) {
			const Line &g = lines[k];
			if (f.a == g.a) {
				if (!better(f.b, g.b)) return;
			} else {
				int edge = better(f.a, g.a) ? min(r, qr) - 1 : max(l, ql);
				if (!better(f.eval(xs[edge]), g.eval(xs[edge]))) return;
			}
		}
		state[k] |= 2;
		int m = (l + r) >> 1;
		if (ql < m) insert_segment(k * 2, l, m, ql, qr, f);
		if (m < qr) insert_segment(k * 2 + 1, m, r, ql, qr, f);
	}

  public:
	using value_type = R;
	explicit LiChaoTree(vec<T> coordinates = {}) { build(std::move(coordinates)); }
	void build(vec<T> coordinates) {
		sort(all(coordinates));
		coordinates.erase(unique(all(coordinates)), coordinates.end());
		assert(coordinates.size() <= size_t(1) << 29);
		n = sz(coordinates);
		base = int(bit_ceil(unsigned(max(1, n))));
		xs = std::move(coordinates);
		if (n) xs.resize(base, xs.back());
		lines.resize(base * 2);
		state.assign(base * 2, 0);
	}
	int size() const { return n; }
	void clear() { fill(all(state), 0); }
	T coordinate(int i) const {
		assert(0 <= i && i < n);
		return xs[i];
	}
	int index(T x) const {
		int i = lower_bound(xs.begin(), xs.begin() + n, x) - xs.begin();
		assert(i < n && xs[i] == x);
		return i;
	}
	void add_line(T a, T b) {
		if (n) insert(1, 0, base, {a, b});
	}
	void add_segment_index(int l, int r, T a, T b) {
		assert(0 <= l && l <= r && r <= n);
		if (l < r) insert_segment(1, 0, base, l, r == n ? base : r, {a, b});
	}
	void add_segment(T l, T r, T a, T b) {
		assert(l <= r);
		int left = lower_bound(xs.begin(), xs.begin() + n, l) - xs.begin();
		int right = lower_bound(xs.begin() + left, xs.begin() + n, r) - xs.begin();
		add_segment_index(left, right, a, b);
	}
	optional<R> query_index(int i) const {
		assert(0 <= i && i < n);
		R result = maximize ? numeric_limits<R>::lowest() : numeric_limits<R>::max();
		bool found = false;
		for (int k = 1, bit = base >> 1;; bit >>= 1) {
			if (!state[k]) break;
			if (state[k] & 1) {
				R value = lines[k].eval(xs[i]);
				if (!found || better(value, result)) result = value;
				found = true;
			}
			if (!bit) break;
			k = k * 2 + bool(i & bit);
		}
		if (found) return result;
		return nullopt;
	}
	optional<R> query(T x) const { return query_index(index(x)); }
};

#endif
