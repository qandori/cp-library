#ifndef WAVELET_MATRIX_HPP
#define WAVELET_MATRIX_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include "../components/oct_bit_vector.hpp"
#include "../components/quad_bit_vector.hpp"
#include "../components/splitmix64.hpp"

template <class T = std::int64_t, class Sum = std::conditional_t<std::is_integral_v<T>, std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>, T>>
class WaveletMatrix {
	using BitVector = OctBitVector;
	int n = 0, levels = 0, ordered = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	std::vector<int> positions, offsets;
	bool dense = false;
	mutable std::vector<BitVector> bits;
	mutable std::vector<std::array<int, 8>> middle;
	bool vectorized = BitVector::has_avx2();
	mutable bool quaternary = false;
	mutable int sum_levels = 0;
	mutable std::vector<QuadBitVector> sum_bits;
	mutable std::vector<std::array<int, 4>> sum_middle;
	static constexpr bool can_narrow = std::is_integral_v<T> && sizeof(T) <= 8 && std::is_same_v<Sum, __int128_t>;
	using CompactSum = std::conditional_t<can_narrow, std::int64_t, Sum>;
	mutable std::vector<std::vector<Sum>> sum_prefix;
	mutable std::vector<std::vector<CompactSum>> compact_prefix;
	mutable bool sum_ready = false, narrow_sums = false;
	mutable std::vector<std::uint32_t> count_index;
	mutable std::uint64_t count_seed = 0;
	std::uint32_t count_code(const T &x) const {
		std::size_t mask = count_index.size() - 1;
		std::size_t p = splitmix64((std::uint64_t(x) ^ count_seed) + 0x9e3779b97f4a7c15ULL) & mask;
		while (auto c = count_index[p]) {
			if (values[c - 1] == x) return c - 1;
			p = (p + 1) & mask;
		}
		return static_cast<std::uint32_t>(values.size());
	}
	static constexpr bool supports_sum = requires(const T &x, Sum s) { Sum(x); s + s; s - s; s * 1; };
	std::array<std::uint32_t, 8> small_codes(int l, int r) const {
		std::array<std::uint32_t, 8> a;
		a.fill(std::numeric_limits<std::uint32_t>::max());
		for (int i = 0; i < r - l; ++i) a[i] = original[l + i];
		auto swap = [&](int i, int j) {
			auto lo = std::min(a[i], a[j]), hi = std::max(a[i], a[j]);
			a[i] = lo;
			a[j] = hi;
		};
		swap(0, 1), swap(2, 3);
		if (r - l <= 4) {
			swap(0, 2), swap(1, 3), swap(1, 2);
		} else {
			swap(4, 5), swap(6, 7);
			swap(0, 2), swap(1, 3), swap(4, 6), swap(5, 7);
			swap(1, 2), swap(5, 6), swap(0, 4), swap(3, 7);
			swap(1, 5), swap(2, 6);
			swap(1, 4), swap(3, 6);
			swap(2, 4), swap(3, 5);
			swap(3, 4);
		}
		return a;
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	std::uint32_t code(const T &x) const {
		if (values.empty()) return 0;
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			if (dense) {
				if (x <= values.front()) return 0;
				if (values.back() < x) return static_cast<int>(values.size());
				return std::uint64_t(x) - std::uint64_t(values.front());
			}
		int p = 0, length = static_cast<int>(values.size());
		while (length > 1) {
			int half = length / 2;
			p += (values[p + half] < x) * half;
			length -= half;
		}
		return p + (values[p] < x);
	}
	std::pair<int, int> interval_scalar(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return {l, r};
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) std::pair<int, int> interval_avx2(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return {l, r};
	}
#endif
	std::pair<int, int> interval(int l, int r, std::uint32_t x) const {
		if (quaternary) return quad_interval(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return interval_avx2(l, r, x);
#endif
		return interval_scalar(l, r, x);
	}
	int less_than_scalar(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (digit == 7) answer += r - l - b + a;
			else answer += bits[d].below(r, digit) - bits[d].below(l, digit);
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		return answer;
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) int less_than_avx2(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (digit == 7) answer += r - l - b + a;
			else answer += bits[d].below(r, digit) - bits[d].below(l, digit);
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		return answer;
	}
#endif
	int less_than(int l, int r, std::uint32_t x) const {
		if (quaternary) return quad_less_than(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return less_than_avx2(l, r, x);
#endif
		return less_than_scalar(l, r, x);
	}
	template <bool Previous>
	std::optional<T> neighbor_scalar(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 11> path;
		int d = 0;
		for (; d < levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (a == b) break;
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		if (d == levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 3 * (levels - 1 - d), digit = (x >> shift) & 7;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 8; digit += Previous ? -1 : 1) {
				int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(7)) | digit;
				l = middle[d][digit] + a;
				r = middle[d][digit] + b;
				for (++d; d < levels; ++d) {
					auto step = route_scalar(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 3) | step.digit;
					l = middle[d][step.digit] + step.a;
					r = middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
#if defined(__x86_64__) || defined(__i386__)
	template <bool Previous>
	__attribute__((target("avx2,popcnt"))) std::optional<T> neighbor_avx2(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 11> path;
		int d = 0;
		for (; d < levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (3 * (levels - 1 - d))) & 7;
			int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
			if (a == b) break;
			l = middle[d][digit] + a;
			r = middle[d][digit] + b;
		}
		if (d == levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 3 * (levels - 1 - d), digit = (x >> shift) & 7;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 8; digit += Previous ? -1 : 1) {
				int a = bits[d].rank(l, digit), b = bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(7)) | digit;
				l = middle[d][digit] + a;
				r = middle[d][digit] + b;
				for (++d; d < levels; ++d) {
					auto step = route_avx2(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 3) | step.digit;
					l = middle[d][step.digit] + step.a;
					r = middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
#endif
	template <bool Previous>
	std::optional<T> neighbor(int l, int r, std::uint32_t x) const {
		if (quaternary) return quad_neighbor<Previous>(l, r, x);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return neighbor_avx2<Previous>(l, r, x);
#endif
		return neighbor_scalar<Previous>(l, r, x);
	}
	std::vector<std::uint32_t> compress(const std::vector<T> &a) {
		std::vector<std::uint32_t> current(n);
		bool ascending = std::is_sorted(a.begin(), a.end());
		if (ascending || std::is_sorted(a.rbegin(), a.rend())) {
			ordered = ascending ? 1 : -1;
			values.clear();
			values.reserve(n);
			for (int i = 0; i < n; ++i) {
				int j = ascending ? i : n - 1 - i;
				if (values.empty() || values.back() != a[j]) values.push_back(a[j]);
				current[j] = static_cast<int>(values.size()) - 1;
			}
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			return current;
		}
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
			if (n >= 1024) {
				using U = std::make_unsigned_t<T>;
				struct [[gnu::packed]] Item {
					U key;
					std::uint32_t index;
				};
				std::vector<Item> order(n);
				U lo = std::numeric_limits<U>::max(), hi = 0;
				constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
				for (int i = 0; i < n; ++i) {
					U key = U(a[i]) ^ sign;
					order[i] = {key, std::uint32_t(i)};
					lo = std::min(lo, key);
					hi = std::max(hi, key);
				}
				if (hi - lo < 65536 && std::uint64_t(hi - lo) < unsigned(n)) {
					std::vector<int> codes(std::size_t(hi - lo) + 1, -1);
					for (const auto &v : order) codes[v.key - lo] = 0;
					values.clear();
					values.reserve(codes.size());
					for (std::size_t i = 0; i < codes.size(); ++i)
						if (codes[i] == 0) {
							codes[i] = static_cast<int>(values.size());
							values.push_back(T(U(lo + U(i)) ^ sign));
						}
					for (const auto &v : order) current[v.index] = codes[v.key - lo];
					if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
					return current;
				}
				std::vector<Item> next(n);
				int width = std::bit_width(U(lo ^ hi)), span_width = std::bit_width(U(hi - lo));
				if ((width + 10) / 11 > (span_width + 10) / 11) {
					for (auto &v : order) v.key -= lo;
					width = span_width;
				}
				for (int shift = 0; shift < width; shift += 11) {
					std::array<int, 2048> offsets{};
					for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
					int total = 0;
					for (int &c : offsets) {
						int t = c;
						c = total;
						total += t;
					}
					for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
					order.swap(next);
				}
				values.clear();
				values.reserve(n);
				U previous = 0;
				positions.resize(n);
				offsets.clear();
				offsets.reserve(std::size_t(n) + 1);
				for (int j = 0; j < n; ++j) {
					const auto &v = order[j];
					if (values.empty() || v.key != previous) {
						values.push_back(a[v.index]);
						offsets.push_back(j);
					}
					previous = v.key;
					current[v.index] = static_cast<int>(values.size()) - 1;
					positions[j] = v.index;
				}
				offsets.push_back(n);
				if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
				return current;
			}
		}
		values = a;
		std::sort(values.begin(), values.end());
		values.erase(std::unique(values.begin(), values.end()), values.end());
		if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
		for (int i = 0; i < n; ++i) current[i] = code(a[i]);
		return current;
	}
	void partition(std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const {
		auto offset = middle[d];
		int shift = 3 * (levels - 1 - d);
		for (int i = 0; i < n; ++i) next[offset[(current[i] >> shift) & 7]++] = current[i];
		current.swap(next);
	}
	template <class S>
	void prefix_sum(std::vector<S> &prefix, const std::vector<std::uint32_t> &current, int l, int r) const
		requires supports_sum
	{
		prefix.resize(std::size_t(r - l) + 1);
		for (int i = 0; i < r - l; ++i) prefix[i + 1] = prefix[i] + S(values[current[l + i]]);
	}
	void partition_for_sum(std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const {
		auto offset = sum_middle[d];
		int shift = 2 * (sum_levels - 1 - d);
		for (int i = 0; i < n; ++i) next[offset[(current[i] >> shift) & 3]++] = current[i];
		current.swap(next);
	}
	template <class S>
	void partition_sum(std::vector<std::vector<S>> &prefix, std::vector<std::uint32_t> &current, std::vector<std::uint32_t> &next, int d) const
		requires supports_sum
	{
		int zero = 0, one = sum_middle[d][2], shift = 2 * (sum_levels - 1 - d);
		for (std::uint32_t x : current) {
			bool high = (x >> (shift + 1)) & 1;
			int p = high ? one : zero;
			one += high;
			zero += !high;
			next[p] = x;
		}
		int base = 3 * d + 1;
		prefix_sum(prefix[base], next, 0, sum_middle[d][2]);
		current.swap(next);
		partition_for_sum(current, next, d);
		prefix_sum(prefix[base + 1], current, 0, sum_middle[d][1]);
		prefix_sum(prefix[base + 2], current, sum_middle[d][2], sum_middle[d][3]);
	}
	struct SumRoute {
		int digit, a, b, k, high_l, high_r, zero_l, zero_r;
	};
	[[gnu::always_inline]] SumRoute sum_route(int l, int r, int k, int d) const {
		const auto &left = sum_bits[d].blocks[l >> 6], &right = sum_bits[d].blocks[r >> 6];
		std::uint64_t lm = (std::uint64_t(1) << (l & 63)) - 1, rm = (std::uint64_t(1) << (r & 63)) - 1;
		std::uint64_t lh = left.high & lm, rh = right.high & rm;
		int ha = left.prefix[2] + left.prefix[3] + std::popcount(lh);
		int hb = right.prefix[2] + right.prefix[3] + std::popcount(rh);
		int high_l = l - ha, high_r = r - hb;
		bool high = k >= high_r - high_l;
		k -= high ? high_r - high_l : 0;
		int a = left.prefix[2 * high] + std::popcount(~left.low & (high ? lh : lm ^ lh));
		int b = right.prefix[2 * high] + std::popcount(~right.low & (high ? rh : rm ^ rh));
		bool low = k >= b - a;
		k -= low ? b - a : 0;
		return {2 * high + low, low ? (high ? ha : high_l) - a : a,
				low ? (high ? hb : high_r) - b : b, k, high_l, high_r, a, b};
	}
	[[gnu::noinline]] std::pair<int, int> quad_interval(int l, int r, std::uint32_t x) const {
		for (int d = 0; d < sum_levels; ++d) {
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			l = sum_middle[d][digit] + sum_bits[d].rank(l, digit);
			r = sum_middle[d][digit] + sum_bits[d].rank(r, digit);
		}
		return {l, r};
	}
	[[gnu::noinline]] int quad_less_than(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return 0;
		if (x >= values.size()) return r - l;
		if (ordered) return std::clamp(ordered > 0 ? offsets[x] - l : r - n + offsets[x], 0, r - l);
		int answer = 0;
		if (r - l <= 8) {
			for (int i = l; i < r; ++i) answer += original[i] < x;
			return answer;
		}
		for (int d = 0; d < sum_levels; ++d) {
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
			if (digit == 3) answer += r - l - b + a;
			else answer += sum_bits[d].below(r, digit) - sum_bits[d].below(l, digit);
			l = sum_middle[d][digit] + a;
			r = sum_middle[d][digit] + b;
		}
		return answer;
	}
	template <bool Previous>
	[[gnu::noinline]] std::optional<T> quad_neighbor(int l, int r, std::uint32_t x) const {
		if (r - l <= 8) {
			std::uint32_t best = Previous ? 0 : static_cast<std::uint32_t>(values.size());
			for (int i = l; i < r; ++i) {
				std::uint32_t c = original[i];
				if constexpr (Previous) {
					if (c <= x) best = std::max(best, c + 1);
				} else if (c >= x) best = std::min(best, c);
			}
			if constexpr (Previous) return best ? std::optional<T>(values[best - 1]) : std::nullopt;
			else return best < values.size() ? std::optional<T>(values[best]) : std::nullopt;
		}
		std::array<std::pair<int, int>, 16> path;
		int d = 0;
		for (; d < sum_levels; ++d) {
			path[d] = {l, r};
			int digit = (x >> (2 * (sum_levels - 1 - d))) & 3;
			int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
			if (a == b) break;
			l = sum_middle[d][digit] + a;
			r = sum_middle[d][digit] + b;
		}
		if (d == sum_levels) return values[x];
		for (; d >= 0; --d) {
			l = path[d].first;
			r = path[d].second;
			int shift = 2 * (sum_levels - 1 - d), digit = (x >> shift) & 3;
			for (digit += Previous ? -1 : 1; 0 <= digit && digit < 4; digit += Previous ? -1 : 1) {
				int a = sum_bits[d].rank(l, digit), b = sum_bits[d].rank(r, digit);
				if (a == b) continue;
				x = ((x >> shift) & ~std::uint32_t(3)) | digit;
				l = sum_middle[d][digit] + a;
				r = sum_middle[d][digit] + b;
				for (++d; d < sum_levels; ++d) {
					auto step = quad_route(l, r, Previous ? r - l - 1 : 0, d);
					x = (x << 2) | step.digit;
					l = sum_middle[d][step.digit] + step.a;
					r = sum_middle[d][step.digit] + step.b;
				}
				return values[x];
			}
		}
		return std::nullopt;
	}
	[[gnu::always_inline]] SumRoute quad_route(int l, int r, int k, int d) const {
		const auto &left = sum_bits[d].blocks[l >> 6], &right = sum_bits[d].blocks[r >> 6];
		std::uint64_t lm = (std::uint64_t(1) << (l & 63)) - 1, rm = (std::uint64_t(1) << (r & 63)) - 1;
		int la = left.prefix[0] + std::popcount(~(left.low | left.high) & lm);
		int ra = right.prefix[0] + std::popcount(~(right.low | right.high) & rm);
		int lb = left.prefix[0] + left.prefix[1] + std::popcount(~left.high & lm);
		int rb = right.prefix[0] + right.prefix[1] + std::popcount(~right.high & rm);
		int lc = l - left.prefix[3] - std::popcount(left.low & left.high & lm);
		int rc = r - right.prefix[3] - std::popcount(right.low & right.high & rm);
		int digit = (k >= ra - la) + (k >= rb - lb) + (k >= rc - lc);
		std::array<int, 5> a{0, la, lb, lc, l}, b{0, ra, rb, rc, r};
		int base = digit & 2;
		return {digit, a[digit + 1] - a[digit], b[digit + 1] - b[digit], k - b[digit] + a[digit], lb, rb, a[base + 1] - a[base], b[base + 1] - b[base]};
	}
	[[gnu::noinline]] T quad_kth(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d < sum_levels; ++d) {
			auto step = quad_route(l, r, k, d);
			x = (x << 2) | step.digit;
			k = step.k;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return values[x];
	}
	using Route = typename BitVector::Route;
	[[gnu::always_inline]] Route route_scalar(int l, int r, int k, int d) const {
		return bits[d].route_scalar(l, r, k);
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"), always_inline)) Route route_avx2(int l, int r, int k, int d) const {
		return bits[d].route_avx2(l, r, k);
	}
#endif
	void init_sum() const
		requires supports_sum
	{
		sum_levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 1) / 2;
		if constexpr (can_narrow) {
			Sum positive = 0, negative = 0;
			for (int i = 0; i < static_cast<int>(values.size()); ++i) {
				Sum v = Sum(values[i]) * (offsets[i + 1] - offsets[i]);
				if (v > 0) positive += v;
				else negative += v;
			}
			narrow_sums = positive <= std::numeric_limits<std::int64_t>::max() && negative >= std::numeric_limits<std::int64_t>::min();
		}
		if (narrow_sums) {
			compact_prefix.resize(ordered ? 1 : 3 * sum_levels + 1);
			prefix_sum(compact_prefix[0], original, 0, n);
		} else {
			sum_prefix.resize(ordered ? 1 : 3 * sum_levels + 1);
			prefix_sum(sum_prefix[0], original, 0, n);
		}
	}
	void construct(const std::vector<T> &a, bool with_sum) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		count_index.clear();
		count_seed = 0;
		sum_bits.clear();
		sum_middle.clear();
		sum_levels = 0;
		quaternary = false;
		sum_prefix.clear();
		compact_prefix.clear();
		sum_ready = narrow_sums = false;
		n = static_cast<int>(a.size());
		dense = false;
		ordered = 0;
		if (!n || std::all_of(a.begin() + 1, a.end(), [&](const T &x) { return x == a.front(); })) {
			values.clear();
			if (n) values.push_back(a.front());
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			original.clear();
			positions.clear();
			offsets = {0, n};
			levels = 0;
			bits.clear();
			middle.clear();
			sum_ready = with_sum;
			return;
		}
		offsets.clear();
		original = compress(a);
		bool indexed = !offsets.empty();
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			dense = std::uint64_t(values.back()) - std::uint64_t(values.front()) == values.size() - 1;
		if (!indexed) offsets.assign(values.size() + 1, 0);
		levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 2) / 3;
		if (!indexed) {
			for (std::uint32_t x : original) ++offsets[x + 1];
			for (int i = 0; i < static_cast<int>(values.size()); ++i) offsets[i + 1] += offsets[i];
		}
		if (ordered) {
			positions.clear();
			bits.clear();
			middle.clear();
			if constexpr (supports_sum)
				if (with_sum) init_sum();
			sum_ready = with_sum;
			return;
		}
		if (!indexed) {
			positions.resize(n);
			auto cursor = offsets;
			for (int i = 0; i < n; ++i) positions[cursor[original[i]]++] = i;
		}
		bits.clear();
		middle.clear();
		if (with_sum) {
			quaternary = true;
			sum_levels = (std::bit_width(unsigned(static_cast<int>(values.size()) - 1)) + 1) / 2;
			levels = sum_levels;
			sum_bits.reserve(sum_levels);
			sum_middle.resize(sum_levels);
			if constexpr (supports_sum)
				if (with_sum) init_sum();
			std::vector<std::uint32_t> current = original, next(n);
			for (int d = 0; d < sum_levels; ++d) {
				sum_bits.emplace_back(n);
				sum_bits.back().build(current, 2 * (sum_levels - 1 - d));
				auto totals = sum_bits.back().ranks(n);
				sum_middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
				if constexpr (supports_sum) {
					if (with_sum) {
						if (narrow_sums) partition_sum(compact_prefix, current, next, d);
						else partition_sum(sum_prefix, current, next, d);
					} else if (d + 1 < sum_levels) partition_for_sum(current, next, d);
				} else if (d + 1 < sum_levels) partition_for_sum(current, next, d);
			}
			sum_ready = with_sum;
			return;
		}
		auto current = original;
		bits.clear();
		bits.reserve(levels);
		middle.resize(levels);
		std::vector<std::uint32_t> next(n);
		for (int d = 0; d < levels; ++d) {
			bits.emplace_back(n);
			bits.back().build(current, 3 * (levels - 1 - d));
			auto totals = bits.back().ranks(n);
			middle[d][0] = 0;
			for (int i = 1; i < 8; ++i) middle[d][i] = middle[d][i - 1] + totals[i - 1];
			if (d + 1 < levels) partition(current, next, d);
		}
		if constexpr (supports_sum)
			if (with_sum) prepare_sum();
	}
	template <class S>
	S sum_smallest_impl_scalar(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
		S answer = 0;
		std::uint32_t x = 0;
		for (int d = 0; d < sum_levels; ++d) {
			auto step = sum_route(l, r, k, d);
			int base = 3 * d + 1;
			if (step.digit & 2) answer += prefix[base][step.high_r] - prefix[base][step.high_l];
			if (step.digit & 1) {
				const auto &row = prefix[base + 1 + (step.digit >> 1)];
				answer += row[step.zero_r] - row[step.zero_l];
			}
			k = step.k;
			if (!k) return answer;
			x = (x << 2) | step.digit;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return answer + S(values[x]) * k;
	}
#if defined(__x86_64__) || defined(__i386__)
	template <class S>
	__attribute__((target("avx2,popcnt"))) S sum_smallest_impl_avx2(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
		S answer = 0;
		std::uint32_t x = 0;
		for (int d = 0; d < sum_levels; ++d) {
			auto step = sum_route(l, r, k, d);
			int base = 3 * d + 1;
			if (step.digit & 2) answer += prefix[base][step.high_r] - prefix[base][step.high_l];
			if (step.digit & 1) {
				const auto &row = prefix[base + 1 + (step.digit >> 1)];
				answer += row[step.zero_r] - row[step.zero_l];
			}
			k = step.k;
			if (!k) return answer;
			x = (x << 2) | step.digit;
			l = sum_middle[d][step.digit] + step.a;
			r = sum_middle[d][step.digit] + step.b;
		}
		return answer + S(values[x]) * k;
	}
#endif
	template <class S>
	S sum_smallest_impl(const std::vector<std::vector<S>> &prefix, int l, int r, int k) const
		requires supports_sum
	{
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return sum_smallest_impl_avx2(prefix, l, r, k);
#endif
		return sum_smallest_impl_scalar(prefix, l, r, k);
	}

	T kth_scalar(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d < levels; ++d) {
			auto step = route_scalar(l, r, k, d);
			x = (x << 3) | step.digit;
			k = step.k;
			l = middle[d][step.digit] + step.a;
			r = middle[d][step.digit] + step.b;
		}
		return values[x];
	}
#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2,popcnt"))) T kth_avx2(int l, int r, int k) const {
		std::uint32_t x = 0;
		for (int d = 0; d + 1 < levels; ++d) {
			auto step = route_avx2(l, r, k, d);
			x = (x << 3) | step.digit;
			k = step.k;
			l = middle[d][step.digit] + step.a;
			r = middle[d][step.digit] + step.b;
		}
		return values[(x << 3) | bits.back().select_avx2(l, r, k)];
	}
#endif

  public:
	using sum_type = Sum;
	WaveletMatrix() = default;
	explicit WaveletMatrix(const std::vector<T> &a) { build(a); }
	WaveletMatrix(const std::vector<T> &a, bool with_sum)
		requires supports_sum
	{ construct(a, with_sum); }
	void build(const std::vector<T> &a) { construct(a, false); }
	void prepare_count() const {
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8) {
			if (values.size() <= 1024 || dense || !count_index.empty()) return;
			count_seed = std::uint64_t(std::chrono::steady_clock::now().time_since_epoch().count());
			count_index.assign(std::bit_ceil(2 * values.size()), 0);
			std::size_t mask = count_index.size() - 1;
			for (std::size_t c = 0; c < values.size(); ++c) {
				std::size_t p = splitmix64((std::uint64_t(values[c]) ^ count_seed) + 0x9e3779b97f4a7c15ULL) & mask;
				while (count_index[p]) p = (p + 1) & mask;
				count_index[p] = static_cast<std::uint32_t>(c + 1);
			}
		}
	}
	int size() const { return n; }
	bool empty() const { return n == 0; }
	T access(int i) const {
		assert(0 <= i && i < n);
		return values[levels ? original[i] : 0];
	}
	T operator[](int i) const { return access(i); }
	int count(int l, int r, const T &x) const {
		check(l, r);
		if (l == r) return 0;
		std::uint32_t c;
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			c = (values.size() <= 1024 || dense || count_index.empty()) ? code(x) : count_code(x);
		else c = code(x);
		if (c == values.size() || values[c] != x) return 0;
		if (!levels) return r - l;
		int start = offsets[c], end = offsets[c + 1];
		if (ordered) {
			int first = ordered > 0 ? start : n - end;
			return std::max(0, std::min(r, first + (end - start)) - std::max(l, first));
		}
		if (l == 0 && r == n) return end - start;
		if (end - start == 1) return int(l <= positions[start]) & int(positions[start] < r);
		if (end - start <= 16) {
			int answer = 0;
			for (int i = start; i < end; ++i) answer += unsigned(positions[i] - l) < unsigned(r - l);
			return answer;
		}
		if (r - l <= 8) {
			int answer = 0;
			for (int i = l; i < r; ++i) answer += original[i] == c;
			return answer;
		}
		if (values.size() >= 1024 && std::size_t(end - start) <= values.size()) {
			int a = 0, b = 0, length = end - start;
			while (length > 1) {
				int half = length / 2;
				a += (positions[start + a + half] < l) * half;
				b += (positions[start + b + half] < r) * half;
				length -= half;
			}
			return b - a + (positions[start + b] < r) - (positions[start + a] < l);
		}
		auto [a, b] = interval(l, r, c);
		return b - a;
	}
	int rank(const T &x, int r) const { return count(0, r, x); }
	int select(const T &x, int k) const {
		std::uint32_t c;
		if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
			c = (values.size() <= 1024 || dense || count_index.empty()) ? code(x) : count_code(x);
		else c = code(x);
		if (k < 0 || c == values.size() || values[c] != x || k >= offsets[c + 1] - offsets[c]) return -1;
		if (ordered) return (ordered > 0 ? offsets[c] : n - offsets[c + 1]) + k;
		return levels ? positions[offsets[c] + k] : k;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		if (!levels) return values[0];
		if (ordered) return values[original[ordered > 0 ? l + k : r - 1 - k]];
		if (r - l <= 8) return values[small_codes(l, r)[k]];
		if (quaternary) return quad_kth(l, r, k);
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized) return kth_avx2(l, r, k);
#endif
		return kth_scalar(l, r, k);
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		return kth_smallest(l, r, r - l - 1 - k);
	}
	int range_freq(int l, int r, const T &upper) const {
		check(l, r);
		return less_than(l, r, code(upper));
	}
	int range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		if (!(lower < upper)) return 0;
		return less_than(l, r, code(upper)) - less_than(l, r, code(lower));
	}
	void prepare_sum() const
		requires supports_sum
	{
		if (sum_ready) return;
		if (!levels) {
			sum_ready = true;
			return;
		}
		init_sum();
		if (ordered) {
			sum_ready = true;
			return;
		}
		std::vector<std::uint32_t> current = original, next(n);
		bool indexed = !sum_bits.empty();
		sum_bits.reserve(sum_levels);
		sum_middle.resize(sum_levels);
		for (int d = 0; d < sum_levels; ++d) {
			if (!indexed) {
				sum_bits.emplace_back(n);
				sum_bits.back().build(current, 2 * (sum_levels - 1 - d));
				auto totals = sum_bits.back().ranks(n);
				sum_middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
			}
			if (narrow_sums) partition_sum(compact_prefix, current, next, d);
			else partition_sum(sum_prefix, current, next, d);
		}
		bits.clear();
		middle.clear();
		quaternary = true;
		sum_ready = true;
	}
	Sum range_sum(int l, int r) const
		requires supports_sum
	{
		check(l, r);
		if (l == r) return Sum(0);
		if (!levels) return Sum(values[0]) * (r - l);
		if (!sum_ready && r - l <= 8) {
			Sum answer = 0;
			for (int i = l; i < r; ++i) answer += Sum(values[original[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return compact_prefix[0][r] - compact_prefix[0][l];
		return sum_prefix[0][r] - sum_prefix[0][l];
	}
	Sum sum_smallest(int l, int r, int k) const
		requires supports_sum
	{
		check(l, r);
		assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (k == r - l) return range_sum(l, r);
		if (!levels) return Sum(values[0]) * k;
		if (ordered) return ordered > 0 ? range_sum(l, l + k) : range_sum(r - k, r);
		if (k == 1) return Sum(kth_smallest(l, r, 0));
		if (r - l <= 8) {
			auto a = small_codes(l, r);
			Sum answer = 0;
			for (int i = 0; i < k; ++i) answer += Sum(values[a[i]]);
			return answer;
		}
		prepare_sum();
		if (narrow_sums) return sum_smallest_impl(compact_prefix, l, r, k);
		return sum_smallest_impl(sum_prefix, l, r, k);
	}
	Sum sum_largest(int l, int r, int k) const
		requires supports_sum
	{
		check(l, r);
		assert(0 <= k && k <= r - l);
		if (!k) return Sum(0);
		if (!levels) return Sum(values[0]) * k;
		if (ordered) return ordered > 0 ? range_sum(r - k, r) : range_sum(l, l + k);
		if (k == 1) return Sum(kth_largest(l, r, 0));
		if (k == r - l) return range_sum(l, r);
		if (r - l <= 8) {
			auto a = small_codes(l, r);
			Sum answer = 0;
			for (int i = r - l - k; i < r - l; ++i) answer += Sum(values[a[i]]);
			return answer;
		}
		return range_sum(l, r) - sum_smallest(l, r, r - l - k);
	}
	std::optional<T> prev_value(int l, int r, const T &upper) const {
		check(l, r);
		if (!levels) return l < r && values[0] < upper ? std::optional<T>(values[0]) : std::nullopt;
		if (!ordered) {
			auto x = code(upper);
			if (!x) return std::nullopt;
			return neighbor<true>(l, r, x - 1);
		}
		int k = range_freq(l, r, upper);
		if (!k) return std::nullopt;
		return kth_smallest(l, r, k - 1);
	}
	std::optional<T> next_value(int l, int r, const T &lower) const {
		check(l, r);
		if (!levels) return l < r && !(values[0] < lower) ? std::optional<T>(values[0]) : std::nullopt;
		if (!ordered) {
			auto x = code(lower);
			if (x == values.size()) return std::nullopt;
			return neighbor<false>(l, r, x);
		}
		int k = range_freq(l, r, lower);
		if (k == r - l) return std::nullopt;
		return kth_smallest(l, r, k);
	}
};

template <class T>
WaveletMatrix(const std::vector<T> &, bool) -> WaveletMatrix<T>;

#endif
