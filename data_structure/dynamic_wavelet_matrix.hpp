#ifndef DYNAMIC_WAVELET_MATRIX_HPP
#define DYNAMIC_WAVELET_MATRIX_HPP

#include "dynamic_bit_vector.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

template <class T = std::int64_t, int Bits = sizeof(T) * 8, class BitVector = DynamicBitVector<>>
class DynamicWaveletMatrix {
	static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	static constexpr int width = std::numeric_limits<U>::digits;
	static_assert(1 <= Bits && Bits <= width);
	static constexpr U mask = std::numeric_limits<U>::max() >> (width - Bits);
	static constexpr U sign = std::is_signed_v<T> ? U(1) << (Bits - 1) : 0;
	static constexpr T minimum = std::is_signed_v<T> ? T(U(0) - sign) : T(0);
	static constexpr T maximum = T(mask >> std::is_signed_v<T>);
	std::array<BitVector, Bits> bits;
	std::array<int, Bits> middle{};
	int n = 0;
	static bool contains(T value) { return minimum <= value && value <= maximum; }
	static U encode(T value) { return (U(value) & mask) ^ sign; }
	static T decode(U value) {
		value ^= sign;
		if constexpr (std::is_signed_v<T> && Bits < width)
			if (value & sign) value |= U(~mask);
		return T(value);
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	std::pair<int, int> interval(int l, int r, U key) const {
		for (int d = 0; d < Bits && l < r; ++d) {
			auto [a, b] = bits[d].rank1(l, r);
			if ((key >> (Bits - 1 - d)) & 1) l = middle[d] + a, r = middle[d] + b;
			else l -= a, r -= b;
		}
		return {l, r};
	}

  public:
	using value_type = T;
	DynamicWaveletMatrix() = default;
	explicit DynamicWaveletMatrix(const std::vector<T> &values) { build(values); }
	DynamicWaveletMatrix(const DynamicWaveletMatrix &) = default;
	DynamicWaveletMatrix &operator=(const DynamicWaveletMatrix &) = default;
	DynamicWaveletMatrix(DynamicWaveletMatrix &&other) noexcept { swap(other); }
	DynamicWaveletMatrix &operator=(DynamicWaveletMatrix &&other) noexcept {
		if (this != &other) clear(), swap(other);
		return *this;
	}
	void swap(DynamicWaveletMatrix &other) noexcept {
		bits.swap(other.bits);
		middle.swap(other.middle);
		std::swap(n, other.n);
	}
	int size() const { return n; }
	bool empty() const { return !n; }
	std::size_t memory_usage() const {
		std::size_t result = 0;
		for (const auto &row : bits) result += row.memory_usage();
		return result;
	}
	void reserve(int size) {
		for (auto &row : bits) row.reserve(size);
	}
	void clear() {
		for (auto &row : bits) row.clear();
		middle.fill(0);
		n = 0;
	}
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = int(values.size());
		std::vector<U> current(n), next(n);
		for (int i = 0; i < n; ++i) {
			assert(contains(values[i]));
			current[i] = encode(values[i]);
		}
		std::vector<std::uint64_t> packed((values.size() + 63) / 64);
		for (int d = 0; d < Bits; ++d) {
			std::fill(packed.begin(), packed.end(), 0);
			int shift = Bits - 1 - d, zero = n;
			for (int i = 0; i < n; ++i) {
				bool value = (current[i] >> shift) & 1;
				packed[i >> 6] |= std::uint64_t(value) << (i & 63);
				zero -= value;
			}
			middle[d] = zero;
			bits[d].build(n, packed);
			std::array<int, 2> offset{0, zero};
			for (U value : current) next[offset[(value >> shift) & 1]++] = value;
			current.swap(next);
		}
	}
	T access(int p) const {
		assert(0 <= p && p < n);
		U result = 0;
		for (int d = 0; d < Bits; ++d) {
			auto [rank, value] = bits[d].access_rank(p);
			result = U((result << 1) | value);
			p = value ? middle[d] + rank : p - rank;
		}
		return decode(result);
	}
	T operator[](int p) const { return access(p); }
	void insert(int p, T value) {
		assert(0 <= p && p <= n && n < std::numeric_limits<int>::max() && contains(value));
		U key = encode(value);
		for (int d = 0; d < Bits; ++d) {
			bool bit = (key >> (Bits - 1 - d)) & 1;
			int rank = bits[d].insert(p, bit);
			p = bit ? middle[d] + rank : p - rank;
			middle[d] += !bit;
		}
		++n;
	}
	void push_back(T value) { insert(n, value); }
	T erase(int p) {
		assert(0 <= p && p < n);
		U result = 0;
		for (int d = 0; d < Bits; ++d) {
			auto [rank, value] = bits[d].erase_rank(p);
			result = U((result << 1) | value);
			p = value ? middle[d] + rank : p - rank;
			middle[d] -= !value;
		}
		--n;
		return decode(result);
	}
	T pop_back() { return erase(n - 1); }
	void set(int p, T value) {
		assert(0 <= p && p < n && contains(value));
		int old = p;
		U key = encode(value);
		for (int d = 0; d < Bits; ++d) {
			bool bit = (key >> (Bits - 1 - d)) & 1;
			int z = middle[d];
			if ((!z && bit) || (z == n && !bit)) continue;
			if (old == p) {
				auto [rank, previous] = bits[d].set_rank(old, bit);
				old = previous ? z + rank : old - rank;
				middle[d] += int(previous) - int(bit);
				p = bit ? middle[d] + rank : p - rank;
			} else {
				auto [rank, previous] = bits[d].erase_rank(old);
				old = previous ? z + rank : old - rank;
				middle[d] -= !previous;
				int next = bits[d].insert(p, bit);
				p = bit ? middle[d] + next : p - next;
				middle[d] += !bit;
			}
		}
	}
	int count(int l, int r, T value) const {
		check(l, r);
		if (!contains(value)) return 0;
		auto [a, b] = interval(l, r, encode(value));
		return b - a;
	}
	int rank(T value, int r) const { return count(0, r, value); }
	int range_freq(int l, int r, T upper) const {
		check(l, r);
		if (upper <= minimum) return 0;
		if (maximum < upper) return r - l;
		U key = encode(upper);
		int result = 0;
		for (int d = 0; d < Bits && l < r; ++d) {
			auto [a, b] = bits[d].rank1(l, r);
			if ((key >> (Bits - 1 - d)) & 1) {
				result += r - l - b + a;
				l = middle[d] + a, r = middle[d] + b;
			} else l -= a, r -= b;
		}
		return result;
	}
	int range_freq(int l, int r, T lower, T upper) const {
		check(l, r);
		return lower < upper ? range_freq(l, r, upper) - range_freq(l, r, lower) : 0;
	}
	T kth_smallest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		U result = 0;
		for (int d = 0; d < Bits; ++d) {
			auto [a, b] = bits[d].rank1(l, r);
			int zero = r - l - b + a;
			bool value = k >= zero;
			result = U((result << 1) | value);
			if (value) k -= zero, l = middle[d] + a, r = middle[d] + b;
			else l -= a, r -= b;
		}
		return decode(result);
	}
	T kth_largest(int l, int r, int k) const {
		check(l, r);
		assert(0 <= k && k < r - l);
		return kth_smallest(l, r, r - l - 1 - k);
	}
	std::optional<T> prev_value(int l, int r, T upper) const {
		int k = range_freq(l, r, upper);
		return k ? std::optional<T>(kth_smallest(l, r, k - 1)) : std::nullopt;
	}
	std::optional<T> next_value(int l, int r, T lower) const {
		int k = range_freq(l, r, lower);
		return k < r - l ? std::optional<T>(kth_smallest(l, r, k)) : std::nullopt;
	}
	int select(T value, int k) const {
		if (k < 0 || !contains(value)) return -1;
		U key = encode(value);
		auto [l, r] = interval(0, n, key);
		if (k >= r - l) return -1;
		int p = l + k;
		for (int d = Bits - 1; d >= 0; --d) {
			bool bit = (key >> (Bits - 1 - d)) & 1;
			p = bits[d].select(bit, p - (bit ? middle[d] : 0));
		}
		return p;
	}
};

#endif
