#ifndef NTT_MOD_HPP
#define NTT_MOD_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ntt_mod_detail {
template <std::uint32_t mod>
std::uint32_t power(std::uint32_t a, std::uint32_t k) {
	std::uint32_t result = 1;
	for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
		if (k & 1) result = std::uint64_t(result) * a % mod;
	return result;
}
}

template <std::uint32_t mod, std::uint32_t root>
void ntt_mod(std::vector<std::uint32_t> &a, bool inverse = false) {
	static_assert(mod > 2 && mod < (std::uint32_t(1) << 31));
	std::size_t n = a.size();
	assert(std::has_single_bit(n) && (mod - 1) % n == 0);
	for (std::size_t i = 1, j = 0; i < n; ++i) {
		std::size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	for (std::size_t width = 2; width <= n; width *= 2) {
		std::uint32_t step = ntt_mod_detail::power<mod>(root, (mod - 1) / width);
		if (inverse) step = ntt_mod_detail::power<mod>(step, mod - 2);
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
		std::uint32_t inv = ntt_mod_detail::power<mod>(n, mod - 2);
		for (auto &x : a) x = std::uint64_t(x) * inv % mod;
	}
}

#endif
