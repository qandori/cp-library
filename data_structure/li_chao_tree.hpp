#ifndef LI_CHAO_TREE_HPP
#define LI_CHAO_TREE_HPP

#include "../template.hpp"

template <class T = int64_t, bool maximize = false, class R = conditional_t<is_integral_v<T>, conditional_t<(sizeof(T) < 8), int64_t, __int128_t>, T>>
class LiChaoTree {
	static_assert((is_integral_v<T> && is_signed_v<T> && sizeof(T) <= 8) || is_floating_point_v<T>);
	struct Line {
		T a, b;
		template <class V>
		V eval(T x) const { return V(R(a) * R(x) + R(b)); }
	};
	int n = 0, base = 1;
	bool wide = false, dense = false;
	vec<T> xs;
	vec<Line> lines;
	vec<uint8_t> state;
	template <class V>
	static bool better(V a, V b) {
		if constexpr (maximize) return a > b;
		else return a < b;
	}
	template <class V>
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
			if (better(f.template eval<V>(xs[m]), g.template eval<V>(xs[m]))) swap(f, g);
			if (!half) return;
			bool right = better(f.a, g.a);
			int edge = right ? l + width - 1 : l;
			if (!better(f.template eval<V>(xs[edge]), g.template eval<V>(xs[edge]))) return;
			k = k * 2 + right;
			if (right) l = m;
			width = half;
		}
	}
	template <class V>
	void insert_segment(int k, int l, int r, int ql, int qr, Line f) {
		if (ql <= l && r <= qr) {
			insert<V>(k, l, r - l, f);
			return;
		}
		if (state[k] & 1) {
			const Line &g = lines[k];
			if (f.a == g.a) {
				if (!better(f.b, g.b)) return;
			} else {
				int edge = better(f.a, g.a) ? min(r, qr) - 1 : max(l, ql);
				if (!better(f.template eval<V>(xs[edge]), g.template eval<V>(xs[edge]))) return;
			}
		}
		state[k] |= 2;
		int m = (l + r) >> 1;
		if (ql < m) insert_segment<V>(k * 2, l, m, ql, qr, f);
		if (m < qr) insert_segment<V>(k * 2 + 1, m, r, ql, qr, f);
	}

	void check(Line f, int l, int r) {
		if constexpr (is_integral_v<T> && is_same_v<R, __int128_t>) {
			if (!wide) {
				R a = f.template eval<R>(xs[l]), b = f.template eval<R>(xs[r - 1]);
				wide = a != R(int64_t(a)) || b != R(int64_t(b));
			}
		}
	}
	template <class F>
	decltype(auto) dispatch(F f) const {
		if constexpr (is_integral_v<T> && is_same_v<R, __int128_t>) {
			if (!wide) return f.template operator()<int64_t>();
		}
		return f.template operator()<R>();
	}

	int lower(T x) const {
		if (n <= 1) return n && xs[0] < x;
		if constexpr (is_integral_v<T>)
			if (dense) return int(clamp<__int128_t>(__int128_t(x) - xs[0], 0, n));
		int i = 0;
		for (int step = base >> 1; step; step >>= 1) i += int(xs[i + step - 1] < x) * step;
		return min(n, i + int(xs[i] < x));
	}

	template <class V>
	optional<R> query_impl(int i) const {
		assert(0 <= i && i < n);
		V result = maximize ? numeric_limits<V>::lowest() : numeric_limits<V>::max();
		bool found = false;
		for (int k = 1, bit = base >> 1;; bit >>= 1) {
			if (!state[k]) break;
			if (state[k] & 1) {
				V value = lines[k].template eval<V>(xs[i]);
				if ((is_floating_point_v<V> && !found) || better(value, result)) result = value;
				found = true;
			}
			if (!bit) break;
			k = k * 2 + bool(i & bit);
		}
		if (found) return result;
		return nullopt;
	}

  public:
	using value_type = R;
	explicit LiChaoTree(vec<T> coordinates = {}) { build(std::move(coordinates)); }
	void build(vec<T> coordinates) {
		if (!is_sorted(all(coordinates))) sort(all(coordinates));
		coordinates.erase(unique(all(coordinates)), coordinates.end());
		assert(coordinates.size() <= size_t(1) << 29);
		n = sz(coordinates);
		if constexpr (is_integral_v<T>) dense = n && __int128_t(coordinates.back()) - coordinates[0] == n - 1;
		base = int(bit_ceil(unsigned(max(1, n))));
		xs = std::move(coordinates);
		if (n) xs.resize(base, xs.back());
		lines.resize(base * 2);
		state.assign(base * 2, 0);
		wide = false;
	}
	int size() const { return n; }
	void clear() {
		fill(all(state), 0);
		wide = false;
	}
	T coordinate(int i) const {
		assert(0 <= i && i < n);
		return xs[i];
	}
	int index(T x) const {
		int i = lower(x);
		assert(i < n && xs[i] == x);
		return i;
	}
	void add_line(T a, T b) {
		if (n) {
			Line f{a, b};
			check(f, 0, n);
			dispatch([&]<class V>() { insert<V>(1, 0, base, f); });
		}
	}
	void add_segment_index(int l, int r, T a, T b) {
		assert(0 <= l && l <= r && r <= n);
		if (l < r) {
			Line f{a, b};
			check(f, l, r);
			dispatch([&]<class V>() { insert_segment<V>(1, 0, base, l, r == n ? base : r, f); });
		}
	}
	void add_segment(T l, T r, T a, T b) {
		assert(l <= r);
		int left = lower(l);
		int right = lower(r);
		add_segment_index(left, right, a, b);
	}
	optional<R> query_index(int i) const {
		return dispatch([&]<class V>() { return query_impl<V>(i); });
	}
	optional<R> query(T x) const { return query_index(index(x)); }
};

#endif
