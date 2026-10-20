#ifndef DYNAMIC_QUAD_WAVELET_HPP
#define DYNAMIC_QUAD_WAVELET_HPP

#include "dynamic_quad_vector.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace dynamic_wavelet_detail {
template<class T, int Bits, int Block = 8192, int Branch = 32>
class QuadMatrix {
	static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	static constexpr int width = std::numeric_limits<U>::digits;
	static_assert(1 <= Bits && Bits <= width);
	static constexpr int levels = (Bits + 1) / 2;
	static constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	static constexpr U sign = std::is_signed_v<T> ? U(1) << (Bits - 1) : 0;
	static constexpr T minimum = std::is_signed_v<T> ? T(U(0) - sign) : T(0);
	static constexpr T maximum = T(mask >> std::is_signed_v<T>);
	std::array<QuadVector<Block, Branch>, levels> rows;
	std::array<std::array<int, 5>, levels> offsets{};
	int n = 0;
	static bool contains(T value) { return minimum <= value && value <= maximum; }
	static U encode(T value) { return (U(value) & mask) ^ sign; }
	static T decode(U value) {
		value ^= sign;
		if constexpr (std::is_signed_v<T> && Bits < width) if (value & sign) value |= U(~mask);
		return T(value);
	}
	static int digit(U key, int d) { return (key >> (2 * (levels - 1 - d))) & 3; }
	static U extend(U prefix, int value, int d) { return U(prefix | (U(value) << (2 * (levels - 1 - d)))); }
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	U suffix(int p, U prefix, int d) const {
		for (; d < levels; ++d) {
			int uniform = rows[d].uniform_digit();
			if (uniform >= 0) { prefix = extend(prefix, uniform, d); continue; }
			auto [rank, value] = rows[d].access_rank(p);
			prefix = extend(prefix, value, d);
			p = offsets[d][value] + rank;
		}
		return prefix;
	}
	bool matches_suffix(int p, U key, int d) const {
		for (; d < levels; ++d) {
			int uniform = rows[d].uniform_digit();
			if (uniform >= 0) { if (uniform != digit(key, d)) return false; continue; }
			auto [rank, value] = rows[d].access_rank(p);
			if (value != digit(key, d)) return false;
			p = offsets[d][value] + rank;
		}
		return true;
	}
	template<bool Largest> U extreme(int l, int r, U prefix, int d) const {
		for (; d < levels; ++d) {
			int uniform = rows[d].uniform_digit();
			if (uniform >= 0) { prefix = extend(prefix, uniform, d); continue; }
			if (r - l == 1) return suffix(l, prefix, d);
			auto [a, b] = rows[d].ranks(l, r);
			int value = Largest ? 3 : 0;
			while (a[value] == b[value]) value += Largest ? -1 : 1;
			prefix = extend(prefix, value, d);
			l = offsets[d][value] + a[value]; r = offsets[d][value] + b[value];
		}
		return prefix;
	}
	int below(int l, int r, U key, int d) const {
		int result = 0;
		for (; d < levels && l < r; ++d) {
			int value = digit(key, d), uniform = rows[d].uniform_digit();
			if (uniform >= 0) {
				if (uniform != value) return result + (uniform < value ? r - l : 0);
				continue;
			}
			auto [a, b] = rows[d].ranks(l, r);
			for (int v = 0; v < value; ++v) result += b[v] - a[v];
			l = offsets[d][value] + a[value]; r = offsets[d][value] + b[value];
		}
		return result;
	}
	template<bool Successor> std::optional<T> adjacent(int l, int r, T bound) const {
		check(l, r);
		if (l == r) return std::nullopt;
		if constexpr (Successor) {
			if (maximum < bound) return std::nullopt;
			if (bound <= minimum) return decode(extreme<false>(l, r, 0, 0));
		} else {
			if (bound <= minimum) return std::nullopt;
			if (maximum < bound) return decode(extreme<true>(l, r, 0, 0));
		}
		U key = encode(bound), prefix = 0, candidate = 0;
		int left = 0, right = 0, depth = -1;
		for (int d = 0; d < levels && l < r; ++d) {
			int uniform = rows[d].uniform_digit();
			if (uniform >= 0) {
				prefix = extend(prefix, uniform, d);
				int wanted = digit(key, d);
				if (uniform == wanted) continue;
				if (Successor ? uniform > wanted : uniform < wanted) return decode(extreme<!Successor>(l, r, prefix, d + 1));
				l = r; break;
			}
			if (r - l == 1) {
				int p = l;
				for (int level = d; level < levels; ++level) {
					auto [rank, value] = rows[level].access_rank(p);
					int wanted = digit(key, level);
					prefix = extend(prefix, value, level);
					p = offsets[level][value] + rank;
					if (value == wanted) continue;
					if (Successor ? value > wanted : value < wanted) return decode(suffix(p, prefix, level + 1));
					l = r; break;
				}
				if constexpr (Successor) if (l < r) return bound;
				l = r; break;
			}
			int value = digit(key, d);
			auto [a, b] = rows[d].ranks(l, r);
			for (int sibling = value + (Successor ? 1 : -1); 0 <= sibling && sibling < 4; sibling += Successor ? 1 : -1)
				if (a[sibling] < b[sibling]) {
					left = offsets[d][sibling] + a[sibling]; right = offsets[d][sibling] + b[sibling];
					candidate = extend(prefix, sibling, d); depth = d + 1; break;
				}
			prefix = extend(prefix, value, d);
			l = offsets[d][value] + a[value]; r = offsets[d][value] + b[value];
		}
		if constexpr (Successor) if (l < r) return bound;
		if (depth < 0) return std::nullopt;
		return decode(extreme<!Successor>(left, right, candidate, depth));
	}

  public:
	using value_type = T;
	QuadMatrix() = default;
	explicit QuadMatrix(const std::vector<T>& values) { build(values); }
	QuadMatrix(const QuadMatrix&) = default;
	QuadMatrix& operator=(const QuadMatrix&) = default;
	QuadMatrix(QuadMatrix&& other) noexcept { swap(other); }
	QuadMatrix& operator=(QuadMatrix&& other) noexcept { if (this != &other) clear(), swap(other); return *this; }
	void swap(QuadMatrix& other) noexcept { rows.swap(other.rows); offsets.swap(other.offsets); std::swap(n, other.n); }
	int size() const { return n; }
	bool empty() const { return !n; }
	std::size_t memory_usage() const { std::size_t result = 0; for (const auto& row : rows) result += row.memory_usage(); return result; }
	void clear() { for (auto& row : rows) row.clear(); offsets.fill({}); n = 0; }
	void reserve(int size) { for (auto& row : rows) row.reserve(size); }
	void build(const std::vector<T>& values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = int(values.size());
		std::vector<U> current(n), next(n);
		std::vector<std::uint8_t> packed(n);
		for (int i = 0; i < n; ++i) { assert(contains(values[i])); current[i] = encode(values[i]); }
		for (int d = 0; d < levels; ++d) {
			for (int i = 0; i < n; ++i) packed[i] = std::uint8_t(digit(current[i], d));
			rows[d].build(packed);
			auto counts = rows[d].ranks(n);
			offsets[d][0] = 0;
			for (int v = 0; v < 4; ++v) offsets[d][v + 1] = offsets[d][v] + counts[v];
			if (d + 1 == levels || rows[d].uniform_digit() >= 0) continue;
			auto positions = offsets[d];
			for (int i = 0; i < n; ++i) next[positions[packed[i]]++] = current[i];
			current.swap(next);
		}
	}
	T access(int p) const { assert(0 <= p && p < n); return decode(suffix(p, 0, 0)); }
	T operator[](int p) const { return access(p); }
	void insert(int p, T value) {
		assert(0 <= p && p <= n && n < std::numeric_limits<int>::max() && contains(value));
		U key = encode(value);
		for (int d = 0; d < levels; ++d) {
			int v = digit(key, d), rank = rows[d].insert(p, v);
			p = offsets[d][v] + rank;
			for (int other = v + 1; other <= 4; ++other) ++offsets[d][other];
		}
		++n;
	}
	void push_back(T value) { insert(n, value); }
	T erase(int p) {
		assert(0 <= p && p < n);
		U key = 0;
		for (int d = 0; d < levels; ++d) {
			auto [rank, value] = rows[d].erase_rank(p);
			key = extend(key, value, d); p = offsets[d][value] + rank;
			for (int other = value + 1; other <= 4; ++other) --offsets[d][other];
		}
		--n; return decode(key);
	}
	T pop_back() { return erase(n - 1); }
	void set(int p, T value) {
		assert(0 <= p && p < n && contains(value));
		U key = encode(value);
		int old = p;
		for (int d = 0; d < levels; ++d) {
			int v = digit(key, d);
			if (offsets[d][v] == 0 && offsets[d][v + 1] == n) continue;
			bool same = old == p;
			auto [rank, previous, next] = same ? rows[d].set_rank(old, v) : rows[d].move_rank(old, p, v);
			if (same && previous == v) { old = p = offsets[d][v] + rank; continue; }
			old = offsets[d][previous] + rank;
			p = offsets[d][v] + next - int(previous < v);
			if (previous < v) for (int other = previous + 1; other <= v; ++other) --offsets[d][other];
			else for (int other = v + 1; other <= previous; ++other) ++offsets[d][other];
		}
	}
	int count(int l, int r, T value) const {
		check(l, r);
		if (l == r || !contains(value)) return 0;
		U key = encode(value);
		for (int d = 0; d < levels && l < r; ++d) {
			if (r - l == 1) return matches_suffix(l, key, d);
			int v = digit(key, d);
			auto [a, b] = rows[d].rank(l, r, v);
			l = offsets[d][v] + a; r = offsets[d][v] + b;
		}
		return r - l;
	}
	int rank(T value, int r) const { return count(0, r, value); }
	int range_freq(int l, int r, T upper) const {
		check(l, r);
		if (upper <= minimum) return 0;
		if (maximum < upper) return r - l;
		return below(l, r, encode(upper), 0);
	}
	int range_freq(int l, int r, T lower, T upper) const {
		check(l, r);
		if (l == r || !(lower < upper)) return 0;
		if (U(U(upper) - U(lower)) == U(1)) return count(l, r, lower);
		if (lower <= minimum) return range_freq(l, r, upper);
		if (maximum < upper) return r - l - range_freq(l, r, lower);
		U low = encode(lower), high = encode(upper);
		int common = levels - (std::bit_width(U(low ^ high)) + 1) / 2, d = 0;
		for (; d < common && l < r; ++d) {
			int v = digit(low, d); auto [a, b] = rows[d].rank(l, r, v);
			l = offsets[d][v] + a; r = offsets[d][v] + b;
		}
		if (l == r) return 0;
		auto [a, b] = rows[d].ranks(l, r);
		int left = digit(low, d), right = digit(high, d), result = 0;
		for (int v = left + 1; v < right; ++v) result += b[v] - a[v];
		result += b[left] - a[left] - below(offsets[d][left] + a[left], offsets[d][left] + b[left], low, d + 1);
		result += below(offsets[d][right] + a[right], offsets[d][right] + b[right], high, d + 1);
		return result;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r); assert(0 <= k && k < r - l);
		if (!k) return decode(extreme<false>(l, r, 0, 0));
		U result = 0;
		for (int d = 0; d < levels; ++d) {
			int uniform = rows[d].uniform_digit();
			if (uniform >= 0) { result = extend(result, uniform, d); continue; }
			if (r - l == 1) return decode(suffix(l, result, d));
			auto [a, b] = rows[d].ranks(l, r);
			int value = 0;
			while (k >= b[value] - a[value]) { k -= b[value] - a[value]; ++value; }
			result = extend(result, value, d);
			l = offsets[d][value] + a[value]; r = offsets[d][value] + b[value];
		}
		return decode(result);
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r); assert(0 <= k && k < r - l);
		if (!k) return decode(extreme<true>(l, r, 0, 0));
		return kth_smallest(l, r, r - l - 1 - k);
	}
	std::optional<T> prev_value(int l, int r, T upper) const { return adjacent<false>(l, r, upper); }
	std::optional<T> next_value(int l, int r, T lower) const { return adjacent<true>(l, r, lower); }
	int select(T value, int k) const {
		if (k < 0 || !contains(value)) return -1;
		U key = encode(value); int l = 0, r = n, d = 0;
		for (; d < levels && l < r; ++d) {
			if (k >= r - l) return -1;
			if (r - l == 1) { if (k || !matches_suffix(l, key, d)) return -1; break; }
			int v = digit(key, d); auto [a, b] = rows[d].rank(l, r, v);
			l = offsets[d][v] + a; r = offsets[d][v] + b;
		}
		if (k >= r - l) return -1;
		int p = l + k;
		for (--d; d >= 0; --d) { int v = digit(key, d); p = rows[d].select(v, p - offsets[d][v]); }
		return p;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
