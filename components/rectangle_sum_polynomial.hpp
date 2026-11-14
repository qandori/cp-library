#ifndef RECTANGLE_SUM_POLYNOMIAL_HPP
#define RECTANGLE_SUM_POLYNOMIAL_HPP

#include <cstdint>
#include <type_traits>
#if defined(__x86_64__)
#include <emmintrin.h>
#endif

namespace mint_detail { struct Mint; }

namespace rectangle_sum_detail {

template <class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct Storage { using type = T; };

template <class T>
struct Storage<T, true> { using type = std::conditional_t<(sizeof(T) < sizeof(unsigned)), unsigned, std::make_unsigned_t<T>>; };

#if defined(__SIZEOF_INT128__)
template <>
struct Storage<__int128_t, false> { using type = __uint128_t; };
#endif

template <class T>
struct Polynomial {
	T weight{}, x{}, y{}, xy{};
	static Polynomial corner(const T &a, const T &b, const T &value) {
		T ax = value * a, by = value * b;
		return {value, ax, by, ax * b};
	}
	Polynomial &operator+=(const Polynomial &other) {
#if defined(__x86_64__)
		if constexpr (std::is_same_v<T, mint_detail::Mint>) {
			static_assert(sizeof(Polynomial) == 16 && sizeof(T) == 4);
			auto modulus = _mm_set1_epi32(2 * 998244353);
			auto sum = _mm_add_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i *>(this)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(&other)));
			auto difference = _mm_sub_epi32(sum, modulus);
			_mm_storeu_si128(reinterpret_cast<__m128i *>(this), _mm_add_epi32(difference, _mm_and_si128(_mm_srai_epi32(difference, 31), modulus)));
			return *this;
		}
#endif
		weight += other.weight; x += other.x; y += other.y; xy += other.xy;
		return *this;
	}
	Polynomial &operator-=(const Polynomial &other) {
#if defined(__x86_64__)
		if constexpr (std::is_same_v<T, mint_detail::Mint>) {
			static_assert(sizeof(Polynomial) == 16 && sizeof(T) == 4);
			auto modulus = _mm_set1_epi32(2 * 998244353);
			auto difference = _mm_sub_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i *>(this)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(&other)));
			_mm_storeu_si128(reinterpret_cast<__m128i *>(this), _mm_add_epi32(difference, _mm_and_si128(_mm_srai_epi32(difference, 31), modulus)));
			return *this;
		}
#endif
		weight -= other.weight; x -= other.x; y -= other.y; xy -= other.xy;
		return *this;
	}
	T evaluate(const T &a, const T &b) const { return (weight * a - x) * b - y * a + xy; }
};

}

#endif
