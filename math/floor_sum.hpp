#ifndef FLOOR_SUM_HPP
#define FLOOR_SUM_HPP

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace floor_sum_detail {
using U = std::uint64_t;

template <class R>
constexpr R triangular(U n) {
	return R(n >> 1) * R(n - 1 + (n & 1));
}

template <class R, class A>
constexpr std::pair<U, R> normalize(A value, U m) {
	U x = static_cast<U>(value);
	if constexpr (std::is_signed_v<A>) {
		if (value < 0) {
			x = U(0) - x;
			U q = x / m, r = x % m;
			return {r ? m - r : 0, R(0) - R(q) - R(r != 0)};
		}
	}
	if (x < m) return {x, R(0)};
	return {x % m, R(x / m)};
}

template <class R, class V>
constexpr R sum(V n, V m, V a, V b) {
	R result = 0;
	bool negative = false;
	for (;;) {
		R value = 0;
		if (a >= m) {
			value += triangular<R>(n) * R(a / m);
			a %= m;
		}
		if (b >= m) {
			value += R(n) * R(b / m);
			b %= m;
		}
		using W = std::conditional_t<sizeof(V) <= 4, U, __uint128_t>;
		W z = W(a) * (n - 1) + b;
		if (z < m) return result + (negative ? R(0) - value : value);
		V next;
		if constexpr (sizeof(V) <= 4) next = z / m;
		else next = z >> 64 ? V(z / m) : V(U(z) / m);
		value += R(n - 1) * R(next);
		result += negative ? R(0) - value : value;
		n = next;
		b = m - b - 1;
		std::swap(a, m);
		negative = !negative;
	}
}
}

template <class T = std::int64_t, class A, class B>
constexpr T floor_sum(std::uint64_t n, std::uint64_t m, A a, B b) {
	static_assert((std::is_integral_v<T> && !std::is_same_v<T, bool>) ||
		std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>);
	static_assert(std::is_integral_v<A> && sizeof(A) <= 8 && std::is_integral_v<B> && sizeof(B) <= 8);
	assert(m > 0);
	if (!n) return T(0);
	using R = std::conditional_t<sizeof(T) <= 8, std::uint64_t, __uint128_t>;
	auto [rb, qb] = floor_sum_detail::normalize<R>(b, m);
	if (n == 1) return static_cast<T>(qb);
	auto [ra, qa] = floor_sum_detail::normalize<R>(a, m);
	R result = floor_sum_detail::triangular<R>(n) * qa + R(n) * qb;
	if (ra) {
		if (n <= std::numeric_limits<std::uint32_t>::max() && m <= std::numeric_limits<std::uint32_t>::max())
			result += floor_sum_detail::sum<R>(std::uint32_t(n), std::uint32_t(m), std::uint32_t(ra), std::uint32_t(rb));
		else result += floor_sum_detail::sum<R>(n, m, ra, rb);
	}
	return static_cast<T>(result);
}

#endif
