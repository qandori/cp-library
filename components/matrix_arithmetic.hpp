#ifndef MATRIX_ARITHMETIC_HPP
#define MATRIX_ARITHMETIC_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

namespace matrix_detail {
template <class T>
inline constexpr bool integer = std::is_integral_v<T> || std::is_same_v<T, __int128_t> || std::is_same_v<T, __uint128_t>;

template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<T>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 31))
		&& std::is_convertible_v<decltype(std::declval<T>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

template <class T>
inline constexpr bool small_modint = SmallModint<T>::value;

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(MATRIX_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

template <class T>
struct Field {
	using V = T;
	static T add(T a, T b) { return a + b; }
	static T mul(T a, T b) { return a * b; }
	static T neg(T a) { return -a; }
	static T inv(T a) { return T(1) / a; }
	static bool zero(T a, long double eps) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) <= eps;
		else { (void)eps; return a == T(0); }
	}
	static bool better(T a, T b) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) > std::abs(b);
		else { (void)a; (void)b; return false; }
	}
	static void scale(T *a, int n, T x) { for (int i = 0; i < n; ++i) a[i] *= x; }
	static void axpy(T *a, const T *b, int n, T x) { for (int i = 0; i < n; ++i) a[i] += x * b[i]; }
};

template <class T>
struct ModularField {
	using V = std::uint32_t;
	static constexpr V mod = T::mod();
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V mul(V a, V b) { return std::uint64_t(a) * b % mod; }
	static V neg(V a) { return a ? mod - a : 0; }
	static V dot(const V *a, const V *b, int n) {
		constexpr int step = mod <= (V(1) << 30) ? 16 : 4;
		std::uint64_t sum = 0;
		for (int i = 0; i < n; i += step) {
			for (int j = i; j < std::min(n, i + step); ++j) sum += std::uint64_t(a[j]) * b[j];
			sum %= mod;
		}
		return sum;
	}
	static V inv(V a) { return (T(1) / T::raw(a)).val(); }
	static bool zero(V a, long double) { return !a; }
	static bool better(V, V) { return false; }
	static V product(V a, V x, V q) {
		V r = a * x - V((std::uint64_t(a) * q) >> 32) * mod;
		return r < mod ? r : r - mod;
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx2"))) static __m256i product8(__m256i a, __m256i x, __m256i q) {
		__m256i p = _mm256_set1_epi32(mod);
		__m256i even = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		__m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		__m256i high = _mm256_or_si256(even, _mm256_slli_epi64(_mm256_srli_epi64(odd, 32), 32));
		__m256i r = _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(high, p));
		return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
	}
	template <bool add_to>
	__attribute__((target("avx2"))) static void apply8(V *a, const V *b, int n, V x, V q) {
		__m256i vx = _mm256_set1_epi32(x), vq = _mm256_set1_epi32(q), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			__m256i r = product8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), vx, vq);
			if constexpr (add_to) {
				r = _mm256_add_epi32(r, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)));
				r = _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	static void apply(V *a, const V *b, int n, V x) {
		V q = (std::uint64_t(x) << 32) / mod;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 16 && avx2()) { apply8<add_to>(a, b, n, x, q); return; }
#endif
		for (int i = 0; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	static void scale(V *a, int n, V x) { apply<false>(a, a, n, x); }
	static void axpy(V *a, const V *b, int n, V x) { apply<true>(a, b, n, x); }
};

template <class T, class F>
auto field(std::vector<T> &a, F &&f) {
	static_assert(!integer<T>);
	if constexpr (small_modint<T>) {
		std::vector<std::uint32_t> work(a.size());
		for (std::size_t i = 0; i < a.size(); ++i) work[i] = a[i].val();
		auto result = f.template operator()<ModularField<T>>(work);
		for (std::size_t i = 0; i < a.size(); ++i) a[i] = T::raw(work[i]);
		return result;
	} else return f.template operator()<Field<T>>(a);
}

template <class Ops>
struct Elimination {
	std::vector<int> pivots;
	typename Ops::V determinant = 1;
};

template <class Ops>
Elimination<Ops> eliminate(std::vector<typename Ops::V> &a, int h, int stride, int columns, bool reduced, long double eps) {
	using V = typename Ops::V;
	Elimination<Ops> result;
	result.pivots.reserve(std::min(h, columns));
	int rank = 0;
	for (int col = 0; col < columns && rank < h; ++col) {
		int p = -1;
		for (int i = rank; i < h; ++i) {
			V x = a[std::size_t(i) * stride + col];
			if (!Ops::zero(x, eps) && (p < 0 || Ops::better(x, a[std::size_t(p) * stride + col]))) p = i;
		}
		if (p < 0) continue;
		V *row = a.data() + std::size_t(rank) * stride;
		if (p != rank) {
			std::swap_ranges(row, row + stride, a.data() + std::size_t(p) * stride);
			result.determinant = Ops::neg(result.determinant);
		}
		result.determinant = Ops::mul(result.determinant, row[col]);
		Ops::scale(row + col + 1, stride - col - 1, Ops::inv(row[col]));
		row[col] = V(1);
		for (int i = rank + 1; i < h; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps)) Ops::axpy(dst + col + 1, row + col + 1, stride - col - 1, Ops::neg(x));
			dst[col] = V(0);
		}
		result.pivots.push_back(col);
		++rank;
	}
	if (reduced) for (int k = rank - 1; k >= 0; --k) {
		int col = result.pivots[k];
		const V *row = a.data() + std::size_t(k) * stride;
		for (int i = 0; i < k; ++i) {
			V *dst = a.data() + std::size_t(i) * stride;
			V x = dst[col];
			if (!Ops::zero(x, eps)) Ops::axpy(dst + col + 1, row + col + 1, stride - col - 1, Ops::neg(x));
			dst[col] = V(0);
		}
	}
	return result;
}

template <class T>
[[gnu::noinline]] T integer_determinant(const std::vector<T> &a, int n) {
	static_assert(std::numeric_limits<T>::is_signed);
	using W = __int128_t;
	std::vector<W> work(a.begin(), a.end());
	W previous = 1, sign = 1;
	if (!n) return T(1);
	for (int k = 0; k + 1 < n; ++k) {
		int pivot = k;
		while (pivot < n && work[std::size_t(pivot) * n + k] == 0) ++pivot;
		if (pivot == n) return T(0);
		if (pivot != k) { for (int j = 0; j < n; ++j) std::swap(work[std::size_t(k) * n + j], work[std::size_t(pivot) * n + j]); sign = -sign; }
		W p = work[std::size_t(k) * n + k];
		for (int i = k + 1; i < n; ++i) {
			for (int j = k + 1; j < n; ++j) {
				W value = work[std::size_t(i) * n + j] * p - work[std::size_t(i) * n + k] * work[std::size_t(k) * n + j];
				assert(value % previous == 0);
				work[std::size_t(i) * n + j] = value / previous;
			}
			work[std::size_t(i) * n + k] = 0;
		}
		previous = p;
	}
	W result = sign * work.back();
	assert(result >= std::numeric_limits<T>::lowest() && result <= std::numeric_limits<T>::max());
	return T(result);
}

template <std::uint32_t mod>
void product_scalar(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	for (int i = 0; i < n; ++i) for (int j = 0; j < m; j += 32) {
		int width = std::min(32, m - j);
		for (int k = 0; k < inner; k += step) {
			std::uint64_t sums[32]{};
			for (int t = k; t < std::min(inner, k + step); ++t) {
				std::uint64_t x = a[std::size_t(i) * inner + t];
				const auto *row = b + std::size_t(t) * m + j;
				for (int s = 0; s < width; ++s) sums[s] += x * row[s];
			}
			for (int s = 0; s < width; ++s) c[std::size_t(i) * m + j + s] = (sums[s] + c[std::size_t(i) * m + j + s]) % mod;
		}
	}
}

#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
template <std::uint32_t mod>
__attribute__((target("avx2"))) void product_avx2(const std::uint32_t *a, const std::uint32_t *b, std::uint32_t *c, int n, int inner, int m) {
	constexpr int step = mod <= (std::uint32_t(1) << 30) ? 16 : 4;
	int i = 0;
	for (; i + 2 <= n; i += 2) {
		int j = 0;
		for (; j + 8 <= m; j += 8) for (int k = 0; k < inner; k += step) {
			__m256i x0 = _mm256_setzero_si256(), x1 = x0, y0 = x0, y1 = x0;
			for (int t = k; t < std::min(inner, k + step); ++t) {
				__m256i row = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + std::size_t(t) * m + j));
				__m256i odd = _mm256_srli_epi64(row, 32);
				__m256i u = _mm256_set1_epi32(a[std::size_t(i) * inner + t]);
				__m256i v = _mm256_set1_epi32(a[std::size_t(i + 1) * inner + t]);
				x0 = _mm256_add_epi64(x0, _mm256_mul_epu32(u, row));
				x1 = _mm256_add_epi64(x1, _mm256_mul_epu32(u, odd));
				y0 = _mm256_add_epi64(y0, _mm256_mul_epu32(v, row));
				y1 = _mm256_add_epi64(y1, _mm256_mul_epu32(v, odd));
			}
			std::uint64_t even[4], odd[4], even2[4], odd2[4];
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even), x0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd), x1);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(even2), y0);
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(odd2), y1);
			for (int s = 0; s < 4; ++s) {
				auto *r = c + std::size_t(i) * m + j + s * 2;
				auto *r2 = r + m;
				r[0] = (even[s] + r[0]) % mod; r[1] = (odd[s] + r[1]) % mod;
				r2[0] = (even2[s] + r2[0]) % mod; r2[1] = (odd2[s] + r2[1]) % mod;
			}
		}
		if (j < m) for (int row = i; row < i + 2; ++row) for (int col = j; col < m; ++col) {
			std::uint64_t sum = 0;
			for (int k = 0; k < inner; k += step) {
				for (int t = k; t < std::min(inner, k + step); ++t) sum += std::uint64_t(a[std::size_t(row) * inner + t]) * b[std::size_t(t) * m + col];
				sum %= mod;
			}
			c[std::size_t(row) * m + col] = sum;
		}
	}
	if (i < n) product_scalar<mod>(a + std::size_t(i) * inner, b, c + std::size_t(i) * m, 1, inner, m);
}
#endif

template <class T>
std::vector<T> multiply(const std::vector<T> &a, const std::vector<T> &b, int n, int inner, int m) {
	std::vector<T> c(std::size_t(n) * m, T(0));
	if (!n || !inner || !m) return c;
	auto basic = [&] {
		for (int i = 0; i < n; ++i) for (int k = 0; k < inner; ++k) {
			const T &x = a[std::size_t(i) * inner + k];
			for (int j = 0; j < m; ++j) c[std::size_t(i) * m + j] += x * b[std::size_t(k) * m + j];
		}
	};
	if constexpr (small_modint<T>) {
		if (std::uint64_t(n) * m <= 128u / unsigned(inner)) { basic(); return c; }
		std::vector<std::uint32_t> x(a.size()), y(b.size()), z(c.size());
		for (std::size_t i = 0; i < a.size(); ++i) x[i] = a[i].val();
		for (std::size_t i = 0; i < b.size(); ++i) y[i] = b[i].val();
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (inner >= 16 && m >= 8 && avx2()) product_avx2<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		else
#endif
		product_scalar<T::mod()>(x.data(), y.data(), z.data(), n, inner, m);
		for (std::size_t i = 0; i < c.size(); ++i) c[i] = T::raw(z[i]);
	} else basic();
	return c;
}
}

#endif
