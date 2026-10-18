#ifndef CP_COMPONENTS_FPS_OPERATIONS_HPP
#define CP_COMPONENTS_FPS_OPERATIONS_HPP

#include "ntt.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <type_traits>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T> struct FormalPowerSeries;

namespace fps_detail {

template<class T, class Work = Mint>
struct Operations {
	using F = FormalPowerSeries<T>;
	using W = FormalPowerSeries<Work>;
	using base = std::vector<T>;
	using Word = NTT::U;

	static W import_work(const base &a) {
		W r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = fps_detail::Mint::raw(a[i].val());
		return r;
	}
	static F export_work(const W &a) {
		F r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = T::raw(a[i].val());
		return r;
	}
	static fps_detail::NTT &transform(int n) { return fps_detail::ntt_plan(n); }
	static Word *words(base &a) {
		static_assert(sizeof(T) == sizeof(Word) && std::is_trivially_copyable_v<T>);
		return reinterpret_cast<Word *>(a.data());
	}
	static const Word *words(const base &a) { return reinterpret_cast<const Word *>(a.data()); }
	template <bool inverse>
	static void transform_inplace(base &a) {
		int n = a.size();
		assert(n && !(n & (n - 1)) && n <= (1 << 23));
		auto &t = transform(std::max(1, n / 2));
		t.template fft<inverse>(words(a), n);
		if constexpr (!std::is_same_v<T, fps_detail::Mint>) t.normalize(words(a), n);
	}
	static void ntt(base &a) { transform_inplace<false>(a); }
	static void ntt_half(Word *a, int n) {
		std::copy_n(a, n / 2, a + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(a, n, n / 2);
	}
	static void ntt_half(base &a) {
		int n = a.size();
		std::copy_n(a.begin(), n / 2, a.begin() + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(words(a), n, n / 2);
	}
	static void ntt_inverse(base &a) { transform_inplace<true>(a); }
	static void scale_words(base &a, T factor) {
		if (a.empty()) return;
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			auto &t = transform(1);
			t.scale(words(a), a.size(), factor.val());
		} else
			for (T &x : a) x *= factor;
	}
	static void geometric_scale(base &a, T first, T step) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::geometric(words(a), a.size(), first.v, step.v);
		else
			for (T &x : a) {
				x *= first;
				first *= step;
			}
	}
	static void batch_inverse(base &dst, const base &src) {
		int n = src.size();
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 128 && fps_detail::NTT::vectorized()) {
				fps_detail::NTT::batch_inverse8(words(dst), words(src), n);
				return;
			}
		}
#endif
		base prefix(n);
		T p = 1;
		for (int i = 0; i < n; ++i) prefix[i] = p, p *= src[i];
		assert(p != T(0));
		p = p.inv();
		for (int i = n - 1; i >= 0; --i) {
			dst[i] = prefix[i] * p;
			p *= src[i];
		}
	}
	static void pointwise(base &r, const base &a, const base &b, int n) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::product(words(r), words(a), words(b), n);
		else
			for (int i = 0; i < n; ++i) r[i] = a[i] * b[i];
	}
#if defined(__x86_64__) || defined(__i386__)
	template <int count, bool first>
	__attribute__((target("avx2"))) static void accumulate_products8(Word *dst, const Word *const *a, const Word *const *b, int n) {
		using I8 = fps_detail::NTT::I8;
		I8 p = _mm256_set1_epi32(T::mod()), pp = _mm256_set1_epi32(2 * T::mod()), ni = _mm256_set1_epi32(998244351);
		for (int i = 0; i < n; i += 8) {
			I8 even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
			for (int k = 0; k < count; ++k) {
				I8 x = fps_detail::NTT::load8(a[k] + i), y = fps_detail::NTT::load8(b[k] + i);
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
			I8 value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (count == 7) value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			if constexpr (!first) value = fps_detail::NTT::add8(fps_detail::NTT::load8(dst + i), value);
			fps_detail::NTT::store8(dst + i, value);
		}
	}
#endif
	template <int count, bool first>
	static void accumulate_products(Word *dst, const Word *const *a, const Word *const *b, int n) {
		static_assert(count >= 1 && count <= 7);
#if defined(__x86_64__) || defined(__i386__)
		if (n >= 8 && fps_detail::NTT::vectorized()) {
			accumulate_products8<count, first>(dst, a, b, n);
			return;
		}
#endif
		for (int i = 0; i < n; ++i) {
			Word value = 0;
			for (int k = 0; k < count; ++k) value = fps_detail::NTT::add(value, fps_detail::NTT::mul(a[k][i], b[k][i]));
			if constexpr (!first) value = fps_detail::NTT::add(value, dst[i]);
			dst[i] = value;
		}
	}
	static void join_windows(Word *blocks, int length, int count) {
		for (int i = count - 1; i > 0; --i) {
			Word *a = blocks + i * length;
			const Word *b = a - length;
			for (int j = 0; j < length / 2; ++j) a[j] = fps_detail::NTT::add(a[j], b[j]);
			for (int j = length / 2; j < length; ++j) a[j] = fps_detail::NTT::sub(a[j], b[j]);
		}
	}
	static void inverse_extend(F &h, const base &a, const base &cache) {
		const int m = static_cast<int>(cache.size());
		const T im = T::raw(T::mod() - (T::mod() - 1) / m);
		base b(m);
		pointwise(b, a, cache, m);
		ntt_inverse(b);
		std::fill(b.begin(), b.begin() + m / 2, T(0));
		ntt(b);
		pointwise(b, b, cache, m);
		ntt_inverse(b);
		h.resize(m);
		for (int i = m / 2; i < m; ++i) h[i] = -b[i] * im * im;
	}
	static T dot(const T *a, const T *b, int n) {

#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (n >= 64 && fps_detail::NTT::vectorized())
				return T::word(fps_detail::NTT::reverse_dot8(reinterpret_cast<const Word *>(a), reinterpret_cast<const Word *>(b), n));
		}
#endif
		unsigned int result = 0;
		for (int i = 0; i < n; i += 16) {
			unsigned long long sum = 0;
			for (int j = i; j < std::min(n, i + 16); ++j)
				sum += (unsigned long long)a[j].val() * b[-j].val();
			result += sum % T::mod();
			if (result >= T::mod()) result -= T::mod();
		}
		return T::raw(result);
	}
	static int prepare_sparse(std::vector<std::pair<int, T>> &terms) {
		int stride = 0;
		for (auto &[j, x] : terms) {
			stride = std::gcd(stride, j);
			if constexpr (std::is_same_v<T, fps_detail::Mint>) if (x.v >= T::mod()) x.v -= T::mod();
		}
		return stride;
	}
	static T sparse_dot(const std::vector<std::pair<int, T>> &terms, const F &g, int i) {
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			static_assert(7ULL * (T::mod() - 1) * (2ULL * T::mod() - 1) + 0xffffffffULL * T::mod() < ~0ULL);
			unsigned long long sum = 0;
			Word result = 0;
			int count = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				sum += (unsigned long long)x.v * g[i - j].v;
				if (++count == 7) {
					result = fps_detail::NTT::add(result, fps_detail::NTT::reduce(sum) % T::mod());
					sum = 0;
					count = 0;
				}
			}
			return T::word(fps_detail::NTT::add(result, fps_detail::NTT::reduce(sum) % T::mod()));
		} else {
			T result = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				result += x * g[i - j];
			}
			return result;
		}
	}

	static void intt(base &a) {
		ntt_inverse(a);
		const T inv = T::raw(T::mod() - (T::mod() - 1) / static_cast<int>(a.size()));
		scale_words(a, inv);
	}
	static T mod_sqrt(T a) {
		if (a == T(0) || a.pow((T::mod() - 1) / 2) != T(1)) return 0;
		T r = a.pow(60), t = a.pow(119), c = T(3).pow(119);
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T u = t; u != T(1); u *= u) ++i;
			T b = c.pow(1 << (m - i - 1));
			r *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return r;
	}
	static const base &inverse_ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m)).inv();
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const base &ntt_roots(int n) {
		static base roots{1};
		while (static_cast<int>(roots.size()) < n) {
			const int m = static_cast<int>(roots.size());
			const T step = T(3).pow((T::mod() - 1) / (2 * m));
			roots.resize(2 * m);
			for (int i = 0; i < m; ++i) roots[m + i] = roots[i] * step;
		}
		return roots;
	}
	static const std::vector<T> &inverses(int n) {
		assert(unsigned(n) < unsigned(T::mod()));
		static std::vector<T> inv{0, 1};
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			int first = inv.size(), count = (n + 1 - first) / 8 * 8;
			if (count >= 256 && fps_detail::NTT::vectorized()) {
				inv.reserve(n + 1);
				inv.resize(first + count);
				fps_detail::NTT::inverse_sequence8(reinterpret_cast<Word *>(inv.data()) + first, first, count);
			}
		}
#endif
		while (static_cast<int>(inv.size()) <= n) {
			const int i = static_cast<int>(inv.size());
			inv.push_back(-inv[T::mod() % i] * (T::mod() / i));
		}
		return inv;
	}
	static void trim(F &a) {
		while (!a.empty() && a.back() == T(0)) a.pop_back();
	}
};

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
