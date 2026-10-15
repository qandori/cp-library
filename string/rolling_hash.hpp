#ifndef ROLLING_HASH_HPP
#define ROLLING_HASH_HPP

#include "../math/splitmix64.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

class RollingHash {
  public:
	using Hash = std::uint64_t;
	struct Value {
		Hash hash = 0;
		int length = 0;
		bool operator==(const Value &) const = default;
	};
	static constexpr Hash mod = (Hash(1) << 61) - 1;
	static Hash base() {
		static const Hash value = splitmix64(Hash(std::chrono::steady_clock::now().time_since_epoch().count()) ^ Hash(reinterpret_cast<std::uintptr_t>(&powers))) % (mod - 512) + 256;
		return value;
	}

  private:
	inline static std::vector<Hash> powers{1};
	std::vector<Hash> prefix;
	static Hash reduce(__uint128_t value) {
		Hash result = (Hash(value) & mod) + Hash(value >> 61);
		return result >= mod ? result - mod : result;
	}
	static Hash mul(Hash a, Hash b) { return reduce(__uint128_t(a) * b); }
	static void ensure_powers(int n) {
		std::size_t old = powers.size();
		if (std::size_t(n) < old) return;
		powers.resize(std::size_t(n) + 1);
		Hash b = base();
		for (std::size_t i = old; i < powers.size(); ++i) powers[i] = mul(powers[i - 1], b);
	}
	template <class T>
	static Hash encode(T value) {
		static_assert(std::is_integral_v<T> && sizeof(T) <= 8);
		if constexpr (std::is_same_v<T, char>) return static_cast<unsigned char>(value) + 1;
		else {
			Hash result;
			if constexpr (std::is_signed_v<T>) {
				std::int64_t x = std::int64_t(value) % std::int64_t(mod);
				result = x < 0 ? Hash(x + std::int64_t(mod)) : Hash(x);
			} else result = Hash(value) % mod;
			return result == mod - 1 ? 0 : result + 1;
		}
	}
	template <class Range>
	void build_range(const Range &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		int n = int(values.size());
		ensure_powers(n);
		prefix.resize(std::size_t(n) + 1);
		Hash value = 0, b = base();
		prefix[0] = 0;
		int i = 0;
		for (typename Range::value_type x : values) prefix[++i] = value = reduce(__uint128_t(value) * b + encode(x));
	}

  public:
	RollingHash() = default;
	explicit RollingHash(std::string_view s) { build(s); }
	template <class T>
	explicit RollingHash(const std::vector<T> &values) { build(values); }
	void build(std::string_view s) { build_range(s); }
	template <class T>
	void build(const std::vector<T> &values) { build_range(values); }
	int size() const { return prefix.empty() ? 0 : int(prefix.size() - 1); }
	bool empty() const { return !size(); }
	static void reserve(int n) {
		assert(n >= 0);
		ensure_powers(n);
	}
	template <class T>
	static Value single(T x) { return {encode(x), 1}; }
	static Value e() { return {}; }
	static Value op(Value left, Value right) {
		assert(left.length >= 0 && right.length >= 0 && left.length <= std::numeric_limits<int>::max() - right.length);
		if (!left.length) return right;
		if (!right.length) return left;
		return {concat(left.hash, right.hash, right.length), left.length + right.length};
	}
	Hash get() const { return prefix.empty() ? 0 : prefix.back(); }
	Hash get(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		if (!l) return r ? prefix[r] : 0;
		Hash value = prefix[r] + mod - mul(prefix[l], powers[r - l]);
		return value >= mod ? value - mod : value;
	}
	Value value() const { return {get(), size()}; }
	Value value(int l, int r) const { return {get(l, r), r - l}; }
	static Hash concat(Hash left, Hash right, int right_length) {
		assert(left < mod && right < mod && right_length >= 0);
		ensure_powers(right_length);
		return reduce(__uint128_t(left) * powers[right_length] + right);
	}
	int lcp(int l, int r, const RollingHash &other, int other_l, int other_r) const {
		assert(0 <= l && l <= r && r <= size() && 0 <= other_l && other_l <= other_r && other_r <= other.size());
		int low = 0, high = std::min(r - l, other_r - other_l);
		while (low < high) {
			int mid = high - (high - low) / 2;
			if (get(l, l + mid) == other.get(other_l, other_l + mid)) low = mid;
			else high = mid - 1;
		}
		return low;
	}
	int lcp(int l, const RollingHash &other, int other_l) const { return lcp(l, size(), other, other_l, other.size()); }
};

#endif
