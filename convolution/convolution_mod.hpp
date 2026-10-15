#ifndef CONVOLUTION_MOD_HPP
#define CONVOLUTION_MOD_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

namespace convolution_mod_detail {
template <std::uint32_t mod>
std::uint32_t power(std::uint32_t a, std::uint32_t k) {
	std::uint32_t result = 1;
	for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
		if (k & 1) result = std::uint64_t(result) * a % mod;
	return result;
}
template <std::uint32_t mod, std::uint32_t root>
void ntt(std::vector<std::uint32_t> &a, bool inverse) {
	std::size_t n = a.size();
	assert(std::has_single_bit(n) && (mod - 1) % n == 0);
	for (std::size_t i = 1, j = 0; i < n; ++i) {
		std::size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	for (std::size_t width = 2; width <= n; width *= 2) {
		std::uint32_t step = power<mod>(root, (mod - 1) / width);
		if (inverse) step = power<mod>(step, mod - 2);
		for (std::size_t start = 0; start < n; start += width) {
			std::uint32_t w = 1;
			for (std::size_t j = 0; j < width / 2; ++j) {
				std::uint32_t x = a[start + j], y = std::uint64_t(a[start + j + width / 2]) * w % mod;
				a[start + j] = x + y < mod ? x + y : x + y - mod;
				a[start + j + width / 2] = x >= y ? x - y : x + mod - y;
				w = std::uint64_t(w) * step % mod;
			}
		}
	}
	if (inverse) {
		std::uint32_t inv = power<mod>(n, mod - 2);
		for (auto &x : a) x = std::uint64_t(x) * inv % mod;
	}
}
template <std::uint32_t mod, std::uint32_t root, class T>
std::vector<std::uint32_t> multiply(const std::vector<T> &a, const std::vector<T> &b) {
	std::size_t length = a.size() + b.size() - 1, n = std::bit_ceil(length);
	std::vector<std::uint32_t> x(n), y(n);
	auto reduce = [](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	for (std::size_t i = 0; i < a.size(); ++i) x[i] = reduce(a[i]);
	for (std::size_t i = 0; i < b.size(); ++i) y[i] = reduce(b[i]);
	ntt<mod, root>(x, false);
	if (&a == &b) y = x;
	else ntt<mod, root>(y, false);
	for (std::size_t i = 0; i < n; ++i) x[i] = std::uint64_t(x[i]) * y[i] % mod;
	ntt<mod, root>(x, true);
	x.resize(length);
	return x;
}
inline constexpr std::uint32_t p = 754974721, q = 1224736769, r = 2013265921;
inline constexpr __uint128_t modulus = __uint128_t(p) * q * r;
inline __uint128_t restore(std::uint32_t a, std::uint32_t b, std::uint32_t c) {
	static const std::uint32_t inverse_p = power<q>(p, q - 2);
	static const std::uint32_t inverse_pq = power<r>(std::uint64_t(p) * q % r, r - 2);
	std::uint64_t x = std::uint64_t(b + q - a) * inverse_p % q;
	std::uint64_t low = a + std::uint64_t(p) * x;
	std::uint64_t y = std::uint64_t(c + r - low % r) * inverse_pq % r;
	return low + __uint128_t(p) * q * y;
}
}

template <class T>
std::vector<T> convolution_integer(const std::vector<T> &a, const std::vector<T> &b) {
	static_assert(std::numeric_limits<T>::is_integer && sizeof(T) <= 8);
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() - 1 <= std::size_t(1) << 24);
	auto x = convolution_mod_detail::multiply<convolution_mod_detail::p, 11>(a, b);
	auto y = convolution_mod_detail::multiply<convolution_mod_detail::q, 3>(a, b);
	auto z = convolution_mod_detail::multiply<convolution_mod_detail::r, 31>(a, b);
	std::vector<T> result(x.size());
	for (std::size_t i = 0; i < x.size(); ++i) {
		auto value = convolution_mod_detail::restore(x[i], y[i], z[i]);
		if constexpr (std::numeric_limits<T>::is_signed) {
			__int128_t v = value > convolution_mod_detail::modulus / 2 ? __int128_t(value) - __int128_t(convolution_mod_detail::modulus) : __int128_t(value);
			assert(std::numeric_limits<T>::lowest() <= v && v <= std::numeric_limits<T>::max());
			result[i] = T(v);
		} else {
			assert(value <= std::numeric_limits<T>::max());
			result[i] = T(value);
		}
	}
	return result;
}

template <class T>
std::vector<std::uint32_t> convolution_mod(const std::vector<T> &a, const std::vector<T> &b, std::uint32_t mod) {
	static_assert(std::numeric_limits<T>::is_integer && sizeof(T) <= 8);
	assert(mod > 0);
	if (a.empty() || b.empty()) return {};
	assert(a.size() + b.size() - 1 <= std::size_t(1) << 24);
	std::vector<std::uint32_t> x(a.size()), y(b.size());
	auto reduce = [=](T value) {
		__int128_t r = __int128_t(value) % mod;
		return std::uint32_t(r < 0 ? r + mod : r);
	};
	std::transform(a.begin(), a.end(), x.begin(), reduce);
	std::transform(b.begin(), b.end(), y.begin(), reduce);
	if (mod == 998244353 && x.size() + y.size() - 1 <= std::size_t(1) << 23) return convolution_mod_detail::multiply<998244353, 3>(x, y);
	auto p = convolution_mod_detail::multiply<convolution_mod_detail::p, 11>(x, y);
	auto q = convolution_mod_detail::multiply<convolution_mod_detail::q, 3>(x, y);
	auto r = convolution_mod_detail::multiply<convolution_mod_detail::r, 31>(x, y);
	for (std::size_t i = 0; i < p.size(); ++i) p[i] = convolution_mod_detail::restore(p[i], q[i], r[i]) % mod;
	return p;
}

#endif
