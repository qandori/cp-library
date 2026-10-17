#ifndef XOR_BASIS_HPP
#define XOR_BASIS_HPP

#include "../components/xor_basis_core.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

template <class UInt, int Bits> class RollbackXorBasis;
template <class UInt, int Bits> class PrefixXorBasis;
template <class UInt, int Bits> class XorBasisWithWitness;

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class XorBasis {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	Core core;
	friend class RollbackXorBasis<UInt, Bits>;
	friend class PrefixXorBasis<UInt, Bits>;
	friend class XorBasisWithWitness<UInt, Bits>;

  public:
	using value_type = UInt;
	using count_type = __uint128_t;
	static constexpr int bits = Bits;
	XorBasis() = default;
	XorBasis(std::initializer_list<UInt> values) { for (UInt x : values) insert(x); }
	explicit XorBasis(const std::vector<UInt> &values) { for (UInt x : values) insert(x); }
	int rank() const { return core.dimension; }
	bool empty() const { return rank() == 0; }
	count_type count() const { return count_type(1) << rank(); }
	void clear() { core = Core{}; }
	bool insert(UInt x) { return core.insert(x) >= 0; }
	UInt reduce(UInt x) const { return core.minimize(x); }
	bool contains(UInt x) const { return reduce(x) == 0; }
	UInt min_xor(UInt x = 0) const { return core.minimize(x); }
	UInt max_xor(UInt x = 0) const { return core.maximize(x); }
	count_type mex(UInt x = 0) const {
		if (!contains(x)) return 0;
		if (rank() == Bits) return count_type(1) << Bits;
		for (int i = 0; i < Bits; ++i) if (!core.at[i]) return count_type(1) << i;
		return count_type(1) << Bits;
	}
	std::optional<UInt> min_nonzero() const {
		if (empty()) return std::nullopt;
		return *std::min_element(core.row.begin(), core.row.begin() + rank());
	}
	std::vector<UInt> vectors() const {
		return {core.row.begin(), core.row.begin() + rank()};
	}
	XorBasis canonical() const {
		XorBasis result = *this;
		std::sort(result.core.row.begin(), result.core.row.begin() + rank());
		result.core.at.fill(0);
		for (int i = 0; i < rank(); ++i) {
			int p = Core::pivot(result.core.row[i]);
			result.core.at[p] = std::uint8_t(i + 1);
			for (int j = i + 1; j < rank(); ++j)
				if ((result.core.row[j] >> p) & 1) result.core.row[j] ^= result.core.row[i];
		}
		return result;
	}
	std::optional<UInt> coordinates(UInt x) const {
		UInt result = 0;
		for (int i = 0; i < rank(); ++i) if (UInt(x ^ core.row[i]) < x) {
			x ^= core.row[i]; result |= UInt(UInt(1) << i);
		}
		if (x) return std::nullopt;
		return result;
	}
	UInt combine(UInt coefficients) const {
		UInt result = 0;
		for (int i = 0; i < rank(); ++i) {
			if (coefficients & 1) result ^= core.row[i];
			coefficients >>= 1;
		}
		assert(!coefficients);
		return result;
	}
	std::optional<UInt> kth(count_type k, UInt x = 0) const {
		if (k >= count()) return std::nullopt;
		if (rank() == Bits) return UInt((x & UInt(~Core::mask)) | UInt(k));
		int remaining = rank();
		for (int i = Bits - 1; i >= 0; --i) if (core.at[i]) {
			--remaining;
			if (((x >> i) & 1) != ((k >> remaining) & 1)) x ^= core.row[core.at[i] - 1];
		}
		return x;
	}
	count_type count_less(count_type value, UInt x = 0) const {
		if (value >= (count_type(1) << Core::digits)) return count();
		UInt bound = UInt(value);
		UInt high = UInt(x & UInt(~Core::mask)), bound_high = UInt(bound & UInt(~Core::mask));
		if (high != bound_high) return high < bound_high ? count() : 0;
		if (rank() == Bits) return UInt(bound & Core::mask);
		count_type result = 0;
		int remaining = rank();
		for (int i = Bits - 1; i >= 0; --i) {
			bool target = (bound >> i) & 1;
			if (core.at[i]) {
				--remaining;
				if (target) result += count_type(1) << remaining;
				if (bool((x >> i) & 1) != target) x ^= core.row[core.at[i] - 1];
			} else if (bool((x >> i) & 1) != target) return result + (target ? count_type(1) << remaining : 0);
		}
		return result;
	}
	count_type count_leq(count_type value, UInt x = 0) const {
		if (value >= std::numeric_limits<UInt>::max()) return count();
		return count_less(value + 1, x);
	}
	count_type count_range(count_type lower, count_type upper, UInt x = 0) const {
		assert(lower <= upper);
		return count_less(upper, x) - count_less(lower, x);
	}
	std::optional<count_type> index_of(count_type value, UInt x = 0) const {
		if (value > std::numeric_limits<UInt>::max() || !contains(UInt(UInt(value) ^ x))) return std::nullopt;
		return count_less(value, x);
	}
	std::optional<UInt> lower_bound(count_type value, UInt x = 0) const { return kth(count_less(value, x), x); }
	std::optional<UInt> upper_bound(count_type value, UInt x = 0) const { return kth(count_leq(value, x), x); }
	std::optional<UInt> predecessor(count_type value, UInt x = 0) const {
		count_type k = count_less(value, x);
		return k ? kth(k - 1, x) : std::nullopt;
	}
	int merge(const XorBasis &other) {
		int before = rank();
		if (this != &other) for (int i = 0; i < other.rank(); ++i) {
			insert(other.core.row[i]);
			if (rank() == Bits) break;
		}
		return rank() - before;
	}
	bool is_subspace_of(const XorBasis &other) const {
		if (rank() > other.rank()) return false;
		for (int i = 0; i < rank(); ++i) if (!other.contains(core.row[i])) return false;
		return true;
	}
	bool operator==(const XorBasis &other) const { return rank() == other.rank() && is_subspace_of(other); }
	XorBasis intersection(const XorBasis &other) const {
		if (core.dimension == Bits || this == &other) return other;
		if (other.core.dimension == Bits) return *this;
		XorBasis result;
		if (empty() || other.empty()) return result;
		if (rank() < other.rank()) return other.intersection(*this);
		if constexpr (Bits <= 32) {
			std::array<std::uint64_t, Bits> work;
			int n = rank();
			for (int i = 0; i < n; ++i) work[i] = std::uint64_t(core.row[i]) << 32;
			for (int j = 0; j < other.rank(); ++j) {
				UInt value = other.core.row[j];
				std::uint64_t x = (std::uint64_t(value) << 32) | value;
				for (int i = 0; i < n; ++i) x = std::min(x, x ^ work[i]);
				if (x >> 32) work[n++] = x;
				else result.insert(UInt(x));
			}
			return result;
		}
		std::array<UInt, Bits> row{};
		for (int i = 0; i < rank(); ++i) row[Core::pivot(core.row[i])] = core.row[i];
		std::array<UInt, Bits> source{};
		for (int j = 0; j < other.rank(); ++j) {
			UInt value = other.core.row[j];
			UInt x = value, y = value;
			while (x) {
				int i = Core::pivot(x);
				if (!row[i]) { row[i] = x; source[i] = y; break; }
				x ^= row[i]; y ^= source[i];
			}
			if (!x) result.insert(y);
		}
		return result;
	}
	XorBasis orthogonal_complement() const {
		auto reduced = canonical();
		XorBasis result;
		for (int j = 0; j < Bits; ++j) if (!reduced.core.at[j]) {
			UInt x = UInt(UInt(1) << j);
			for (int i = 0; i < rank(); ++i) if ((reduced.core.row[i] >> j) & 1) x |= UInt(UInt(1) << Core::pivot(reduced.core.row[i]));
			result.insert(x);
		}
		return result;
	}
	XorBasis &operator|=(const XorBasis &other) { merge(other); return *this; }
	XorBasis &operator&=(const XorBasis &other) { return *this = intersection(other); }
	friend XorBasis operator|(XorBasis a, const XorBasis &b) { a.merge(b); return a; }
	friend XorBasis operator&(const XorBasis &a, const XorBasis &b) { return a.intersection(b); }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class RollbackXorBasis {
	XorBasis<UInt, Bits> current;
	std::vector<int> history;

  public:
	const XorBasis<UInt, Bits> &basis() const { return current; }
	std::size_t snapshot() const { return history.size(); }
	bool insert(UInt x) {
		history.push_back(-1);
		int i = current.core.insert(x);
		history.back() = i;
		return i >= 0;
	}
	void undo() {
		assert(!history.empty());
		int i = history.back(); history.pop_back();
		if (i >= 0) {
			current.core.at[current.core.pivot(current.core.row[i])] = 0;
			current.core.row[i] = 0; --current.core.dimension;
		}
	}
	void rollback(std::size_t token) { assert(token <= history.size()); while (history.size() > token) undo(); }
	void clear() { current.clear(); history.clear(); }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class PrefixXorBasis {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	std::array<UInt, Bits> row{};
	std::array<std::size_t, Bits> position{};
	std::size_t length = 0;

  public:
	std::size_t size() const { return length; }
	void clear() { row.fill(0); position.fill(0); length = 0; }
	bool push_back(UInt x) {
		assert(Core::valid(x) && length < std::numeric_limits<std::size_t>::max());
		std::size_t p = ++length;
		while (x) {
			int i = Core::pivot(x);
			if (!row[i]) { row[i] = x; position[i] = p; return true; }
			if (position[i] < p) { std::swap(position[i], p); std::swap(row[i], x); }
			x ^= row[i];
		}
		return false;
	}
	XorBasis<UInt, Bits> basis(std::size_t lower = 0) const {
		assert(lower <= length);
		XorBasis<UInt, Bits> result;
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) {
			int k = result.core.dimension++;
			result.core.row[k] = row[i]; result.core.at[i] = std::uint8_t(k + 1);
		}
		return result;
	}
	int rank(std::size_t lower = 0) const {
		assert(lower <= length);
		int result = 0;
		for (std::size_t p : position) result += p > lower;
		return result;
	}
	UInt min_xor(std::size_t lower = 0, UInt x = 0) const {
		assert(lower <= length);
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) x = std::min(x, UInt(x ^ row[i]));
		return x;
	}
	UInt max_xor(std::size_t lower = 0, UInt x = 0) const {
		assert(lower <= length);
		for (int i = Bits - 1; i >= 0; --i) if (position[i] > lower) x = std::max(x, UInt(x ^ row[i]));
		return x;
	}
	bool contains(std::size_t lower, UInt x) const { return min_xor(lower, x) == 0; }
};

template <class UInt = std::uint64_t, int Bits = std::numeric_limits<UInt>::digits>
class XorBasisWithWitness {
	using Core = xor_basis_detail::Core<UInt, Bits>;
	XorBasis<UInt, Bits> current;
	std::array<UInt, Bits> source{};
	std::array<std::size_t, Bits> index{};
	std::size_t length = 0;

  public:
	const XorBasis<UInt, Bits> &basis() const { return current; }
	std::size_t size() const { return length; }
	void clear() { current.clear(); source.fill(0); length = 0; }
	bool insert(UInt x) {
		assert(Core::valid(x) && length < std::numeric_limits<std::size_t>::max());
		std::size_t id = length++;
		if (current.rank() == Bits) return false;
		UInt mask = UInt(UInt(1) << current.rank());
		for (int i = 0; i < current.rank(); ++i) if (UInt(x ^ current.core.row[i]) < x) {
			x ^= current.core.row[i]; mask ^= source[i];
		}
		if (!x) return false;
		int k = current.core.dimension++;
		index[k] = id; source[k] = mask; current.core.row[k] = x;
		current.core.at[Core::pivot(x)] = std::uint8_t(k + 1);
		return true;
	}
	std::optional<std::vector<std::size_t>> represent(UInt x) const {
		if (!Core::valid(x)) return std::nullopt;
		UInt mask = 0;
		for (int i = 0; i < current.rank(); ++i) if (UInt(x ^ current.core.row[i]) < x) {
			x ^= current.core.row[i]; mask ^= source[i];
		}
		if (x) return std::nullopt;
		std::vector<std::size_t> result;
		while (mask) {
			int i = std::countr_zero(std::uint64_t(mask));
			result.push_back(index[i]); mask &= UInt(mask - 1);
		}
		return result;
	}
};

#endif
