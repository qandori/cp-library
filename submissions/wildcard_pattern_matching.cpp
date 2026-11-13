#line 1 "test/wildcard_pattern_matching.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/wildcard_pattern_matching"
#line 1 "string/wildcard_pattern_matching.hpp"



#line 1 "components/wildcard_pattern_matching.hpp"



#line 1 "components/ntt_mod_core.hpp"



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
std::uint32_t power(std::uint32_t a, std::uint32_t k) {
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


#line 8 "components/wildcard_pattern_matching.hpp"
#include <stdexcept>
#line 10 "components/wildcard_pattern_matching.hpp"

namespace wildcard_matching_detail {

template <std::uint32_t mod>
void filter_prime(const std::vector<std::uint16_t> &text, const std::vector<std::uint16_t> &pattern,
		std::vector<bool> &matches, bool text_has_wildcard, bool pattern_has_wildcard) {
	std::size_t m = pattern.size(), length = std::bit_ceil(std::min(text.size(), 2 * m - 1));
	bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
	vectorized = __builtin_cpu_supports("avx2");
#endif
	ntt_mod_detail::Plan<mod, 3> plan(length, vectorized);
	std::vector<std::uint32_t> values(length), mask(pattern_has_wildcard ? length : 0), squares(text_has_wildcard ? length : 0);
	std::uint64_t pattern_square_sum = 0;
	for (std::size_t i = 0; i < m; ++i) {
		auto value = pattern[m - 1 - i];
		values[i] = value;
		if (pattern_has_wildcard) mask[i] = value != 0;
		if (text_has_wildcard) squares[i] = std::uint32_t(value) * value;
		pattern_square_sum += std::uint32_t(value) * value;
	}
	ntt_mod_detail::forward(values, plan, vectorized);
	if (pattern_has_wildcard) ntt_mod_detail::forward(mask, plan, vectorized);
	if (text_has_wildcard) ntt_mod_detail::forward(squares, plan, vectorized);
	std::uint32_t scale = (std::uint64_t(1) << 32) % mod * (mod - (mod - 1) / length) % mod;
	std::uint32_t constant = text_has_wildcard ? 0 : pattern_square_sum % mod;
	std::vector<std::uint32_t> work(length), result(length);
	for (std::size_t start = 0; start < matches.size(); start += length - m + 1) {
		std::size_t size = std::min(length, text.size() - start), count = size - m + 1;
		auto first = matches.begin() + start, last = first + count;
		if (std::find(first, last, true) == last) continue;
		for (std::size_t i = 0; i < size; ++i) result[i] = text[start + i];
		std::fill(result.begin() + size, result.end(), 0);
		ntt_mod_detail::forward(result, plan, vectorized);
		ntt_mod_detail::pointwise<mod>(result, values, vectorized);
		for (auto &value : result) {
			std::uint32_t twice = value + value;
			if (twice >= mod) twice -= mod;
			value = twice ? mod - twice : 0;
		}
		for (int channel = 0; channel < 2; ++channel) {
			if (channel == 0 ? !pattern_has_wildcard : !text_has_wildcard) continue;
			for (std::size_t i = 0; i < size; ++i) {
				auto value = text[start + i];
				work[i] = channel == 0 ? std::uint32_t(value) * value : value != 0;
			}
			std::fill(work.begin() + size, work.end(), 0);
			ntt_mod_detail::forward(work, plan, vectorized);
			ntt_mod_detail::pointwise<mod>(work, channel == 0 ? mask : squares, vectorized);
			for (std::size_t i = 0; i < length; ++i) {
				std::uint32_t sum = result[i] + work[i];
				result[i] = sum < mod ? sum : sum - mod;
			}
		}
		ntt_mod_detail::backward(result, plan, vectorized);
		std::reverse(result.begin() + 1, result.end());
		ntt_mod_detail::scale<mod>(result, scale, vectorized);
		std::uint64_t window = 0;
		if (!pattern_has_wildcard)
			for (std::size_t i = 0; i < m; ++i) window += std::uint32_t(text[start + i]) * text[start + i];
		for (std::size_t i = 0; i < count; ++i) {
			if (matches[start + i]) {
				std::uint32_t value = result[m - 1 + i] + constant;
				if (value >= mod) value -= mod;
				if (!pattern_has_wildcard) value = (value + window) % mod;
				if (value != 0) matches[start + i] = false;
			}
			if (!pattern_has_wildcard && i + 1 < count) {
				window -= std::uint32_t(text[start + i]) * text[start + i];
				window += std::uint32_t(text[start + i + m]) * text[start + i + m];
			}
		}
	}
}

inline void filter_ntt(const std::vector<std::uint16_t> &text, const std::vector<std::uint16_t> &pattern,
		std::vector<bool> &matches, std::uint64_t bound, bool text_has_wildcard, bool pattern_has_wildcard) {
	if (pattern.size() > (std::size_t(1) << 22)) throw std::length_error("wildcard pattern is too long");
	if (std::find(matches.begin(), matches.end(), true) == matches.end()) return;
	filter_prime<998244353>(text, pattern, matches, text_has_wildcard, pattern_has_wildcard);
	if (bound >= 998244353 && std::find(matches.begin(), matches.end(), true) != matches.end())
		filter_prime<469762049>(text, pattern, matches, text_has_wildcard, pattern_has_wildcard);
}

}


#line 6 "string/wildcard_pattern_matching.hpp"
#include <array>
#include <climits>
#line 9 "string/wildcard_pattern_matching.hpp"
#include <optional>
#line 11 "string/wildcard_pattern_matching.hpp"
#include <string_view>
#line 13 "string/wildcard_pattern_matching.hpp"

struct WildcardOptions {
	std::optional<unsigned char> text_wildcard = '*';
	std::optional<unsigned char> pattern_wildcard = '*';
};

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, WildcardOptions options) {
	if (text.size() > std::size_t(INT_MAX) || pattern.size() > (std::size_t(1) << 22))
		throw std::length_error("wildcard_pattern_matching: string too long");
	const std::size_t n = text.size(), m = pattern.size();
	if (m > n) return {};
	std::vector<bool> matches(n - m + 1, true);
	if (!m) return matches;
	auto wildcard = [](unsigned char c, std::optional<unsigned char> w) { return w && c == *w; };
	if (n == m) {
		for (std::size_t i = 0; i < m; ++i)
			if (text[i] != pattern[i] && !wildcard(text[i], options.text_wildcard) && !wildcard(pattern[i], options.pattern_wildcard)) {
				matches[0] = false;
				break;
			}
		return matches;
	}
	if (m <= 64) {
		std::array<std::uint64_t, 256> masks{};
		std::uint64_t any = 0, state = 0, last = std::uint64_t(1) << (m - 1);
		for (std::size_t i = 0; i < m; ++i) {
			unsigned char c = pattern[i];
			if (wildcard(c, options.pattern_wildcard)) any |= std::uint64_t(1) << i;
			else masks[c] |= std::uint64_t(1) << i;
		}
		for (auto &mask : masks) mask |= any;
		for (std::size_t i = 0; i < n; ++i) {
			unsigned char c = text[i];
			state = ((state << 1) | 1) & (wildcard(c, options.text_wildcard) ? ~std::uint64_t(0) : masks[c]);
			if (i + 1 >= m) matches[i + 1 - m] = (state & last) != 0;
		}
		return matches;
	}
	std::array<bool, 256> used{};
	std::array<std::size_t, 256> frequency{};
	bool text_wild = false, pattern_wild = false;
	std::size_t literals = 0, text_literals = 0;
	for (unsigned char c : text) {
		if (wildcard(c, options.text_wildcard)) text_wild = true;
		else { used[c] = true; ++frequency[c]; ++text_literals; }
	}
	for (unsigned char c : pattern) {
		if (wildcard(c, options.pattern_wildcard)) pattern_wild = true;
		else { used[c] = true; ++literals; }
	}
	if (!text_wild && !pattern_wild) {
		std::vector<int> prefix(m);
		for (std::size_t i = 1, j = 0; i < m; ++i) {
			while (j && pattern[i] != pattern[j]) j = prefix[j - 1];
			if (pattern[i] == pattern[j]) ++j;
			prefix[i] = static_cast<int>(j);
		}
		std::fill(matches.begin(), matches.end(), false);
		for (std::size_t i = 0, j = 0; i < n; ++i) {
			while (j && text[i] != pattern[j]) j = prefix[j - 1];
			if (text[i] == pattern[j]) ++j;
			if (j == m) { matches[i + 1 - m] = true; j = prefix[j - 1]; }
		}
		return matches;
	}
	std::array<std::uint16_t, 256> codes{};
	std::uint16_t alphabet = 0;
	for (std::size_t c = 0; c < used.size(); ++c)
		if (used[c]) codes[c] = ++alphabet;
	if (alphabet <= 1 || !literals || !text_literals) return matches;
	std::vector<std::uint16_t> a(n), b(m);
	for (std::size_t i = 0; i < n; ++i) {
		unsigned char c = text[i];
		a[i] = wildcard(c, options.text_wildcard) ? 0 : codes[c];
	}
	std::size_t first = m;
	for (std::size_t i = 0; i < m; ++i) {
		unsigned char c = pattern[i];
		if (!wildcard(c, options.pattern_wildcard)) {
			b[i] = codes[c];
			if (first == m || frequency[c] < frequency[static_cast<unsigned char>(pattern[first])]) first = i;
		}
	}
	std::vector<int> candidates;
	candidates.reserve(matches.size());
	for (std::size_t i = 0; i < matches.size(); ++i) candidates.push_back(static_cast<int>(i));
	std::uint64_t budget = 8 * (std::uint64_t(n) + m);
	auto filter = [&](std::size_t position) {
		if (candidates.size() > budget) return false;
		budget -= candidates.size();
		std::size_t kept = 0;
		for (int start : candidates) {
			auto value = a[std::size_t(start) + position];
			if (!value || value == b[position]) candidates[kept++] = start;
			else matches[start] = false;
		}
		candidates.resize(kept);
		return true;
	};
	filter(first);
	for (std::size_t i = 0; i < m && !candidates.empty(); ++i)
		if (b[i] && i != first && !filter(i)) {
			std::uint64_t bound = std::uint64_t(literals) * (alphabet - 1) * (alphabet - 1);
			wildcard_matching_detail::filter_ntt(a, b, matches, bound, text_wild, pattern_wild);
			break;
		}
	return matches;
}

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_pattern_matching(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, WildcardOptions options) {
	auto matches = wildcard_pattern_matching(text, pattern, options);
	std::vector<int> positions;
	for (std::size_t i = 0; i < matches.size(); ++i)
		if (matches[i]) positions.push_back(static_cast<int>(i));
	return positions;
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_find_all(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 15 "components/fast_io.hpp"
#include <string>
#line 17 "components/fast_io.hpp"
#include <type_traits>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/wildcard_pattern_matching.test.cpp"

int main() {
	FastInput in;
	FastOutput out;
	std::string text, pattern;
	in.read(text);
	in.read(pattern);
	for (bool matched : wildcard_pattern_matching(text, pattern)) out.write(matched ? '1' : '0');
	out.write('\n');
}
