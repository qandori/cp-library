#ifndef WAVELET_MATRIX_HPP
#define WAVELET_MATRIX_HPP

#include "../template.hpp"

template <class T = int64_t>
class WaveletMatrix {
	struct BitVector {
		int n;
		vec<uint64_t> bits;
		vec<int> prefix;
		explicit BitVector(int n) : n(n), bits((n >> 6) + 1), prefix(bits.size() + 1) {}
		void set(int i) { bits[i >> 6] |= uint64_t(1) << (i & 63); }
		bool get(int i) const { return (bits[i >> 6] >> (i & 63)) & 1; }
		void build() { rep(i, sz(bits)) prefix[i + 1] = prefix[i] + popcount(bits[i]); }
		int rank(int r) const { return prefix[r >> 6] + popcount(bits[r >> 6] & ((uint64_t(1) << (r & 63)) - 1)); }
		int select(int k, bool one) const {
			int l = 0, r = sz(bits);
			while (l + 1 < r) {
				int m = (l + r) / 2;
				int count = one ? prefix[m] : int(min<int64_t>(n, int64_t(m) * 64)) - prefix[m];
				if (count <= k) l = m; else r = m;
			}
			k -= one ? prefix[l] : int(min<int64_t>(n, int64_t(l) * 64)) - prefix[l];
			uint64_t word = one ? bits[l] : ~bits[l];
			int offset = 0;
			for (int step = 32; step; step >>= 1) {
				uint64_t mask = (uint64_t(1) << step) - 1;
				int count = popcount(word & mask);
				if (k >= count) k -= count, word >>= step, offset += step;
				else word &= mask;
			}
			return l * 64 + offset;
		}
	};
	int n = 0, levels = 0;
	vec<T> values;
	vec<BitVector> bits;
	vec<int> middle;
	void check(int l, int r) const { assert(0 <= l && l <= r && r <= n); }
	uint32_t code(const T &x) const { return lower_bound(all(values), x) - values.begin(); }
	pair<int, int> interval(int l, int r, uint32_t x) const {
		rep(d, levels) {
			int a = bits[d].rank(l), b = bits[d].rank(r);
			if ((x >> (levels - 1 - d)) & 1) l = middle[d] + a, r = middle[d] + b;
			else l -= a, r -= b;
		}
		return {l, r};
	}
	int less_than(int l, int r, uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		int answer = 0;
		rep(d, levels) {
			int a = bits[d].rank(l), b = bits[d].rank(r);
			if ((x >> (levels - 1 - d)) & 1) {
				answer += r - l - b + a;
				l = middle[d] + a; r = middle[d] + b;
			} else l -= a, r -= b;
		}
		return answer;
	}
public:
	WaveletMatrix() = default;
	explicit WaveletMatrix(const vec<T> &a) { build(a); }
	void build(const vec<T> &a) {
		assert(a.size() <= size_t(numeric_limits<int>::max()));
		n = sz(a);
		values = a;
		sort(all(values));
		values.erase(unique(all(values)), values.end());
		values.shrink_to_fit();
		levels = bit_width(unsigned(max(0, sz(values) - 1)));
		bits.clear(); bits.reserve(levels);
		middle.resize(levels);
		if (!levels) return;
		vec<uint32_t> current(n), next(n);
		rep(i, n) current[i] = code(a[i]);
		rep(d, levels) {
			bits.emplace_back(n);
			int shift = levels - 1 - d;
			rep(i, n) if ((current[i] >> shift) & 1) bits.back().set(i);
			bits.back().build();
			middle[d] = n - bits.back().rank(n);
			int zero = 0, one = middle[d];
			rep(i, n) next[((current[i] >> shift) & 1) ? one++ : zero++] = current[i];
			current.swap(next);
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	T access(int i) const {
		assert(0 <= i && i < n);
		uint32_t x = 0;
		rep(d, levels) {
			int ones = bits[d].rank(i);
			if (bits[d].get(i)) x |= uint32_t(1) << (levels - 1 - d), i = middle[d] + ones;
			else i -= ones;
		}
		return values[x];
	}
	T operator[](int i) const { return access(i); }
	int count(int l, int r, const T &x) const {
		check(l, r);
		uint32_t c = code(x);
		if (c == values.size() || values[c] != x) return 0;
		auto [a, b] = interval(l, r, c);
		return b - a;
	}
	int rank(const T &x, int r) const { return count(0, r, x); }
	int select(const T &x, int k) const {
		uint32_t c = code(x);
		if (k < 0 || c == values.size() || values[c] != x) return -1;
		auto [l, r] = interval(0, n, c);
		if (k >= r - l) return -1;
		int p = l + k;
		per(d, levels) {
			bool one = (c >> (levels - 1 - d)) & 1;
			p = bits[d].select(p - (one ? middle[d] : 0), one);
		}
		return p;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		uint32_t x = 0;
		rep(d, levels) {
			int a = bits[d].rank(l), b = bits[d].rank(r);
			int zero = r - l - b + a;
			if (k < zero) l -= a, r -= b;
			else {
				k -= zero; x |= uint32_t(1) << (levels - 1 - d);
				l = middle[d] + a; r = middle[d] + b;
			}
		}
		return values[x];
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		return kth_smallest(l, r, r - l - 1 - k);
	}
	int range_freq(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, code(upper)); }
	int range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		if (!(lower < upper)) return 0;
		return less_than(l, r, code(upper)) - less_than(l, r, code(lower));
	}
	optional<T> prev_value(int l, int r, const T &upper) const {
		int k = range_freq(l, r, upper);
		if (!k) return nullopt;
		return kth_smallest(l, r, k - 1);
	}
	optional<T> next_value(int l, int r, const T &lower) const {
		int k = range_freq(l, r, lower);
		if (k == r - l) return nullopt;
		return kth_smallest(l, r, k);
	}
};

#endif
