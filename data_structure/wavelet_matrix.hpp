#ifndef WAVELET_MATRIX_HPP
#define WAVELET_MATRIX_HPP

#include "../template.hpp"

template <class T = int64_t, class Sum = conditional_t<is_integral_v<T>, conditional_t<(sizeof(T) < 8), int64_t, __int128_t>, T>>
class WaveletMatrix {
	struct BitVector {
		struct Block { uint64_t low = 0, high = 0; array<int, 4> prefix{}; };
		int n;
		vec<Block> blocks;
		explicit BitVector(int n) : n(n), blocks((n >> 6) + 1) {}
		void build(const vec<uint32_t> &a, int shift) {
			array<int, 4> counts{};
			rep(w, sz(blocks)) {
				uint64_t low = 0, high = 0;
				int start = w * 64, length = min(64, n - start);
				rep(j, length) {
					low |= uint64_t((a[start + j] >> shift) & 1) << j;
					high |= uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
				}
				blocks[w] = {low, high, counts};
				int c1 = popcount(low & ~high), c2 = popcount(~low & high), c3 = popcount(low & high);
				counts[0] += length - c1 - c2 - c3;
				counts[1] += c1; counts[2] += c2; counts[3] += c3;
			}
		}
		uint64_t mask(const Block &b, int digit) const {
			return ((digit & 1) ? b.low : ~b.low) & ((digit & 2) ? b.high : ~b.high);
		}

		int rank(int r, int digit) const {
			const auto &b = blocks[r >> 6];
			return b.prefix[digit] + popcount(mask(b, digit) & ((uint64_t(1) << (r & 63)) - 1));
		}
		int below(int r, int digit) const {
			if (!digit) return 0;
			if (digit == 1) return rank(r, 0);
			if (digit == 3) return r - rank(r, 3);
			const auto &b = blocks[r >> 6];
			return b.prefix[0] + b.prefix[1] + popcount(~b.high & ((uint64_t(1) << (r & 63)) - 1));
		}
		array<int, 4> ranks(int r) const {
			const auto &b = blocks[r >> 6];
			uint64_t m = (uint64_t(1) << (r & 63)) - 1;
			array<int, 4> a;
			a[1] = b.prefix[1] + popcount(b.low & ~b.high & m);
			a[2] = b.prefix[2] + popcount(~b.low & b.high & m);
			a[3] = b.prefix[3] + popcount(b.low & b.high & m);
			a[0] = r - a[1] - a[2] - a[3];
			return a;
		}
	};
	int n = 0, levels = 0;
	vec<T> values;
	vec<uint32_t> original;
	vec<int> positions, offsets;
	bool dense = false;
	vec<BitVector> bits;
	vec<array<int, 4>> middle;
	static constexpr bool can_narrow = is_integral_v<T> && sizeof(T) <= 8 && is_same_v<Sum, __int128_t>;
	using CompactSum = conditional_t<can_narrow, int64_t, Sum>;
	mutable vec<vec<Sum>> sum_prefix;
	mutable vec<vec<CompactSum>> compact_prefix;
	mutable bool sum_ready = false, narrow_sums = false;
	static constexpr bool supports_sum = requires(const T &x, Sum s) { Sum(x); s + s; s - s; s * 1; };
	array<uint32_t, 8> small_codes(int l, int r) const {
		array<uint32_t, 8> a{};
		rep(i, r - l) a[i] = original[l + i];
		sort(a.begin(), a.begin() + (r - l));
		return a;
	}
	void check(int l, int r) const { assert(0 <= l && l <= r && r <= n); }
	uint32_t code(const T &x) const {
		if (values.empty()) return 0;
		if constexpr (is_integral_v<T> && sizeof(T) <= 8) if (dense) {
			if (x <= values.front()) return 0;
			if (values.back() < x) return sz(values);
			return uint64_t(x) - uint64_t(values.front());
		}
		int p = 0, length = sz(values);
		while (length > 1) {
			int half = length / 2;
			p += (values[p + half] < x) * half;
			length -= half;
		}
		return p + (values[p] < x);
	}
	pair<int, int> interval(int l, int r, uint32_t x) const {
		rep(d, levels) {
			int digit = (x >> (2 * (levels - 1 - d))) & 3;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return {l, r};
	}
	int less_than(int l, int r, uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		int answer = 0;
		rep(d, levels) {
			int digit = (x >> (2 * (levels - 1 - d))) & 3;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (digit == 3) answer += r - l - b + a;
			else answer += bits[d].below(r, digit) - bits[d].below(l, digit);
			l = middle[d][digit] + a; r = middle[d][digit] + b;
		}
		return answer;
	}
	vec<uint32_t> compress(const vec<T> &a) {
		vec<uint32_t> current(n);
		if constexpr (is_integral_v<T> && !is_same_v<T, bool> && sizeof(T) <= 8) {
			if (n >= 1024) {
				using U = make_unsigned_t<T>;
				struct Item { U key; uint32_t index; };
				vec<Item> order(n), next(n);
				U lo = numeric_limits<U>::max(), hi = 0;
				constexpr U sign = is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
				rep(i, n) {
					U key = U(a[i]) ^ sign;
					order[i] = {key, uint32_t(i)};
					lo = min(lo, key); hi = max(hi, key);
				}
				for (int shift = 0, width = bit_width(U(lo ^ hi)); shift < width; shift += 11) {
					array<int, 2048> offsets{};
					for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
					int total = 0;
					for (int &c : offsets) { int t = c; c = total; total += t; }
					for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
					order.swap(next);
				}
				values.clear(); values.reserve(n);
				U previous = 0;
				for (const auto &v : order) {
					if (values.empty() || v.key != previous) values.push_back(a[v.index]);
					previous = v.key; current[v.index] = sz(values) - 1;
				}
				values.shrink_to_fit();
				return current;
			}
		}
		values = a; sort(all(values));
		values.erase(unique(all(values)), values.end()); values.shrink_to_fit();
		rep(i, n) current[i] = code(a[i]);
		return current;
	}
	void partition(vec<uint32_t> &current, vec<uint32_t> &next, int d) const {
		auto offset = middle[d];
		int shift = 2 * (levels - 1 - d);
		rep(i, n) next[offset[(current[i] >> shift) & 3]++] = current[i];
		current.swap(next);
	}
	template <class S>
	void prefix_sum(vec<S> &prefix, const vec<uint32_t> &current, int l, int r) const requires supports_sum {
		prefix.resize(size_t(r - l) + 1);
		rep(i, r - l) prefix[i + 1] = prefix[i] + S(values[current[l + i]]);
	}
	template <class S>
	void partition_sum(vec<vec<S>> &prefix, vec<uint32_t> &current, vec<uint32_t> &next, int d) const requires supports_sum {
		int zero = 0, one = middle[d][2], shift = 2 * (levels - 1 - d);
		for (uint32_t x : current) {
			bool high = (x >> (shift + 1)) & 1;
			int p = high ? one : zero;
			one += high; zero += !high; next[p] = x;
		}
		int base = 3 * d + 1;
		prefix_sum(prefix[base], next, 0, middle[d][2]);
		current.swap(next); partition(current, next, d);
		prefix_sum(prefix[base + 1], current, 0, middle[d][1]);
		prefix_sum(prefix[base + 2], current, middle[d][2], middle[d][3]);
	}
	struct Route { int digit, a, b, k, high_l, high_r, zero_l, zero_r; };
	[[gnu::always_inline]] Route route(int l, int r, int k, int d) const {
		const auto &left = bits[d].blocks[l >> 6], &right = bits[d].blocks[r >> 6];
		uint64_t lm = (uint64_t(1) << (l & 63)) - 1, rm = (uint64_t(1) << (r & 63)) - 1;
		uint64_t lh = left.high & lm, rh = right.high & rm;
		int ha = left.prefix[2] + left.prefix[3] + popcount(lh);
		int hb = right.prefix[2] + right.prefix[3] + popcount(rh);
		int high_l = l - ha, high_r = r - hb;
		bool high = k >= high_r - high_l;
		k -= high ? high_r - high_l : 0;
		int a = left.prefix[2 * high] + popcount(~left.low & (high ? lh : lm ^ lh));
		int b = right.prefix[2 * high] + popcount(~right.low & (high ? rh : rm ^ rh));
		bool low = k >= b - a;
		k -= low ? b - a : 0;
		return {2 * high + low, low ? (high ? ha : high_l) - a : a,
			low ? (high ? hb : high_r) - b : b, k, high_l, high_r, a, b};
	}
	void init_sum() const requires supports_sum {
		if constexpr (can_narrow) {
			Sum positive = 0, negative = 0;
			rep(i, sz(values)) {
				Sum v = Sum(values[i]) * (offsets[i + 1] - offsets[i]);
				if (v > 0) positive += v; else negative += v;
			}
			narrow_sums = positive <= numeric_limits<int64_t>::max() && negative >= numeric_limits<int64_t>::min();
		}
		if (narrow_sums) {
			compact_prefix.resize(3 * levels + 1); prefix_sum(compact_prefix[0], original, 0, n);
		} else {
			sum_prefix.resize(3 * levels + 1); prefix_sum(sum_prefix[0], original, 0, n);
		}
	}
	void construct(const vec<T> &a, bool with_sum) {
		assert(a.size() <= size_t(numeric_limits<int>::max()));
		sum_prefix.clear(); compact_prefix.clear(); sum_ready = narrow_sums = false; n = sz(a);
		dense = false;
		if (!n || all_of(a.begin() + 1, a.end(), [&](const T &x) { return x == a.front(); })) {
			values.clear();
			if (n) values.push_back(a.front());
			values.shrink_to_fit(); original.clear(); positions.clear(); offsets = {0, n};
			levels = 0; bits.clear(); middle.clear(); sum_ready = with_sum;
			return;
		}
		original = compress(a);
		if constexpr (is_integral_v<T> && sizeof(T) <= 8)
			dense = !values.empty() && uint64_t(values.back()) - uint64_t(values.front()) == values.size() - 1;
		offsets.assign(values.size() + 1, 0); positions.resize(n);
		for (uint32_t x : original) ++offsets[x + 1];
		rep(i, sz(values)) offsets[i + 1] += offsets[i];
		auto cursor = offsets;
		rep(i, n) positions[cursor[original[i]]++] = i;
		auto current = original;
		levels = (bit_width(unsigned(max(0, sz(values) - 1))) + 1) / 2;
		bits.clear(); bits.reserve(levels); middle.resize(levels);
		if (!levels) return;
		if constexpr (supports_sum) if (with_sum) init_sum();
		vec<uint32_t> next(n);
		rep(d, levels) {
			bits.emplace_back(n); bits.back().build(current, 2 * (levels - 1 - d));
			auto totals = bits.back().ranks(n);
			middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
			if constexpr (supports_sum) {
				if (with_sum) {
					if (narrow_sums) partition_sum(compact_prefix, current, next, d);
					else partition_sum(sum_prefix, current, next, d);
				} else partition(current, next, d);
			} else partition(current, next, d);
		}
		sum_ready = with_sum;
	}
	template <class S>
	S sum_smallest_impl(const vec<vec<S>> &prefix, int l, int r, int k) const requires supports_sum {
		S answer = 0;
		uint32_t x = 0;
		rep(d, levels) {
			auto step = route(l, r, k, d);
			int base = 3 * d + 1;
			if (step.digit & 2) answer += prefix[base][step.high_r] - prefix[base][step.high_l];
			if (step.digit & 1) {
				const auto &row = prefix[base + 1 + (step.digit >> 1)];
				answer += row[step.zero_r] - row[step.zero_l];
			}
			k = step.k;
			if (!k) return answer;
			x = (x << 2) | step.digit;
			l = middle[d][step.digit] + step.a; r = middle[d][step.digit] + step.b;
		}
		return answer + S(values[x]) * k;
	}
public:
	using sum_type = Sum;
	WaveletMatrix() = default;
	explicit WaveletMatrix(const vec<T> &a) { build(a); }
	WaveletMatrix(const vec<T> &a, bool with_sum) requires supports_sum { construct(a, with_sum); }
	void build(const vec<T> &a) { construct(a, false); }
	int size() const { return n; }
	bool empty() const { return n == 0; }
	T access(int i) const { assert(0 <= i && i < n); return values[levels ? original[i] : 0]; }
	T operator[](int i) const { return access(i); }
	int count(int l, int r, const T &x) const {
		check(l, r);
		if (l == r) return 0;
		uint32_t c = code(x);
		if (c == values.size() || values[c] != x) return 0;
		if (!levels) return r - l;
		int start = offsets[c], end = offsets[c + 1];
		if (l == 0 && r == n) return end - start;
		if (end - start == 1) return int(l <= positions[start]) & int(positions[start] < r);
		if (end - start <= 16) {
			const int *first = positions.data() + start, *last = positions.data() + end;
			return lower_bound(first, last, r) - lower_bound(first, last, l);
		}
		auto [a, b] = interval(l, r, c);
		return b - a;
	}
	int rank(const T &x, int r) const { return count(0, r, x); }
	int select(const T &x, int k) const {
		uint32_t c = code(x);
		if (k < 0 || c == values.size() || values[c] != x || k >= offsets[c + 1] - offsets[c]) return -1;
		return levels ? positions[offsets[c] + k] : k;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r); assert(0 <= k && k < r - l);
		if (!levels) return values[0];
		if (r - l <= 8) return values[small_codes(l, r)[k]];
		uint32_t x = 0;
		rep(d, levels) {
			auto step = route(l, r, k, d);
			x = (x << 2) | step.digit; k = step.k;
			l = middle[d][step.digit] + step.a; r = middle[d][step.digit] + step.b;
		}
		return values[x];
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r); assert(0 <= k && k < r - l);
		return kth_smallest(l, r, r - l - 1 - k);
	}
	int range_freq(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, code(upper)); }
	int range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		if (!(lower < upper)) return 0;
		return less_than(l, r, code(upper)) - less_than(l, r, code(lower));
	}
	void prepare_sum() const requires supports_sum {
		if (sum_ready) return;
		if (!levels) { sum_ready = true; return; }
		init_sum();
		vec<uint32_t> current = original, next(n);
		if (narrow_sums) rep(d, levels) partition_sum(compact_prefix, current, next, d);
		else rep(d, levels) partition_sum(sum_prefix, current, next, d);
		sum_ready = true;
	}
	Sum range_sum(int l, int r) const requires supports_sum {
		check(l, r);
		if (l == r) return Sum(0);
		if (!levels) return Sum(values[0]) * (r - l);
		if (!sum_ready && r - l <= 8) {
			Sum answer = 0;
			rep(i, l, r) answer += Sum(values[original[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return compact_prefix[0][r] - compact_prefix[0][l];
		return sum_prefix[0][r] - sum_prefix[0][l];
	}
	Sum sum_smallest(int l, int r, int k) const requires supports_sum {
		check(l, r); assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (k == r - l) return range_sum(l, r);
		if (!levels) return Sum(values[0]) * k;
		if (k == 1) return Sum(kth_smallest(l, r, 0));
		if (r - l <= 8) {
			auto a = small_codes(l, r); Sum answer = 0;
			rep(i, k) answer += Sum(values[a[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return sum_smallest_impl(compact_prefix, l, r, k);
		return sum_smallest_impl(sum_prefix, l, r, k);
	}
	Sum sum_largest(int l, int r, int k) const requires supports_sum {
		check(l, r); assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (!levels) return Sum(values[0]) * k;
		if (k == 1) return Sum(kth_largest(l, r, 0));
		if (k == r - l) return range_sum(l, r);
		if (r - l <= 8) {
			auto a = small_codes(l, r); Sum answer = 0;
			rep(i, r - l - k, r - l) answer += Sum(values[a[i]]);
			return answer;
		}
		return range_sum(l, r) - sum_smallest(l, r, r - l - k);
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

template <class T>
WaveletMatrix(const vec<T> &, bool) -> WaveletMatrix<T>;

#endif
