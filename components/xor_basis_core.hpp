#ifndef XOR_BASIS_CORE_HPP
#define XOR_BASIS_CORE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace xor_basis_detail {
template <class UInt, int Bits>
struct Core {
	static_assert(std::is_integral_v<UInt> && std::is_unsigned_v<UInt> && !std::is_same_v<UInt, bool>);
	static constexpr int digits = std::numeric_limits<UInt>::digits;
	static_assert(0 < Bits && Bits <= digits && digits <= 64);
	static constexpr UInt mask = UInt(UInt(~UInt(0)) >> (digits - Bits));
	std::array<UInt, Bits> row{};
	std::array<std::uint8_t, Bits> at{};
	int dimension = 0;
	static bool valid(UInt x) { return (x & mask) == x; }
	static int pivot(UInt x) { return int(std::bit_width(std::uint64_t(x))) - 1; }
	int insert(UInt x) {
		assert(valid(x));
		if (dimension == Bits) return -1;
		for (int i = 0; i < dimension; ++i) x = std::min(x, UInt(x ^ row[i]));
		if (!x) return -1;
		row[dimension] = x; at[pivot(x)] = std::uint8_t(dimension + 1);
		return dimension++;
	}
	UInt minimize(UInt x) const {
		if (dimension == Bits) return UInt(x & UInt(~mask));
		for (int i = 0; i < dimension; ++i) x = std::min(x, UInt(x ^ row[i]));
		return x;
	}
	UInt maximize(UInt x) const {
		if (dimension == Bits) return UInt(x | mask);
		for (int i = 0; i < dimension; ++i) x = std::max(x, UInt(x ^ row[i]));
		return x;
	}
};
}

#endif
