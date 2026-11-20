#ifndef NTT_MOD_CORE_HPP
#define NTT_MOD_CORE_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace ntt_mod_detail {
#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx2"), always_inline))
inline __m256i load256(const void *p) { return _mm256_loadu_si256(static_cast<const __m256i *>(p)); }
__attribute__((target("avx2"), always_inline))
inline void store256(void *p, __m256i x) { _mm256_storeu_si256(static_cast<__m256i *>(p), x); }

#endif

template <std::uint32_t mod> struct Vector;

template <std::uint32_t mod>
constexpr std::uint32_t power(std::uint32_t a, std::uint32_t k) {
	std::uint32_t result = 1;
	for (; k; k >>= 1, a = std::uint64_t(a) * a % mod)
		if (k & 1) result = std::uint64_t(result) * a % mod;
	return result;
}

template <std::uint32_t mod>
void butterfly(std::uint32_t *a, std::size_t n, std::size_t half,
			   const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], v = a[start + j + half];
			std::uint32_t q = std::uint64_t(v) * quotients[j] >> 32;
			std::uint32_t y = v * roots[j] - q * mod;
			if (y >= mod) y -= mod;
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			a[start + j + half] = x >= y ? x - y : x + mod - y;
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
[[gnu::target("avx2")]]
void butterfly_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
					const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = load256(a + start + j);
			auto y = load256(a + start + j + half);
			auto r = load256(roots + j);
			auto q = load256(quotients + j);
			y = V::multiply(y, r, q);
			store256(a + start + j, V::add(x, y));
			store256(a + start + j + half, V::subtract(x, y));
		}
}
#endif

template <std::uint32_t mod>
std::uint32_t shoup(std::uint32_t value, std::uint32_t factor, std::uint32_t quotient) {
	std::uint32_t q = std::uint64_t(value) * quotient >> 32;
	std::uint32_t result = value * factor - q * mod;
	return result < mod ? result : result - mod;
}

template <std::uint32_t mod>
void forward_stage(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; ++j) {
			std::uint32_t x = a[start + j], y = a[start + j + half];
			a[start + j] = x + y < mod ? x + y : x + y - mod;
			std::uint32_t difference = x >= y ? x - y : x + mod - y;
			a[start + j + half] = shoup<mod>(difference, roots[j], quotients[j]);
		}
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
[[gnu::target("avx2")]]
void forward_stage_avx2(std::uint32_t *a, std::size_t n, std::size_t half,
		const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 2 * half)
		for (std::size_t j = 0; j < half; j += 8) {
			auto x = load256(a + start + j);
			auto y = load256(a + start + j + half);
			auto r = load256(roots + j);
			auto q = load256(quotients + j);
			auto difference = V::multiply(V::subtract(x, y), r, q);
			store256(a + start + j, V::add(x, y));
			store256(a + start + j + half, difference);
		}
}

template <std::uint32_t mod>
struct Vector {
	static constexpr std::uint32_t range = mod < (std::uint32_t(1) << 30) ? 2 * mod : mod;
	__attribute__((target("avx2"), always_inline))
	static inline __m256i add(__m256i x, __m256i y) {
		auto sum = _mm256_add_epi32(x, y);
		return _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i subtract(__m256i x, __m256i y) {
		auto difference = _mm256_sub_epi32(x, y);
		return _mm256_min_epu32(difference, _mm256_add_epi32(difference, _mm256_set1_epi32(range)));
	}
	__attribute__((target("avx2"), always_inline))
	static inline __m256i multiply(__m256i x, __m256i factor, __m256i quotient) {
		auto even = _mm256_mul_epu32(x, quotient);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(quotient, 32));
		auto q = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
		auto result = _mm256_sub_epi32(_mm256_mullo_epi32(x, factor), _mm256_mullo_epi32(q, _mm256_set1_epi32(mod)));
		if constexpr (range == 2 * mod) return result;
		return _mm256_min_epu32(result, _mm256_sub_epi32(result, _mm256_set1_epi32(mod)));
	}
};

template <std::uint32_t mod, bool inverse>
[[gnu::target("avx2")]]
void leaf_avx2(std::uint32_t *a, std::size_t n, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	auto r4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(roots + 4)));
	auto q4 = _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i *>(quotients + 4)));
	auto r2 = _mm256_setr_epi32(roots[2], roots[3], roots[2], roots[3], roots[2], roots[3], roots[2], roots[3]);
	auto q2 = _mm256_setr_epi32(quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3], quotients[2], quotients[3]);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = load256(a + i);
		if constexpr (!inverse) {
			auto y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r4, q4), 0xf0);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::multiply(V::subtract(y, x), r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
		} else {
			auto y = _mm256_shuffle_epi32(x, 0xb1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xaa);
			x = _mm256_blend_epi32(x, V::multiply(x, r2, q2), 0xcc);
			y = _mm256_shuffle_epi32(x, 0x4e);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xcc);
			x = _mm256_blend_epi32(x, V::multiply(x, r4, q4), 0xf0);
			y = _mm256_permute2x128_si256(x, x, 1);
			x = _mm256_blend_epi32(V::add(x, y), V::subtract(y, x), 0xf0);
		}
		store256(a + i, x);
	}
}

template <std::uint32_t mod, bool inverse>
[[gnu::target("avx2")]]
void pair_avx2(std::uint32_t *a, std::size_t n, std::size_t h, const std::uint32_t *roots, const std::uint32_t *quotients) {
	using V = Vector<mod>;
	for (std::size_t start = 0; start < n; start += 4 * h)
		for (std::size_t j = 0; j < h; j += 8) {
			auto x0 = load256(a + start + j);
			auto x1 = load256(a + start + j + h);
			auto x2 = load256(a + start + j + 2 * h);
			auto x3 = load256(a + start + j + 3 * h);
			auto r0 = load256(roots + 2 * h + j);
			auto q0 = load256(quotients + 2 * h + j);
			auto r1 = load256(roots + 3 * h + j);
			auto q1 = load256(quotients + 3 * h + j);
			auto r2 = load256(roots + h + j);
			auto q2 = load256(quotients + h + j);
			if constexpr (!inverse) {
				auto y0 = V::add(x0, x2), y1 = V::add(x1, x3);
				auto y2 = V::multiply(V::subtract(x0, x2), r0, q0);
				auto y3 = V::multiply(V::subtract(x1, x3), r1, q1);
				x0 = V::add(y0, y1); x1 = V::multiply(V::subtract(y0, y1), r2, q2);
				x2 = V::add(y2, y3); x3 = V::multiply(V::subtract(y2, y3), r2, q2);
			} else {
				x1 = V::multiply(x1, r2, q2); x3 = V::multiply(x3, r2, q2);
				auto y0 = V::add(x0, x1), y1 = V::subtract(x0, x1);
				auto y2 = V::multiply(V::add(x2, x3), r0, q0);
				auto y3 = V::multiply(V::subtract(x2, x3), r1, q1);
				x0 = V::add(y0, y2); x2 = V::subtract(y0, y2);
				x1 = V::add(y1, y3); x3 = V::subtract(y1, y3);
			}
			store256(a + start + j, x0);
			store256(a + start + j + h, x1);
			store256(a + start + j + 2 * h, x2);
			store256(a + start + j + 3 * h, x3);
		}
}

template <std::uint32_t mod>
[[gnu::target("avx2")]]
void build_roots_avx2(std::uint32_t *roots, std::uint32_t *quotients, std::size_t half, std::uint32_t step) {
	std::uint32_t initial[8], w = 1;
	for (int i = 0; i < 8; ++i) {
		initial[i] = w;
		w = std::uint64_t(w) * step % mod;
	}
	auto x = load256(initial);
	auto factor = _mm256_set1_epi32(w), quotient = _mm256_set1_epi32((std::uint64_t(w) << 32) / mod);
	for (std::size_t j = 0; j < half; j += 8) {
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(mod)));
		store256(roots + j, x);
		for (int k = 0; k < 8; ++k) quotients[j + k] = (std::uint64_t(roots[j + k]) << 32) / mod;
		x = Vector<mod>::multiply(x, factor, quotient);
	}
}
#endif

template <std::uint32_t mod, std::uint32_t root>
struct Plan {
	std::vector<std::uint32_t> roots, quotients;
	explicit Plan(std::size_t n, [[maybe_unused]] bool vectorized = false) : roots(n), quotients(n) {
		static_assert(mod > 2 && mod < (std::uint32_t(1) << 31));
		assert(std::has_single_bit(n) && (mod - 1) % n == 0);
		if (n == 1) return;
		std::size_t half = n / 2;
		std::uint32_t step = power<mod>(root, (mod - 1) / n), w = 1;
#if defined(__x86_64__) || defined(__i386__)
		if (vectorized && half >= 8) build_roots_avx2<mod>(roots.data() + half, quotients.data() + half, half, step);
		else
#endif
		for (std::size_t j = 0; j < half; ++j) {
			roots[half + j] = w;
			quotients[half + j] = (std::uint64_t(w) << 32) / mod;
			w = std::uint64_t(w) * step % mod;
		}
		for (half /= 2; half; half /= 2)
			for (std::size_t j = 0; j < half; ++j) {
				roots[half + j] = roots[2 * half + 2 * j];
				quotients[half + j] = quotients[2 * half + 2 * j];
			}
	}
};

template <std::uint32_t mod, std::uint32_t root>
void forward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = a.size() / 2;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		for (; half >= 16; half /= 4) pair_avx2<mod, false>(a.data(), a.size(), half / 2, plan.roots.data(), plan.quotients.data());
		if (half == 8) forward_stage_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		leaf_avx2<mod, false>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		return;
	}
#endif
	for (; half; half /= 2) forward_stage<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

template <std::uint32_t mod, std::uint32_t root>
void backward(std::vector<std::uint32_t> &a, const Plan<mod, root> &plan, [[maybe_unused]] bool vectorized) {
	std::size_t half = 1;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		leaf_avx2<mod, true>(a.data(), a.size(), plan.roots.data(), plan.quotients.data());
		half = 8;

		for (; 2 * half < a.size(); half *= 4) pair_avx2<mod, true>(a.data(), a.size(), half, plan.roots.data(), plan.quotients.data());
		if (half < a.size()) butterfly_avx2<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
		return;
	}
#endif
	for (; half < a.size(); half *= 2) butterfly<mod>(a.data(), a.size(), half, plan.roots.data() + half, plan.quotients.data() + half);
}

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
[[gnu::target("avx2")]]
void scale_avx2(std::uint32_t *a, std::size_t n, std::uint32_t factor, std::uint32_t quotient) {
	auto r = _mm256_set1_epi32(factor), q = _mm256_set1_epi32(quotient);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = load256(a + i);
		auto y = Vector<mod>::multiply(x, r, q);
		store256(a + i, _mm256_min_epu32(y, _mm256_sub_epi32(y, _mm256_set1_epi32(mod))));
	}
}
#endif

template <std::uint32_t mod>
void scale(std::vector<std::uint32_t> &a, std::uint32_t factor, [[maybe_unused]] bool vectorized) {
	std::uint32_t quotient = (std::uint64_t(factor) << 32) / mod;
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		scale_avx2<mod>(a.data(), a.size(), factor, quotient);
		return;
	}
#endif
	for (auto &x : a) x = shoup<mod>(x, factor, quotient);
}

template <std::uint32_t mod>
constexpr std::uint32_t negative_inverse = [] {
	std::uint32_t value = mod;
	for (int i = 0; i < 5; ++i) value *= 2 - mod * value;
	return 0 - value;
}();

#if defined(__x86_64__) || defined(__i386__)
template <std::uint32_t mod>
[[gnu::target("avx2")]]
void pointwise_avx2(std::uint32_t *a, const std::uint32_t *b, std::size_t n) {
	auto p = _mm256_set1_epi32(mod), inverse = _mm256_set1_epi32(negative_inverse<mod>);
	for (std::size_t i = 0; i < n; i += 8) {
		auto x = load256(a + i);
		auto y = load256(b + i);
		auto even = _mm256_mul_epu32(x, y);
		auto odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
		auto qe = _mm256_mul_epu32(even, inverse), qo = _mm256_mul_epu32(odd, inverse);
		even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(qe, p)), 32);
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(qo, p));
		x = _mm256_blend_epi32(even, odd, 0xaa);
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		store256(a + i, x);
	}
}
#endif

template <std::uint32_t mod>
void pointwise(std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, [[maybe_unused]] bool vectorized) {
#if defined(__x86_64__) || defined(__i386__)
	if (vectorized && a.size() >= 8) {
		pointwise_avx2<mod>(a.data(), b.data(), a.size());
		return;
	}
#endif
	for (std::size_t i = 0; i < a.size(); ++i) {
		std::uint64_t product = std::uint64_t(a[i]) * b[i];
		std::uint32_t q = std::uint32_t(product) * negative_inverse<mod>;
		std::uint32_t value = (product + std::uint64_t(q) * mod) >> 32;
		a[i] = value < mod ? value : value - mod;
	}
}

}

#endif
