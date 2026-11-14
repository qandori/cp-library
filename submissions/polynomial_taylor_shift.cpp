#line 1 "test/fps_taylor_shift.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_taylor_shift"
#line 1 "fps/taylor_shift.hpp"



#line 1 "fps/fps.hpp"



#line 1 "components/fps_operations.hpp"



#line 1 "components/ntt.hpp"



#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#line 1 "components/mint_arithmetic.hpp"



#line 1 "components/mint_simd.hpp"



#line 1 "math/mint.hpp"


#line 1 "components/compiler_features.hpp"



#if defined(__x86_64__) || defined(__i386__)
#define CP_X86 1
#else
#define CP_X86 0
#endif
#if CP_X86 && defined(__GNUC__) && !defined(__clang__)
#define CP_GCC_X86 1
#else
#define CP_GCC_X86 0
#endif
#if CP_X86 && (defined(__GNUC__) || defined(__clang__))
#define CP_GNU_X86 1
#else
#define CP_GNU_X86 0
#endif
#if CP_GCC_X86 && defined(__linux__) && defined(__x86_64__)
#define CP_GCC_LINUX64 1
#else
#define CP_GCC_LINUX64 0
#endif


#line 5 "math/mint.hpp"
#include <istream>
#include <ostream>
#line 8 "math/mint.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct [[gnu::warn_unused]] Mint {
	using mint = Mint;
	unsigned v = 0;
	[[nodiscard]] static constexpr unsigned mod() { return 998244353; }
	[[nodiscard, gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[nodiscard, gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[nodiscard, gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[nodiscard, gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[nodiscard, gnu::always_inline]] Mint operator+() const { return *this; }
	[[nodiscard, gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[nodiscard, gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[nodiscard, gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[nodiscard, gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	[[nodiscard]] Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[nodiscard, gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[nodiscard, gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	[[nodiscard]] friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	[[nodiscard]] Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[nodiscard, gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 5 "components/mint_simd.hpp"
#if CP_X86
#include <immintrin.h>
#endif

namespace mint_detail {
struct SimdOps {
	typedef unsigned int U __attribute__((may_alias));
	static constexpr U mod = 998244353, twice = 2 * mod;
#if CP_X86
	static bool vectorized() {
#if defined(__AVX2__)
		return true;
#else
		static const bool available = __builtin_cpu_supports("avx2");
		return available;
#endif
	}
	using I8 = __m256i;
	[[gnu::target("avx2")]] static I8 load8(const U *a) {
		return _mm256_loadu_si256(reinterpret_cast<const I8 *>(a));
	}
	[[gnu::target("avx2")]] static void store8(U *a, I8 x) {
		_mm256_storeu_si256(reinterpret_cast<I8 *>(a), x);
	}
	__attribute__((target("avx2"), always_inline)) static inline I8 reduce8(I8 x, I8 y) {
		I8 r = _mm256_set1_epi32(998244351), p = _mm256_set1_epi32(mod);
		x = _mm256_add_epi64(x, _mm256_mul_epu32(_mm256_mul_epu32(x, r), p));
		y = _mm256_add_epi64(y, _mm256_mul_epu32(_mm256_mul_epu32(y, r), p));
		return _mm256_blend_epi32(_mm256_srli_epi64(x, 32), y, 0xaa);
	}
	[[gnu::target("avx2")]] static I8 mul8(I8 a, I8 b) {
		return reduce8(_mm256_mul_epu32(a, b), _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
	}
	[[gnu::target("avx2")]] static I8 add8(I8 a, I8 b) {
		I8 x = _mm256_add_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(twice)));
	}
#endif
};
}


#line 5 "components/mint_arithmetic.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
struct Ops : SimdOps {
	using T = mint;
	static constexpr U one = 301989884;
	static U reduce(unsigned long long x) { return T::reduce(x); }
	static U mul(U a, U b) { return reduce((unsigned long long)a * b); }
	static U add(U a, U b) {
		U c = a + b;
		return c < twice ? c : c - twice;
	}
	static U sub(U a, U b) {
		U c = a - b;
		return c < twice ? c : c + twice;
	}
	static U in(U a) { return mul(a, 932051910); }
	static U out(U a) {
		a = reduce(a);
		return a < mod ? a : a - mod;
	}
	static U quotient(U r) { return ((unsigned long long)r << 32) / mod; }
	static U fixed_mul(U a, U r, U q) {
		return a * r - U(((unsigned long long)a * q) >> 32) * mod;
	}

#if CP_X86
	__attribute__((target("avx2"))) static I8 fixed_mul8(I8 a, I8 r, I8 q) {
		I8 x = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		I8 y = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		I8 t = _mm256_blend_epi32(x, y, 0xaa);
		return _mm256_sub_epi32(_mm256_mullo_epi32(a, r),
								_mm256_mullo_epi32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 fixed_even8(I8 a, I8 r, I8 q) {
		I8 t = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		return _mm256_sub_epi64(_mm256_mul_epu32(a, r),
								_mm256_mul_epu32(t, _mm256_set1_epi32(mod)));
	}
	__attribute__((target("avx2"))) static I8 sub8(I8 a, I8 b) {
		I8 x = _mm256_sub_epi32(a, b);
		return _mm256_min_epu32(x, _mm256_add_epi32(x, _mm256_set1_epi32(twice)));
	}

#endif
};
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 15 "components/ntt.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
struct NTT : mint_detail::Ops {
	using V = std::vector<unsigned int>;
	V roots, inverse_roots;
	V root_quotients, inverse_root_quotients;
	mutable V cube_roots, inverse_cube_roots, cube_quotients, inverse_cube_quotients;
	void prepare_cubes(int n) const {
		int begin = cube_roots.size();
		if (begin >= n) return;
		cube_roots.resize(n);
		inverse_cube_roots.resize(n);
		cube_quotients.resize(n);
		inverse_cube_quotients.resize(n);
		for (int k = begin; k < n; ++k) {
			U x = (unsigned long long)roots[k] * roots[2 * k] % mod;
			U y = (unsigned long long)inverse_roots[k] * inverse_roots[2 * k] % mod;
			cube_roots[k] = x;
			inverse_cube_roots[k] = y;
			cube_quotients[k] = quotient(x);
			inverse_cube_quotients[k] = quotient(y);
		}
	}
	NTT(int size, [[maybe_unused]] bool composition = true)
		: roots(size, one),
		  inverse_roots(size, one),
		  root_quotients(size),
		  inverse_root_quotients(size) {
		for (int h = 1; h < size; h <<= 1) {
			U r = in(T(3).pow((mod - 1) / (4 * h)).val());
			U ir = in(T(3).pow(mod - 1 - (mod - 1) / (4 * h)).val());
			for (int i = 0; i < h; ++i) {
				roots[h + i] = mul(roots[i], r);
				inverse_roots[h + i] = mul(inverse_roots[i], ir);
			}
		}
		for (int i = 0; i < size; ++i) {
			root_quotients[i] = (roots[i] < mod ? roots[i] : roots[i] - mod) * 998244351u;
			inverse_root_quotients[i] = (inverse_roots[i] < mod ? inverse_roots[i] : inverse_roots[i] - mod) * 998244351u;
			roots[i] = out(roots[i]);
			inverse_roots[i] = out(inverse_roots[i]);
		}
	}

#if CP_X86

	template <int stride>
	__attribute__((target("avx2"))) static I8 rates8(const V &values, int k) {
		if constexpr (stride == 1)
			return _mm256_cvtepu32_epi64(
				_mm_loadu_si128(reinterpret_cast<const __m128i *>(values.data() + 4 * k)));
		if constexpr (stride == 2)
			return _mm256_permute4x64_epi64(
				_mm256_cvtepu32_epi64(
					_mm_loadl_epi64(reinterpret_cast<const __m128i *>(values.data() + 2 * k))),
				0x50);
		if constexpr (stride == 4) return _mm256_set1_epi32(values[k]);
	}
	template <int stride, bool inverse>
	__attribute__((target("avx2"))) I8 small8(I8 z, int k) const {
		if constexpr (stride == 4) {
			if (k == 0) {
				I8 x = _mm256_permute2x128_si256(z, z, 0x00),
				   y = _mm256_permute2x128_si256(z, z, 0x11);
				return _mm256_blend_epi32(add8(x, y), sub8(x, y), 0xf0);
			}
		}
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		I8 r = rates8<stride>(w, k), q = rates8<stride>(quot, k);
		if constexpr (inverse) {
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
			I8 y = _mm256_srli_epi64(z, 32), x = add8(z, y);
			I8 d = fixed_even8(sub8(z, y), r, q);
			z = _mm256_blend_epi32(x, _mm256_slli_epi64(d, 32), 0xaa);
		} else {
			if constexpr (stride == 1) z = _mm256_shuffle_epi32(z, 0xb1);
			if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0x72);
			if constexpr (stride == 4)
				z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(4, 0, 5, 1, 6, 2, 7, 3));
			I8 y = fixed_even8(z, r, q);
			I8 x = _mm256_blend_epi32(_mm256_srli_epi64(z, 32), z, 0xaa);
			y = _mm256_blend_epi32(
				y, _mm256_sub_epi32(_mm256_set1_epi32(twice), _mm256_slli_epi64(y, 32)), 0xaa);
			z = add8(x, y);
		}
		if constexpr (stride == 2) z = _mm256_shuffle_epi32(z, 0xd8);
		if constexpr (stride == 4)
			z = _mm256_permutevar8x32_epi32(z, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
		return z;
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void fft8(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
		if constexpr (!inverse) {
			int h = limit / 2;
			for (; h >= 16; h >>= 2) radix4_8<false>(a, h / 2, length);
			if (h == 8) {
				if (limit >= 32)
					for (int i = 0; i < length; i += 8) {
						I8 x = load8(a + i), p = _mm256_set1_epi32(twice);
						store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
					}
				stage8<false>(a, h, length);
			}
		}
		for (int j = 0; j < length; j += 8) {
			I8 z = load8(a + j);
			if constexpr (!inverse)
				if ((limit >= 32) && (__builtin_ctz((unsigned int)limit) & 1)) {
					I8 p = _mm256_set1_epi32(twice);
					z = _mm256_min_epu32(z, _mm256_sub_epi32(z, p));
				}
			if constexpr (inverse)
				z = small8<4, true>(small8<2, true>(small8<1, true>(z, j / 8), j / 8), j / 8);
			else
				z = small8<1, false>(small8<2, false>(small8<4, false>(z, j / 8), j / 8),
									 j / 8);
			store8(a + j, z);
		}
		if constexpr (inverse) {
			int h = 8;
			for (; 4 * h <= limit; h <<= 2) radix4_8<true>(a, h, length);
			if (h < limit) stage8<true>(a, h, length);
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void radix4_8(U *a, int stride, int length) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &q = inverse ? inverse_root_quotients : root_quotients;
		prepare_cubes(length / (4 * stride));
		const V &w3 = inverse ? inverse_cube_roots : cube_roots;
		const V &q3s = inverse ? inverse_cube_quotients : cube_quotients;
		I8 p = _mm256_set1_epi32(twice);
		I8 im = _mm256_set1_epi32(w[1]), iq = _mm256_set1_epi32(q[1]);
		for (int j = 0, k = 0; j < length; j += 4 * stride, ++k) {
			I8 r = _mm256_set1_epi32(w[k]), rq = _mm256_set1_epi32(q[k]);
			I8 r0 = _mm256_set1_epi32(w[2 * k]), q0 = _mm256_set1_epi32(q[2 * k]);
			I8 r3 = _mm256_set1_epi32(w3[k]), q3 = _mm256_set1_epi32(q3s[k]);
			for (int i = 0; i < stride; i += 8) {
				I8 x0 = load8(a + j + i), x1 = load8(a + j + stride + i);
				I8 x2 = load8(a + j + 2 * stride + i), x3 = load8(a + j + 3 * stride + i);
				if constexpr (inverse) {
					I8 u = add8(x0, x1), v = sub8(x0, x1);
					I8 s = add8(x2, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x2, p), x3), im, iq);
					x0 = add8(u, s);
					x1 = _mm256_add_epi32(v, t);
					x2 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					} else {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
				} else {
					x0 = _mm256_min_epu32(x0, _mm256_sub_epi32(x0, p));
					if (!k) {
						x1 = _mm256_min_epu32(x1, _mm256_sub_epi32(x1, p));
						x2 = _mm256_min_epu32(x2, _mm256_sub_epi32(x2, p));
						x3 = _mm256_min_epu32(x3, _mm256_sub_epi32(x3, p));
					}
					if (k) {
						x1 = fixed_mul8(x1, r0, q0);
						x2 = fixed_mul8(x2, r, rq);
						x3 = fixed_mul8(x3, r3, q3);
					}
					I8 u = add8(x0, x2), v = sub8(x0, x2), s = add8(x1, x3);
					I8 t = fixed_mul8(_mm256_sub_epi32(_mm256_add_epi32(x1, p), x3), im, iq);
					x0 = _mm256_add_epi32(u, s);
					x1 = _mm256_sub_epi32(_mm256_add_epi32(u, p), s);
					x2 = _mm256_add_epi32(v, t);
					x3 = _mm256_sub_epi32(_mm256_add_epi32(v, p), t);
				}
				store8(a + j + i, x0);
				store8(a + j + stride + i, x1);
				store8(a + j + 2 * stride + i, x2);
				store8(a + j + 3 * stride + i, x3);
			}
		}
	}
	template <bool inverse>
	__attribute__((target("avx2"))) void stage8(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
		if (stride < 8) {
			for (int j = 0; j < length; j += 8) {
				I8 z = load8(a + j);
				if (stride == 1)
					z = small8<1, inverse>(z, j / 8);
				else if (stride == 2)
					z = small8<2, inverse>(z, j / 8);
				else
					z = small8<4, inverse>(z, j / 8);
				store8(a + j, z);
			}
		} else {
			for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
				I8 r = _mm256_set1_epi32(w[k]), q = _mm256_set1_epi32(quot[k]);
				for (int i = 0; i < stride; i += 8) {
					I8 x = load8(a + j + i), y = load8(a + j + stride + i);
					if constexpr (!inverse)
						if (k) y = fixed_mul8(y, r, q);
					I8 z = sub8(x, y);
					x = add8(x, y);
					if constexpr (inverse)
						if (k) z = fixed_mul8(z, r, q);
					store8(a + j + i, x);
					store8(a + j + stride + i, z);
				}
			}
		}
	}

	__attribute__((target("avx2"))) static void load_pairs(const U *a, I8 &x, I8 &y) {
		I8 index = _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7);
		I8 u = _mm256_permutevar8x32_epi32(load8(a), index),
		   v = _mm256_permutevar8x32_epi32(load8(a + 8), index);
		x = _mm256_permute2x128_si256(u, v, 0x20);
		y = _mm256_permute2x128_si256(u, v, 0x31);
	}
	__attribute__((target("avx2"))) static void scale_copy8(U *dst, const U *src, int length, U r) {
		U quotient_r = quotient(r);
		I8 v = _mm256_set1_epi32(r), q = _mm256_set1_epi32(quotient_r);
		int i = 0;
		for (; i + 8 <= length; i += 8) store8(dst + i, fixed_mul8(load8(src + i), v, q));
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, quotient_r);
	}
	__attribute__((target("avx2"))) static void scale8(U *a, int length, U r) {
		scale_copy8(a, a, length, r);
	}
#endif

	template <bool inverse>
	void stage(U *a, int stride, int length, int first = 0) const {
		const V &w = inverse ? inverse_roots : roots;
		const V &quot = inverse ? inverse_root_quotients : root_quotients;
#if CP_X86
		if (length >= 8 && vectorized()) {
			stage8<inverse>(a, stride, length, first);
			return;
		}
#endif
		for (int j = 0, k = first; j < length; j += 2 * stride, ++k) {
			U r = w[k], q = quot[k];
			int i = 0;
			for (; i < stride; ++i) {
				U x = a[j + i], y = a[j + stride + i];
				if constexpr (!inverse)
					if (k) y = fixed_mul(y, r, q);
				a[j + i] = add(x, y);
				a[j + stride + i] = inverse && k ? fixed_mul(sub(x, y), r, q) : sub(x, y);
			}
		}
	}

	template <bool inverse>
	void fft(U *a, int length, int limit = 0) const {
		if (!limit) limit = length;
#if CP_X86
		if (limit >= 8 && vectorized()) {
			fft8<inverse>(a, length, limit);
			return;
		}
#endif
		if constexpr (inverse) {
			for (int h = 1; h < limit; h <<= 1) stage<true>(a, h, length);
		} else {
			for (int h = limit / 2; h; h >>= 1) stage<false>(a, h, length);
		}
	}
#if CP_X86
	__attribute__((target("avx2"))) static void product8(U *dst, const U *a, const U *b, int n) {
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = mul8(load8(a + i), load8(b + i));
			store8(dst + i, x);
		}
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
	__attribute__((target("avx2"))) static void weighted8(U *dst, const U *src, int n, int first) {
		alignas(32) U factors[8];
		for (int j = 0; j < 8; ++j) factors[j] = in(first + j);
		I8 r = load8(factors), step = _mm256_set1_epi32(in(8));
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(dst + i, mul8(load8(src + i), r));
			r = add8(r, step);
		}
		for (; i < n; ++i) dst[i] = mul(src[i], in(first + i));
	}
	__attribute__((target("avx2"), always_inline)) static I8 inverse8(I8 a) {
		I8 a2 = mul8(a, a), a3 = mul8(a2, a);
		I8 a7 = mul8(mul8(a3, a3), a), a31 = a7;
		for (int repeat_index = 0; repeat_index < 2; ++repeat_index) a31 = mul8(a31, a31);
		a31 = mul8(a31, a3);
		I8 r = a7;
		for (int repeat_index = 0; repeat_index < 3; ++repeat_index) r = mul8(r, r);
		r = mul8(r, a3);
		r = mul8(r, r);
		for (int j = 0; j < 4; ++j) {
			for (int repeat_index = 0; repeat_index < 5; ++repeat_index) r = mul8(r, r);
			r = mul8(r, a31);
		}
		for (int repeat_index = 0; repeat_index < 3; ++repeat_index) r = mul8(r, r);
		return mul8(r, a7);
	}
	__attribute__((target("avx2"))) static void batch_inverse8(U *dst, const U *src, int n) {
		if (n < 16) {
			for (int i = 0; i < n; ++i) dst[i] = T::word(src[i]).inv().v;
			return;
		}
		int count = n / 16 * 16;
		I8 p = load8(src), q = load8(src + 8);
		store8(dst, p);
		store8(dst + 8, q);
		for (int i = 16; i < count; i += 16) {
			p = mul8(p, load8(src + i));
			q = mul8(q, load8(src + i + 8));
			store8(dst + i, p);
			store8(dst + i + 8, q);
		}
		alignas(32) U lanes[16];
		store8(lanes, p);
		store8(lanes + 8, q);
		for ([[maybe_unused]] auto x : lanes) assert(out(x) != 0);
		I8 inverse = inverse8(p), other = inverse8(q);
		for (int i = count - 16; i >= 16; i -= 16) {
			store8(dst + i, mul8(inverse, load8(dst + i - 16)));
			store8(dst + i + 8, mul8(other, load8(dst + i - 8)));
			inverse = mul8(inverse, load8(src + i));
			other = mul8(other, load8(src + i + 8));
		}
		store8(dst, inverse);
		store8(dst + 8, other);
		for (int i = count; i < n; ++i) dst[i] = T::word(src[i]).inv().v;
	}
	__attribute__((target("avx2"))) static void inverse_sequence8(U *dst, int first, int n) {
		if (n >= 4096) {
			const int count = n / 32 * 32;
			const I8 step = _mm256_set1_epi32(in(32));
			I8 factor[4], prefix[4], inverse[4];
			for (int c = 0; c < 4; ++c) {
				alignas(32) U initial[8];
				for (int j = 0; j < 8; ++j) initial[j] = in(first + 8 * c + j);
				factor[c] = prefix[c] = load8(initial);
				store8(dst + 8 * c, prefix[c]);
			}
			for (int i = 32; i < count; i += 32)
				for (int c = 0; c < 4; ++c) {
					factor[c] = add8(factor[c], step);
					prefix[c] = mul8(prefix[c], factor[c]);
					store8(dst + i + 8 * c, prefix[c]);
				}
			for (int c = 0; c < 4; ++c) inverse[c] = inverse8(prefix[c]);
			for (int i = count - 32; i >= 32; i -= 32)
				for (int c = 0; c < 4; ++c) {
					store8(dst + i + 8 * c, mul8(inverse[c], load8(dst + i - 32 + 8 * c)));
					inverse[c] = mul8(inverse[c], factor[c]);
					factor[c] = sub8(factor[c], step);
				}
			for (int c = 0; c < 4; ++c) store8(dst + 8 * c, inverse[c]);
			if (count != n) inverse_sequence8(dst + count, first + count, n - count);
			return;
		}

		alignas(32) U initial[8];
		for (int j = 0; j < 8; ++j) initial[j] = in(first + j);
		I8 factor = load8(initial), step = _mm256_set1_epi32(in(8));
		I8 p = factor;
		store8(dst, p);
		for (int i = 8; i < n; i += 8) {
			factor = add8(factor, step);
			p = mul8(p, factor);
			store8(dst + i, p);
		}
		I8 inverse = inverse8(p);
		for (int i = n - 8; i >= 8; i -= 8) {
			store8(dst + i, mul8(inverse, load8(dst + i - 8)));
			inverse = mul8(inverse, factor);
			factor = sub8(factor, step);
		}
		store8(dst, inverse);
	}
	__attribute__((target("avx2"))) static void normalize8(U *a, int n) {
		I8 p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			I8 x = load8(a + i);
			store8(a + i, _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
		}
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
#endif
	static void weighted(U *dst, const U *src, int n, int first) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			weighted8(dst, src, n, first);
			return;
		}
#endif
		for (int i = 0; i < n; ++i) dst[i] = mul(src[i], in(first + i));
	}
	static void normalize(U *a, int n) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			normalize8(a, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i)
			if (a[i] >= mod) a[i] -= mod;
	}
	static void product(U *dst, const U *a, const U *b, int n) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			product8(dst, a, b, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) dst[i] = mul(a[i], b[i]);
	}
#if CP_X86
	__attribute__((target("avx2"), always_inline)) void partial_stage8(U *a, int stride, int k) const {
		const I8 p = _mm256_set1_epi32(twice);
		const I8 r = _mm256_set1_epi32(roots[k]), q = _mm256_set1_epi32(root_quotients[k]);
		for (int i = 0; i < stride; i += 8) {
			I8 x = load8(a + i), y = load8(a + stride + i);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			y = k ? fixed_mul8(y, r, q) : _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			store8(a + i, _mm256_add_epi32(x, y));
			store8(a + stride + i, _mm256_sub_epi32(_mm256_add_epi32(x, p), y));
		}
	}
	template <int count, bool second = false>
	__attribute__((target("avx2"), always_inline)) void inverse_leaf_products8(U *a, U *b, const U *ws, const U *qs) const {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		alignas(32) U window[count][16], coefficients[count][8];
		I8 e[count], o[count];
		for (int k = 0; k < count; ++k) {
			I8 x = load8(a + 8 * k), y = load8(b + 8 * k);
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
			x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
			if constexpr (!second) y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
			I8 low = fixed_mul8(x, _mm256_set1_epi32(ws[k]), _mm256_set1_epi32(qs[k]));
			store8(window[k], _mm256_min_epu32(low, _mm256_sub_epi32(low, p)));
			store8(window[k] + 8, x);
			store8(coefficients[k], y);
			if constexpr (!second) store8(b + 8 * k, y);
			e[k] = o[k] = _mm256_setzero_si256();
		}
		for (int i = 0; i < 8; ++i)
			for (int k = 0; k < count; ++k) {
				I8 z = load8(window[k] + 8 - i), r = _mm256_set1_epi32(coefficients[k][i]);
				e[k] = _mm256_add_epi64(e[k], _mm256_mul_epu32(z, r));
				o[k] = _mm256_add_epi64(o[k], _mm256_mul_epu32(_mm256_srli_epi64(z, 32), r));
			}
		for (int k = 0; k < count; ++k) {
			I8 x = reduce8(e[k], o[k]);
			store8(a + 8 * k, _mm256_min_epu32(x, _mm256_sub_epi32(x, pp)));
		}
	}
	__attribute__((target("avx2"))) static U reverse_dot8(const U *a, const U *b, int n) {
		I8 p = _mm256_set1_epi32(mod), pp = _mm256_set1_epi32(twice);
		I8 reversed = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0), total = _mm256_setzero_si256();
		int i = 0;
		for (; i + 8 <= n;) {
			I8 even = _mm256_setzero_si256(), odd = even;
			int end = std::min(n / 8 * 8, i + 64);
			for (; i < end; i += 8) {
				I8 x = load8(a + i), y = _mm256_permutevar8x32_epi32(load8(b - i - 7), reversed);
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, pp));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, pp));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
				y = _mm256_min_epu32(y, _mm256_sub_epi32(y, p));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, y));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
			}
			I8 value = reduce8(even, odd);
			value = _mm256_min_epu32(value, _mm256_sub_epi32(value, pp));
			total = add8(total, value);
		}
		total = add8(total, _mm256_permute2x128_si256(total, total, 1));
		total = add8(total, _mm256_shuffle_epi32(total, 0x4e));
		total = add8(total, _mm256_shuffle_epi32(total, 0xb1));
		U result = _mm_cvtsi128_si32(_mm256_castsi256_si128(total));
		for (; i < n; ++i) {
			U x = a[i], y = b[-i];
			if (x >= twice) x -= twice;
			if (y >= twice) y -= twice;
			if (x >= mod) x -= mod;
			if (y >= mod) y -= mod;
			result = add(result, mul(x, y));
		}
		return result;
	}

	template <bool inverse>
	__attribute__((target("avx2"))) void inverse_partial8(U *a, int n, int limit = 0) const {
		if (!limit) limit = n;
		if constexpr (!inverse) {
			int stride = limit / 2;
			for (; stride >= 16; stride >>= 2) radix4_8<false>(a, stride / 2, n);
			if (stride == 8)
				for (int i = 0; i < n; i += 16) partial_stage8(a + i, 8, i / 16);
		} else {
			int stride = 8;
			for (; 4 * stride <= n; stride <<= 2) radix4_8<true>(a, stride, n);
			if (stride < n) stage8<true>(a, stride, n);
		}
	}
	__attribute__((target("avx2"))) void inverse_newton8(U *a, U *b, int n) const {
		assert(n >= 64 && !(n & (n - 1)) && n <= (1 << 23) && static_cast<int>(roots.size()) >= n / 16);
		std::copy_n(b, n / 2, b + n / 2);
		inverse_partial8<false>(b, n, n / 2);
		inverse_partial8<false>(a, n);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = (k & 1) ? mod - roots[k / 2] : roots[k / 2];
				qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
			}
			inverse_leaf_products8<8>(a + i, b + i, ws, qs);
		}
		inverse_partial8<true>(a, n);
		std::fill_n(a, n / 2, U(0));
		inverse_partial8<false>(a, n);
		for (int i = 0; i < n; i += 64) {
			U ws[8], qs[8];
			for (int j = 0; j < 8; ++j) {
				int k = i / 8 + j;
				ws[j] = (k & 1) ? mod - roots[k / 2] : roots[k / 2];
				qs[j] = (k & 1) ? ~root_quotients[k / 2] : root_quotients[k / 2];
			}
			inverse_leaf_products8<8, true>(a + i, b + i, ws, qs);
		}
		inverse_partial8<true>(a, n);
		U norm = mod - (mod - 1) / (n / 8);
		scale8(a + n / 2, n / 2, mod - (unsigned long long)norm * norm % mod);
	}

	template <bool odd>
	__attribute__((target("avx2"))) static void reduce_pairs8(U *a, U *b, const U *twist, int n) {
		for (int i = 0; i < n; i += 8) {
			I8 x, y, u, v;
			load_pairs(a + 2 * i, x, y);
			load_pairs(b + 2 * i, u, v);
			x = mul8(x, v);
			y = mul8(y, u);
			if constexpr (odd) x = mul8(sub8(x, y), load8(twist + i));
			else x = add8(x, y);
			store8(a + i, x);
			store8(b + i, mul8(u, v));
		}
	}
	__attribute__((target("avx2"))) static void combine8(U *a, const U *b, const U *x, const U *y, int n) {
		for (int i = 0; i < n; i += 8) store8(a + i, add8(mul8(load8(a + i), load8(y + i)), mul8(load8(b + i), load8(x + i))));
	}
#endif

	template <bool odd>
	static void reduce_pairs(U *a, U *b, const U *twist, int n) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			reduce_pairs8<odd>(a, b, twist, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) {
			U u = mul(a[2 * i], b[2 * i + 1]), v = mul(a[2 * i + 1], b[2 * i]);
			a[i] = odd ? mul(sub(u, v), twist[i]) : add(u, v);
			b[i] = mul(b[2 * i], b[2 * i + 1]);
		}
	}
	static void combine(U *a, const U *b, const U *x, const U *y, int n) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			combine8(a, b, x, y, n);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) a[i] = add(mul(a[i], y[i]), mul(b[i], x[i]));
	}
#if CP_X86
	__attribute__((target("avx2"))) static void geometric8(U *a, int n, U first, U step) {
		alignas(32) U factors[8];
		U power = one;
		for (int i = 0; i < 8; ++i) {
			factors[i] = mul(first, power);
			power = mul(power, step);
		}
		I8 r = load8(factors), ratio = _mm256_set1_epi32(power);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			store8(a + i, mul8(load8(a + i), r));
			r = mul8(r, ratio);
		}
		store8(factors, r);
		for (int j = 0; i < n; ++i, ++j) a[i] = mul(a[i], factors[j]);
	}
#endif
	static void geometric(U *a, int n, U first, U step) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			geometric8(a, n, first, step);
			return;
		}
#endif
		int i = 0;
		for (; i < n; ++i) {
			a[i] = mul(a[i], first);
			first = mul(first, step);
		}
	}
#if CP_X86
	__attribute__((target("avx2"))) static void add_scaled8(U *a, const U *b, int n, U r) {
		U q = quotient(r);
		I8 v = _mm256_set1_epi32(r), vq = _mm256_set1_epi32(q);
		int i = 0;
		for (; i + 8 <= n; i += 8) store8(a + i, add8(load8(a + i), fixed_mul8(load8(b + i), v, vq)));
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
#endif
	static void add_scaled(U *a, const U *b, int n, U r) {
#if CP_X86
		if (n >= 8 && vectorized()) {
			add_scaled8(a, b, n, r);
			return;
		}
#endif
		int i = 0;
		U q = quotient(r);
		for (; i < n; ++i) a[i] = add(a[i], fixed_mul(b[i], r, q));
	}
	static void scale_copy(U *dst, const U *src, int length, U r) {
#if CP_X86
		if (length >= 8 && vectorized()) {
			scale_copy8(dst, src, length, r);
			return;
		}
#endif
		U q = quotient(r);
		int i = 0;
		for (; i < length; ++i) dst[i] = fixed_mul(src[i], r, q);
	}
	void scale(U *a, int length, U r) const {
		scale_copy(a, a, length, r);
	}
	struct CompositionDelete {
		void operator()(unsigned int *p) const { ::operator delete[](p, std::align_val_t(64)); }
	};
	using CompositionWords = std::unique_ptr<unsigned int[], CompositionDelete>;

};
inline NTT &ntt_plan(int size) {
	static NTT t(1, false);
	if (static_cast<int>(t.roots.size()) < size) t = NTT(size, false);
	return t;
}

}

inline void ntt(std::vector<mint> &a, bool inverse = false) {
	assert(!a.empty() && !(a.size() & (a.size() - 1)) && a.size() <= (1U << 23));
	int n = static_cast<int>(a.size());
	auto &t = fps_detail::ntt_plan(std::max(1, n / 2));
	auto reorder = [&] {
		for (int i = 1, j = 0; i < n; ++i) {
			int bit = n >> 1;
			for (; j & bit; bit >>= 1) j ^= bit;
			j ^= bit;
			if (i < j) std::swap(a[i], a[j]);
		}
	};
	static_assert(sizeof(mint) == sizeof(fps_detail::NTT::U) && std::is_trivially_copyable_v<mint>);
	auto *words = reinterpret_cast<fps_detail::NTT::U *>(a.data());
	if (inverse) {
		reorder();
		t.fft<true>(words, n);
		t.scale(words, n, mint(n).inv().val());
	} else {
		t.fft<false>(words, n);
		reorder();
	}
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 7 "components/fps_operations.hpp"
#include <numeric>
#line 10 "components/fps_operations.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_API
template<class T> struct FormalPowerSeries;
#endif

namespace fps_detail {

template<class T, class Work = mint>
struct Operations {
	using F = FormalPowerSeries<T>;
	using W = FormalPowerSeries<Work>;
	using base = std::vector<T>;
	using Word = NTT::U;

	static W import_work(const base &a) {
		W r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = mint::raw(a[i].val());
		return r;
	}
	static F export_work(const W &a) {
		F r(a.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) r[i] = T::raw(a[i].val());
		return r;
	}
	static NTT &transform(int n) { return ntt_plan(n); }
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
		if constexpr (!std::is_same_v<T, mint>) t.normalize(words(a), n);
	}
	static void ntt(base &a) { transform_inplace<false>(a); }
	static void ntt_half(Word *a, int n) {
		std::copy_n(a, n / 2, a + n / 2);
		transform(std::max(1, n / 2)).template fft<false>(a, n, n / 2);
	}
	static void ntt_half(base &a) { ntt_half(words(a), a.size()); }
	static void ntt_inverse(base &a) { transform_inplace<true>(a); }
	static void scale_words(base &a, T factor) {
		if (a.empty()) return;
		if constexpr (std::is_same_v<T, mint>) {
			auto &t = transform(1);
			t.scale(words(a), a.size(), factor.val());
		} else
			for (T &x : a) x *= factor;
	}
	static void geometric_scale(base &a, T first, T step) {
		if constexpr (std::is_same_v<T, mint>) NTT::geometric(words(a), a.size(), first.v, step.v);
		else
			for (T &x : a) {
				x *= first;
				first *= step;
			}
	}
	static void batch_inverse(base &dst, const base &src) {
		int n = src.size();
#if CP_X86
		if constexpr (std::is_same_v<T, mint>) {
			if (n >= 128 && NTT::vectorized()) {
				NTT::batch_inverse8(words(dst), words(src), n);
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
		if constexpr (std::is_same_v<T, mint>) NTT::product(words(r), words(a), words(b), n);
		else
			for (int i = 0; i < n; ++i) r[i] = a[i] * b[i];
	}
	template <int count, bool first>
	static void accumulate_products(Word *dst, const Word *const *a, const Word *const *b, int n) {
		static_assert(count >= 1 && count <= 7);
		for (int i = 0; i < n; ++i) {
			Word value = 0;
			for (int k = 0; k < count; ++k) value = NTT::add(value, NTT::mul(a[k][i], b[k][i]));
			if constexpr (!first) value = NTT::add(value, dst[i]);
			dst[i] = value;
		}
	}
#if CP_X86
	template<int count, bool first>
	__attribute__((target("avx2"))) static void accumulate_products8(Word *dst, const Word *const *a, const Word *const *b, int n) {
		accumulate_products<count, first>(dst, a, b, n);
	}
#endif
	static void join_windows(Word *blocks, int length, int count) {
		join_windows_strided(blocks, length, count, length);
	}
	static void join_windows_strided(Word *blocks, int length, int count, int pitch) {
		for (int i = count - 1; i > 0; --i) {
			Word *a = blocks + i * pitch;
			const Word *b = a - pitch;
			for (int j = 0; j < length / 2; ++j) a[j] = NTT::add(a[j], b[j]);
			for (int j = length / 2; j < length; ++j) a[j] = NTT::sub(a[j], b[j]);
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

#if CP_X86
		if constexpr (std::is_same_v<T, mint>) {
			if (n >= 64 && NTT::vectorized())
				return T::word(NTT::reverse_dot8(reinterpret_cast<const Word *>(a), reinterpret_cast<const Word *>(b), n));
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

	static void intt(base &a) {
		ntt_inverse(a);
		const T inv = T::raw(T::mod() - (T::mod() - 1) / static_cast<int>(a.size()));
		scale_words(a, inv);
	}
	static T mod_sqrt(T a) {
		if (a == T(0)) return 0;
		T t = a.pow(59), r = t * a, c = T(3).pow(119);
		t *= r;
		int m = 23;
		while (t != T(1)) {
			int i = 0;
			for (T u = t; u != T(1) && i < m; u *= u) ++i;
			if (i == m) return 0;
			T b = c.pow(1 << (m - i - 1));
			r *= b;
			c = b * b;
			t *= c;
			m = i;
		}
		return r;
	}
	template<bool Inverse>
	static const base &roots(int n) {
		static base values{1};
		while (static_cast<int>(values.size()) < n) {
			const int m = static_cast<int>(values.size());
			T step = T(3).pow((T::mod() - 1) / (2 * m));
			if constexpr (Inverse) step = step.inv();
			values.resize(2 * m);
			for (int i = 0; i < m; ++i) values[m + i] = values[i] * step;
		}
		return values;
	}
	static const base &inverse_ntt_roots(int n) { return roots<true>(n); }
	static const base &ntt_roots(int n) { return roots<false>(n); }
	static const std::vector<T> &inverses(int n) {
		assert(unsigned(n) < unsigned(T::mod()));
		static std::vector<T> inv{0, 1};
#if CP_X86
		if constexpr (std::is_same_v<T, mint>) {
			int first = inv.size(), count = (n + 1 - first) / 8 * 8;
			if (count >= 256 && NTT::vectorized()) {
				inv.reserve(n + 1);
				inv.resize(first + count);
				NTT::inverse_sequence8(reinterpret_cast<Word *>(inv.data()) + first, first, count);
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

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 1 "components/fps_horner.hpp"



#line 5 "components/fps_horner.hpp"
#include <array>
#include <cstring>

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {
using mint_detail::Ops;
namespace horner_detail {
template<int K, class T>
inline T scalar(const T* a, std::size_t n, T x) {
	static_assert(K > 0);
	T step = 1;
	for (int i = 0; i < K; ++i) step *= x;
	std::array<T, K> sum{};
	std::size_t groups = n / K, tail = n % K;
	for (std::size_t i = 0; i < tail; ++i) sum[i] = a[groups * K + i];
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) sum[i] = sum[i] * step + a[groups * K + i];
	}
	T result = 0;
	for (int i = K; i-- > 0;) result = result * x + sum[i];
	return result;
}

#if CP_X86
template<int K>
__attribute__((target("avx2"))) inline mint vector(const mint* a, std::size_t n, mint x) {
	static_assert(K == 1 || K == 2 || K == 4);
	using V = Ops::I8;
	static_assert(std::is_trivially_copyable_v<mint> && sizeof(mint) == sizeof(unsigned));
	mint step = x;
	for (int i = 0; i < 3; ++i) step *= step;
	const unsigned x8 = step.val();
	for (int i = 1; i < K; i *= 2) step *= step;
	const unsigned value = step.val();
	const V factor = _mm256_set1_epi32(value), quotient = _mm256_set1_epi32(Ops::quotient(value));
	V sum[K];
	for (auto& v : sum) v = _mm256_setzero_si256();
	constexpr std::size_t width = 8 * K;
	std::size_t groups = n / width, tail = n % width;
	if (tail) {
		alignas(32) mint block[width]{};
		std::memcpy(static_cast<void*>(block), a + groups * width, tail * sizeof(mint));
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], block + 8 * i, sizeof(V));
	} else if (groups) {
		--groups;
		for (int i = 0; i < K; ++i) std::memcpy(&sum[i], a + groups * width + 8 * i, sizeof(V));
	}
	while (groups) {
		--groups;
		for (int i = 0; i < K; ++i) {
			V coefficient;
			std::memcpy(&coefficient, a + groups * width + 8 * i, sizeof(V));
			sum[i] = Ops::add8(value == 1 ? sum[i] : Ops::fixed_mul8(sum[i], factor, quotient), coefficient);
		}
	}
	const V step8 = _mm256_set1_epi32(x8), quotient8 = _mm256_set1_epi32(Ops::quotient(x8));
	V joined = sum[K - 1];
	for (int i = K - 1; i-- > 0;) joined = Ops::add8(Ops::fixed_mul8(joined, step8, quotient8), sum[i]);
	alignas(32) mint word[8];
	std::memcpy(static_cast<void*>(word), &joined, sizeof(joined));
	mint result = 0;
	for (int i = 8; i-- > 0;) result = result * x + word[i];
	return result;
}
#endif
}

inline mint horner(const mint* a, std::size_t n, mint x) {
	if (!n) return mint(0);
	if (x == mint(0)) return a[0];
#if CP_X86
	if (n >= 32 && Ops::vectorized()) {
		if (n < 64) return horner_detail::vector<1>(a, n, x);
		return horner_detail::vector<4>(a, n, x);
	}
#endif
	if (n >= 32) return horner_detail::scalar<8>(a, n, x);
	mint result = 0;
	while (n) result = result * x + a[--n];
	return result;
}
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 1 "components/fps_workspace.hpp"



#line 5 "components/fps_workspace.hpp"
#include <cstdint>
#include <limits>
#line 9 "components/fps_workspace.hpp"
#if defined(__linux__)
#include <sys/mman.h>
#endif

namespace fps_detail {

struct FpsWordsDelete {
	void *region = nullptr;
	std::size_t bytes = 0;
	void operator()(unsigned int *p) const {
#if defined(__linux__)
		if (region) {
			munmap(region, bytes);
			return;
		}
#endif
		::operator delete[](p, std::align_val_t(64));
	}
};

using FpsWords = std::unique_ptr<unsigned int[], FpsWordsDelete>;

inline FpsWords allocate_fps_words(std::size_t count) {
#if defined(__linux__) && defined(MADV_HUGEPAGE)
	constexpr std::size_t page = 1 << 21;
	if (count >= page / sizeof(unsigned int) &&
		count <= (std::numeric_limits<std::size_t>::max() - 2 * page) / sizeof(unsigned int)) {
		const std::size_t bytes = (count * sizeof(unsigned int) + page - 1) / page * page;
		void *region = mmap(nullptr, bytes + page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (region != MAP_FAILED) {
			const auto address = (reinterpret_cast<std::uintptr_t>(region) + page - 1) & ~(page - 1);
			auto *p = ::new (reinterpret_cast<void *>(address)) unsigned int[count];
			madvise(p, bytes, MADV_HUGEPAGE);
			return FpsWords(p, FpsWordsDelete{region, bytes + page});
		}
	}
#endif
	return FpsWords(new (std::align_val_t(64)) unsigned int[count]);
}

}


#line 1 "components/fps_sparse.hpp"



#line 7 "components/fps_sparse.hpp"
#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#line 1 "components/fps_sparse_core.hpp"



#line 1 "components/fps_sparse_terms.hpp"



#line 10 "components/fps_sparse_terms.hpp"

namespace fps_detail {
struct SparseTerms {
	using Series = std::vector<mint>;
	using Terms = std::vector<std::pair<int, mint>>;
	enum class Kind { inverse, logarithm, exponential, power };
	static Terms collect(const mint *f, int size, int n, int limit = 12) {
		Terms terms;
		for (int i = 1; i < std::min(size, n) && int(terms.size()) <= limit; ++i)
			if (f[i] != mint(0)) terms.emplace_back(i, f[i]);
		return terms;
	}
};
}

#line 10 "components/fps_sparse_core.hpp"

namespace fps_detail {
template <class E> struct SparseBase : SparseTerms {
	static Terms multiply(const Terms &a, const Terms &b, int n) {
		Terms result;
		for (auto [i, x] : a) for (auto [j, y] : b) if (i < n - j) result.emplace_back(i + j, x * y);
		std::sort(result.begin(), result.end(), [](auto a, auto b) { return a.first < b.first; });
		std::size_t used = 0;
		for (auto [i, x] : result) {
			if (used && result[used - 1].first == i) result[used - 1].second += x;
			else result[used++] = {i, x};
		}
		result.resize(used);
		result.erase(std::remove_if(result.begin(), result.end(), [](auto term) { return term.second == mint(0); }), result.end());
		return result;
	}
	template <Kind kind>
	static Series get(Terms terms, int n, mint initial, mint exponent, const mint *(*inverses)(int)) {
		assert(n >= 0 && terms.size() <= 64);
		if constexpr (kind != Kind::inverse) assert(unsigned(n) < mint::mod());
		if (!n) return {};
		if (terms.empty()) { Series out(n); out[0] = initial; return out; }
		if constexpr (kind == Kind::power) {
			unsigned e = exponent.val();
			if (e <= 4 && terms.size() <= 12 && (n >= 4096 || terms.size() <= 3)) {
				Terms power{{0, 1}}, f{{0, 1}};
				f.insert(f.end(), terms.begin(), terms.end());
				while (e) {
					if (e & 1) power = multiply(power, f, n);
					e >>= 1;
					if (e) f = multiply(f, f, n);
				}
				Series out(n);
				for (auto [i, x] : power) out[i] = x * initial;
				return out;
			}
		}
		int stride = 0;
		for (auto term : terms) stride = std::gcd(stride, term.first);
		int count = (n - 1) / stride + 1;
		for (auto &term : terms) term.first /= stride;
		if constexpr (kind == Kind::power) {
			const auto degree = 1LL * terms.back().first * exponent.val() + 1;
			if (2 * degree <= count) count = degree;
		}
		const mint *inv = nullptr;
		if constexpr (kind != Kind::inverse) inv = inverses(count);
		Series compressed;
		if (terms.size() == 1 && kind != Kind::power) {
			compressed.resize(count);
			if constexpr (kind == Kind::exponential) {
				E::template geometric<true>(compressed, initial, terms[0].second, inv);
			} else {
				E::geometric(compressed, kind == Kind::logarithm ? mint(-1) : initial, -terms[0].second);
				if constexpr (kind == Kind::logarithm) compressed[0] = 0, E::scale(compressed, inv);
			}
		} else compressed = E::template solve<kind>(terms, count, initial, exponent, inv);
		if (stride == 1 && (kind != Kind::power || count == n)) return compressed;
		Series out(n);
		for (int i = 0; i < count; ++i) out[i * stride] = compressed[i];
		return out;
	}
};
}

#line 13 "components/fps_sparse.hpp"

namespace fps_detail {
struct SparseSeries : SparseBase<SparseSeries> {
	struct Tap { int degree; std::uint32_t constant, slope; };
	using O = mint_detail::Ops;
	using U = O::U;
	static_assert(12ULL * (O::mod - 1) * (O::mod - 1) + 0xffffffffULL * O::mod < ~0ULL);
	static U canonical(U x) { return x < O::mod ? x : x - O::mod; }
#if CP_X86
	using V = O::I8;
	__attribute__((target("avx2"))) static V canonical8(V x) {
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(O::twice)));
		return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(O::mod)));
	}
	__attribute__((target("avx2"))) static V reduce8(V even, V odd) {
		return canonical8(O::reduce8(even, odd));
	}
	__attribute__((target("avx2"), always_inline)) static inline V triangular(V x, const U *f) {
		const V lower = _mm256_permute2x128_si256(x, x, 0x08);
		V shifted[8] = {x, _mm256_alignr_epi8(x, lower, 12), _mm256_alignr_epi8(x, lower, 8), _mm256_alignr_epi8(x, lower, 4),
			lower, _mm256_slli_si256(lower, 4), _mm256_slli_si256(lower, 8), _mm256_slli_si256(lower, 12)};
		V even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 7
		for (int j = 1; j < 8; ++j) if (f[j]) {
			V factor = _mm256_set1_epi32(f[j]);
			even = _mm256_add_epi64(even, _mm256_mul_epu32(shifted[j], factor));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(shifted[j], 32), factor));
		}
		return canonical8(O::add8(x, reduce8(even, odd)));
	}
	template <Kind kind>
	__attribute__((target("avx2"), always_inline)) static inline void block(U *out, int i, const Tap *taps, int near, int count, const U *inv, const U *p, const U *q, V *coefficients, const U *steps) {
		V total = _mm256_setzero_si256();
		const V lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		for (int first = 0; first < count; first += 12) {
			V ce = _mm256_setzero_si256(), co = ce;
			for (int j = first; j < std::min(count, first + 12); ++j) {
				V x = O::load8(out + i - taps[j].degree);
				if (j < near) x = _mm256_and_si256(x, _mm256_cmpgt_epi32(_mm256_set1_epi32(taps[j].degree), lanes));
				V c;
				if constexpr (kind == Kind::power) c = coefficients[j];
				else c = _mm256_set1_epi32(taps[j].constant);
				ce = _mm256_add_epi64(ce, _mm256_mul_epu32(x, c));
				co = _mm256_add_epi64(co, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), kind == Kind::power ? _mm256_srli_epi64(c, 32) : c));
				if constexpr (kind == Kind::power) {
					V next = _mm256_add_epi32(c, _mm256_set1_epi32(steps[j]));
					coefficients[j] = _mm256_min_epu32(next, _mm256_sub_epi32(next, _mm256_set1_epi32(O::mod)));
				}
			}
			total = O::add8(total, reduce8(ce, co));
		}
		V value = canonical8(total);
		if constexpr (kind == Kind::logarithm) value = O::add8(value, O::load8(out + i));
		if constexpr (kind == Kind::power || kind == Kind::exponential) {
			if (near) value = triangular(canonical8(value), q);
			value = O::mul8(value, O::load8(inv + i));
		}
		if (near) value = triangular(canonical8(value), p);
		O::store8(out + i, canonical8(value));
	}
	template <Kind kind>
	__attribute__((target("avx2"))) static int homogeneous(U *out, int i, int end, const Tap *taps, int count) {
		const int width = taps[count - 1].degree;
		if constexpr (kind == Kind::logarithm) if (i == width) {
			mint value = mint::word(out[i]);
			for (int j = 0; j < count; ++j) value += mint::word(taps[j].constant) * mint::word(out[i - taps[j].degree]);
			out[i++] = canonical(value.v);
		}
		V coefficients[8][15];
		for (int j = 0; j < width; ++j) {
			U words[64];
			for (int t = 0; t < 64; ++t) {
				U value = 0;
				for (int first = 0; first < count; first += 12) {
					std::uint64_t sum = 0;
					for (int k = first; k < std::min(count, first + 12); ++k) {
						int d = taps[k].degree;
						U x = t >= d ? words[t - d] : j == d - t - 1 ? O::one : 0;
						sum += std::uint64_t(taps[k].constant) * x;
					}
					value = canonical(O::add(value, O::reduce(sum) % O::mod));
				}
				words[t] = value;
			}
			for (int b = 0; b < 8; ++b) coefficients[b][j] = O::load8(words + 8 * b);
		}
		for (; end - i >= 64; i += 64) {
			for (int block = 0; block < 8; ++block) {
				int b = block == 0 ? 7 : block - 1;
				V total = _mm256_setzero_si256();
				for (int first = 0; first < width; first += 12) {
					V even = _mm256_setzero_si256(), odd = even;
					for (int j = first; j < std::min(width, first + 12); ++j) {
						V x = _mm256_set1_epi32(out[i - 1 - j]), c = coefficients[b][j];
						even = _mm256_add_epi64(even, _mm256_mul_epu32(x, c));
						odd = _mm256_add_epi64(odd, _mm256_mul_epu32(x, _mm256_srli_epi64(c, 32)));
					}
					total = O::add8(total, reduce8(even, odd));
				}
				O::store8(out + i + 8 * b, canonical8(total));
			}
		}
		return i;
	}

	template <Kind kind, int groups>
	__attribute__((target("avx2"))) static int chained(U *out, int i, int end, int width, const U *inv, const U *p, const V *coefficients, const V *steps) {
		V total = _mm256_setzero_si256();
		for (int first = 0; first < width; first += 12) {
			V even = _mm256_setzero_si256(), odd = even;
			for (int j = first; j < std::min(width, first + 12); ++j) {
				V x = _mm256_set1_epi32(out[i - 1 - j]), c = coefficients[j];
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, c));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(x, _mm256_srli_epi64(c, 32)));
			}
			total = O::add8(total, reduce8(even, odd));
		}
		V h = canonical8(O::mul8(canonical8(total), O::load8(inv + i))), previous{};
		if constexpr (groups == 2) {
			U inverse[8] = {O::one};
			for (int t = 1; t < 8; ++t) {
				mint value = 0;
				for (int d = 1; d <= t; ++d) value -= mint::word(p[d]) * mint::word(inverse[t - d]);
				inverse[t] = canonical(value.v);
			}
			previous = triangular(O::load8(out + i - 8), inverse);
		}
		V matrix[8 * groups], delta[8 * groups];
		for (int group = 0; group < groups; ++group) for (int s = 0; s < 8; ++s) {
			V even = _mm256_setzero_si256(), odd = even, de = even, dodd = even;
			for (int j = 8 * group; j < width && j + s < 8 * group + 8; ++j) {
				V c = coefficients[j], factor = _mm256_set1_epi32(p[7 + 8 * group - j - s]);
				if constexpr (kind == Kind::power) c = canonical8(O::add8(c, steps[j]));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(c, factor));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(c, 32), factor));
				if constexpr (kind == Kind::power) {
					de = _mm256_add_epi64(de, _mm256_mul_epu32(steps[j], factor));
					dodd = _mm256_add_epi64(dodd, _mm256_mul_epu32(_mm256_srli_epi64(steps[j], 32), factor));
				}
			}
			matrix[8 * group + s] = reduce8(even, odd);
			if constexpr (kind == Kind::power) delta[8 * group + s] = reduce8(de, dodd);
		}
		for (; end - i >= 16; i += 8) {
			total = _mm256_setzero_si256();
			for (int group = 0; group < groups; ++group) {
				V value = group ? previous : h;
				V low = _mm256_permute2x128_si256(value, value, 0x00), high = _mm256_permute2x128_si256(value, value, 0x11);
				V x[8] = {_mm256_shuffle_epi32(low, 0x00), _mm256_shuffle_epi32(low, 0x55), _mm256_shuffle_epi32(low, 0xaa), _mm256_shuffle_epi32(low, 0xff),
					_mm256_shuffle_epi32(high, 0x00), _mm256_shuffle_epi32(high, 0x55), _mm256_shuffle_epi32(high, 0xaa), _mm256_shuffle_epi32(high, 0xff)};
				V even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
				for (int j = 0; j < 8; ++j) {
					V c = matrix[8 * group + j];
					even = _mm256_add_epi64(even, _mm256_mul_epu32(x[j], c));
					odd = _mm256_add_epi64(odd, _mm256_mul_epu32(x[j], _mm256_srli_epi64(c, 32)));
				}
				if constexpr (groups == 1) total = reduce8(even, odd);
				else total = group ? _mm256_add_epi32(total, reduce8(even, odd)) : reduce8(even, odd);
			}
			if constexpr (groups == 2) total = _mm256_min_epu32(total, _mm256_sub_epi32(total, _mm256_set1_epi32(O::mod)));
			V next = O::mul8(total, O::load8(inv + i + 8));
			next = _mm256_min_epu32(next, _mm256_sub_epi32(next, _mm256_set1_epi32(O::mod)));
			O::store8(out + i, triangular(h, p));
			if constexpr (groups == 2) previous = h;
			h = next;
			if constexpr (kind == Kind::power) for (int j = 0; j < 8 * groups; ++j) {
				V value = _mm256_add_epi32(matrix[j], delta[j]);
				matrix[j] = _mm256_min_epu32(value, _mm256_sub_epi32(value, _mm256_set1_epi32(O::mod)));
			}
		}
		O::store8(out + i, triangular(h, p));
		return i + 8;
	}

	template <Kind kind>
	__attribute__((target("avx2"))) static int near_blocks(U *out, int i, int end, const Tap *taps, int count, const U *inv, const U *p, const U *q) {
		const int width = taps[count - 1].degree;
		if constexpr (kind == Kind::logarithm) if (i == width) {
			mint value = mint::word(out[i]);
			for (int j = 0; j < count; ++j) value += mint::word(taps[j].constant) * mint::word(out[i - taps[j].degree]);
			out[i++] = canonical(value.v);
		}
		constexpr int limit = kind == Kind::power || kind == Kind::exponential ? 16 : 8;
		std::array<mint, limit> constant{}, slope{};
		for (int j = 0; j < count; ++j) {
			constant[taps[j].degree] = mint::word(taps[j].constant);
			slope[taps[j].degree] = mint::word(taps[j].slope);
		}
		V coefficients[limit - 1], steps[limit - 1];
		for (int j = 0; j < width; ++j) {
			std::array<std::uint32_t, 8> first{}, step{};
			for (int t = 0; t < 8; ++t) {
				mint a = 0, b = 0;
				for (int u = 0; u <= t && u + j < width; ++u) {
					const int d = u + j + 1;
					mint factor = mint::word((kind == Kind::power || kind == Kind::exponential ? q : p)[t - u]);
					a += factor * (constant[d] + mint(u) * slope[d]);
					if constexpr (kind == Kind::power) b += factor * slope[d];
				}
				first[t] = canonical((a + mint(i) * b).v);
				if constexpr (kind == Kind::power) step[t] = canonical((mint(8) * b).v);
			}
			coefficients[j] = O::load8(first.data());
			if constexpr (kind == Kind::power) steps[j] = O::load8(step.data());
		}
		if constexpr (kind == Kind::power || kind == Kind::exponential) if (end - i >= 8192) {
			if (width <= 8) return chained<kind, 1>(out, i, end, width, inv, p, coefficients, steps);
			return chained<kind, 2>(out, i, end, width, inv, p, coefficients, steps);
		}
		for (; end - i >= 8; i += 8) {
			V even = _mm256_setzero_si256(), odd = even;
			for (int j = 0; j < width; ++j) {
				V x = _mm256_set1_epi32(out[i - 1 - j]), c = coefficients[j];
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, c));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(x, _mm256_srli_epi64(c, 32)));
				if constexpr (kind == Kind::power) {
					V next = _mm256_add_epi32(c, steps[j]);
					coefficients[j] = _mm256_min_epu32(next, _mm256_sub_epi32(next, _mm256_set1_epi32(O::mod)));
				}
			}
			V value = reduce8(even, odd);
			if constexpr (kind == Kind::power || kind == Kind::exponential) value = triangular(canonical8(O::mul8(value, O::load8(inv + i))), p);
			O::store8(out + i, value);
		}
		return i;
	}
	template <Kind kind>
	__attribute__((target("avx2"))) static int blocks(U *out, int i, int end, const Tap *taps, int near, int count, const U *inv, const U *p, const U *q) {
		if (end - i < 8) return i;
		if constexpr (kind == Kind::inverse || kind == Kind::logarithm)
			if (count && taps[count - 1].degree < 16 && end - i >= 8192) i = homogeneous<kind>(out, i, end, taps, count);
		if constexpr (kind == Kind::power || kind == Kind::exponential)
			if (near && taps[count - 1].degree < 16 && end - i >= 8192) return near_blocks<kind>(out, i, end, taps, count, inv, p, q);
		if (count == near && count && end - i >= 256) return near_blocks<kind>(out, i, end, taps, count, inv, p, q);
		V coefficients[64];
		U steps[64];
		if constexpr (kind == Kind::power) {
			const V offsets = _mm256_setr_epi32(0, O::one, O::in(2), O::in(3), O::in(4), O::in(5), O::in(6), O::in(7));
			const V index = O::add8(_mm256_set1_epi32(O::in(i)), offsets);
			for (int j = 0; j < count; ++j) {
				coefficients[j] = canonical8(O::add8(_mm256_set1_epi32(taps[j].constant), O::mul8(index, _mm256_set1_epi32(taps[j].slope))));
				steps[j] = canonical(O::mul(O::in(8), taps[j].slope));
			}
		}
		for (; end - i >= 8; i += 8) block<kind>(out, i, taps, near, count, inv, p, q, coefficients, steps);
		return i;
	}
	__attribute__((target("avx2"))) static int scale8(U *out, const U *factors, int n) {
		int i = 1;
		for (; n - i >= 8; i += 8) O::store8(out + i, O::mul8(O::load8(out + i), O::load8(factors + i)));
		return i;
	}
	template <bool exponential = false, int streams = 1>
	__attribute__((target("avx2"))) static void geometric8(U *out, int n, U initial, U ratio, const U *inv = nullptr) {
		std::uint32_t first[8];
		U step = O::one;
		first[0] = initial;
		for (int j = 0; j < 8; ++j) {
			step = O::mul(step, ratio);
			if (j) {
				first[j] = O::mul(first[j - 1], ratio);
				if constexpr (exponential) first[j] = O::mul(first[j], inv[j]);
			}
		}
		V value[streams], multiplier = _mm256_set1_epi32(step);
		value[0] = O::load8(first);
		for (int j = 1; j < streams; ++j) value[j] = O::mul8(value[j - 1], multiplier);
		for (int j = 0; j < streams; ++j) O::store8(out + 8 * j, value[j]);
		for (int j = 1; j < streams; j *= 2) step = O::mul(step, step);
		multiplier = _mm256_set1_epi32(step);
		int i = 8 * streams;
		for (; n - i >= 8 * streams; i += 8 * streams) for (int k = 0; k < streams; ++k) {
			value[k] = O::mul8(value[k], multiplier);
			if constexpr (exponential) {
				V factors[4];
#pragma GCC unroll 4
				for (int j = 0; j < 4; ++j) factors[j] = O::mul8(O::load8(inv + i - 2 * j), O::load8(inv + i - 2 * j - 1));
				value[k] = O::mul8(value[k], O::mul8(O::mul8(factors[0], factors[1]), O::mul8(factors[2], factors[3])));
			}
			O::store8(out + i + 8 * k, value[k]);
		}
		for (; i < n; ++i) {
			out[i] = O::mul(out[i - 1], ratio);
			if constexpr (exponential) out[i] = O::mul(out[i], inv[i]);
		}
	}

#endif
	template <bool exponential = false>
	static void geometric(Series &out, mint initial, mint ratio, const mint *inv = nullptr) {
#if CP_X86
		if (out.size() >= 8 && O::vectorized()) {
			if (!exponential && out.size() >= 256) geometric8<false, 4>(reinterpret_cast<U *>(out.data()), out.size(), initial.v, ratio.v);
			else geometric8<exponential>(reinterpret_cast<U *>(out.data()), out.size(), initial.v, ratio.v, reinterpret_cast<const U *>(inv));
			return;
		}
#endif
		for (int i = 0; i < int(out.size()); ++i) {
			out[i] = initial;
			initial *= ratio;
			if constexpr (exponential) if (i + 1 < int(out.size())) initial *= inv[i + 1];
		}
	}
	static void scale(Series &out, const mint *factors) {
		int i = 1;
#if CP_X86
		if (O::vectorized()) i = scale8(reinterpret_cast<U *>(out.data()), reinterpret_cast<const U *>(factors), out.size());
#endif
		for (; i < int(out.size()); ++i) out[i] *= factors[i];
	}
	template <Kind kind>
	static Series solve(const Terms &terms, int n, mint initial, mint exponent, const mint *inverses) {
		std::vector<Tap> taps;
		Series result(n);
		U *out = reinterpret_cast<U *>(result.data());
		const U *inv = reinterpret_cast<const U *>(inverses);
		for (auto [j, x] : terms) {
			mint c = 0, s = 0;
			if constexpr (kind == Kind::inverse || kind == Kind::logarithm) c = -x;
			if constexpr (kind == Kind::exponential) c = x * mint(j);
			if constexpr (kind == Kind::power) c = (exponent + mint(1)) * mint(j) * x, s = -x;
			taps.push_back({j, canonical(c.v), canonical(s.v)});
			if constexpr (kind == Kind::logarithm) out[j] = canonical((x * mint(j)).v);
		}
		std::array<std::uint32_t, 8> p{}, q{};
		p[0] = q[0] = O::one;
		if (n >= 8) for (int i = 1; i < 8; ++i) {
			mint a = 0, b = 0;
			for (const auto &tap : taps) {
				int j = tap.degree;
				if (j > i) break;
				mint c = mint::word(tap.constant), s = mint::word(tap.slope);
				a += (c + mint(i) * s) * mint::word(p[i - j]);
				b += (-c + mint(i - j) * s) * mint::word(q[i - j]);
			}
			if constexpr (kind == Kind::power || kind == Kind::exponential) a *= inverses[i], b *= inverses[i];
			p[i] = canonical(a.v), q[i] = canonical(b.v);
		}
		out[0] = canonical(initial.v);
		int active = 0, i = 1;
		[[maybe_unused]] int near = 0;
		while (i < n) {
			while (active < int(taps.size()) && taps[active].degree <= i) {
				near += taps[active].degree < 8;
				++active;
			}
			int end = active < int(taps.size()) ? taps[active].degree : n;
			if (!active) { i = end; continue; }
#if CP_X86
			if (O::vectorized()) i = blocks<kind>(out, i, end, taps.data(), near, active, inv, p.data(), q.data());
#endif
			for (; i < end; ++i) {
				U x = 0;
				for (int first = 0; first < active; first += 12) {
					std::uint64_t total = 0;
					for (int j = first; j < std::min(active, first + 12); ++j) {
						U c = taps[j].constant;
						if constexpr (kind == Kind::power) c = canonical(O::add(c, O::mul(O::in(i), taps[j].slope)));
						total += std::uint64_t(c) * out[i - taps[j].degree];
					}
					x = O::add(x, O::reduce(total) % O::mod);
				}
				if constexpr (kind == Kind::logarithm) x = O::add(x, out[i]);
				if constexpr (kind == Kind::power || kind == Kind::exponential) x = O::mul(x, inv[i]);
				out[i] = canonical(x);
			}
		}
		if constexpr (kind == Kind::logarithm) scale(result, inverses);
		return result;
	}

};
}
#if CP_GCC_X86
#pragma GCC pop_options
#endif

#line 8 "fps/fps.hpp"

#line 1 "components/fps_blocked.hpp"


#line 1 "components/fps_output.hpp"



#line 8 "components/fps_output.hpp"
#include <stdexcept>
#line 11 "components/fps_output.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#include <sys/mman.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {

inline void prepare_fps_output(std::vector<mint> &out, std::size_t n) {
	if (n <= out.capacity()) return;
	out.reserve(n);
	if (n - out.size() < (1u << 17)) return;
	const auto first = reinterpret_cast<std::uintptr_t>(out.data() + out.size());
	const auto end = reinterpret_cast<std::uintptr_t>(out.data() + n);
	[[maybe_unused]] const auto advise = [first, end](std::uintptr_t alignment, int advice) {
		const auto lo = (first + alignment - 1) & ~(alignment - 1), hi = end & ~(alignment - 1);
		if (lo < hi) static_cast<void>(madvise(reinterpret_cast<void *>(lo), hi - lo, advice));
	};
#if defined(MADV_HUGEPAGE)
	advise(2u * 1024 * 1024, MADV_HUGEPAGE);
#endif
#if defined(MADV_POPULATE_WRITE)
	advise(4096, MADV_POPULATE_WRITE);
#endif
}

template <bool montgomery>
inline void append_fps_words(std::vector<mint> &out, const unsigned *in, std::size_t n) {
	static_assert(sizeof(mint) == sizeof(unsigned) && std::is_trivially_copyable_v<mint>);
	if (n == 0) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + n);
	std::array<mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if constexpr (montgomery) std::memcpy(static_cast<void *>(batch.data()), in + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (int j = 0; j < 64; j += 8) {
				const auto x = mint_detail::Ops::mul8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i + j)), _mm256_set1_epi32(932051910u));
				std::memcpy(static_cast<void *>(batch.data() + j), &x, sizeof(x));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	for (; i < n; ++i) {
		if constexpr (montgomery) out.emplace_back(mint::word(in[i]));
		else out.emplace_back(mint::raw(in[i]));
	}
}
}
#pragma GCC pop_options
#endif

#line 8 "components/fps_blocked.hpp"
#include <bit>
#line 11 "components/fps_blocked.hpp"
#include <cstdlib>
#line 13 "components/fps_blocked.hpp"
#include <span>
#line 16 "components/fps_blocked.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
using Word = std::uint32_t;
using Words = std::span<Word>;
using ConstWords = std::span<const Word>;
using std::size_t;
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP, kNI = 998244351;

}
namespace blocked_ntt {

inline constexpr Word kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec, kernels::kP;

constexpr Word multiply_mod(Word x, Word y) {
	return Word(std::uint64_t(x) * y % kP);
}

constexpr Word power(Word x, Word e) {
	Word r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr Word quotient(Word w) { return Word((std::uint64_t(w) << 32) / kP); }

inline constexpr Word kR = Word((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<Word, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(Word x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, Word bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(Word value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(Word value, Word value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) { return mint_detail::Ops::fixed_mul8(x, f.w, f.q); }

inline size_t slot(size_t k) { return (k >> 3 << 4) | (k & 7); }

inline constexpr size_t table_words(int lg) { return (size_t(1) << lg) / 8 + 16; }

inline void build_table(Word* t, size_t count, const std::array<Word, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const size_t h = size_t(1) << level;
		if (h >= count) break;
		const Word root = roots[level];
		if (h < 8) {
			for (size_t j = 0; j < h; ++j) {
				const Word v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

}
}
namespace blocked_ntt::kernels {
using namespace detail;

template<bool Inverse, bool Identity>
inline void radix4(Vec* a, size_t h, const Word* x, const Word* y) {
	const Factor wx(Identity ? 1 : x[0], Identity ? 4 : x[8]);
	const Factor wy(Identity ? 1 : y[0], Identity ? 4 : y[8]), wz(y[1], y[9]);
#pragma GCC unroll 1
	for (size_t k = 0; k < h; ++k) {
		const Vec f0 = a[k], f1 = a[k + h], f2 = a[k + 2 * h], f3 = a[k + 3 * h];
		if constexpr (Inverse) {
			const Vec ab = reduce(add(f0, f1), 2 * kP), cd = reduce(add(f2, f3), 2 * kP);
			const Vec amb = Identity ? reduce(diff(f0, f1), 2 * kP) : multiply(diff(f0, f1), wy);
			const Vec cmd = multiply(diff(f2, f3), wz);
			a[k] = reduce(add(ab, cd), 2 * kP), a[k + h] = reduce(add(amb, cmd), 2 * kP);
			if constexpr (Identity) {
				a[k + 2 * h] = reduce(diff(ab, cd), 2 * kP);
				a[k + 3 * h] = reduce(diff(amb, cmd), 2 * kP);
			} else {
				a[k + 2 * h] = multiply(diff(ab, cd), wx);
				a[k + 3 * h] = multiply(diff(amb, cmd), wx);
			}
		} else {
			const Vec lo = reduce(f0, 2 * kP), hi = reduce(f1, 2 * kP);
			const Vec xc = Identity ? reduce(f2, 2 * kP) : multiply(f2, wx);
			const Vec xd = Identity ? reduce(f3, 2 * kP) : multiply(f3, wx);
			const Vec ac = reduce(add(lo, xc), 2 * kP), amc = reduce(diff(lo, xc), 2 * kP);
			const Vec bd = Identity ? reduce(add(hi, xd), 2 * kP) : multiply(add(hi, xd), wy);
			const Vec bmd = multiply(diff(hi, xd), wz);
			a[k] = add(ac, bd), a[k + h] = diff(ac, bd);
			a[k + 2 * h] = add(amc, bmd), a[k + 3 * h] = diff(amc, bmd);
		}
	}
}

inline void forward(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<false, false>(a, h, x, y);
}
inline void inverse(Vec* a, size_t h, const Word* x, const Word* y) {
	radix4<true, false>(a, h, x, y);
}
inline void forward_identity(Vec* a, size_t h, const Word* y) {
	radix4<false, true>(a, h, y, y);
}
inline void inverse_identity(Vec* a, size_t h, const Word* y) {
	radix4<true, true>(a, h, y, y);
}

}

namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr Word kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::forward(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, size_t count, const Word* table, size_t k) {
	for (size_t j = 0; j < count; ++j)
		blocked_ntt::kernels::inverse(a + 16 * j, 4, table + blocked_ntt::detail::slot(k + j), table + blocked_ntt::detail::slot(2 * (k + j)));
}

}

namespace blocked_poly {

inline constexpr Word kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr size_t footprint(size_t n) { return (n + 7) / 8 * 8 + 16; }
	Words take(size_t n) {
		if (size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

	bool zero_initialized() const { return storage_.get_deleter().region != nullptr; }

private:
	FpsWords storage_;
	Word *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add, blocked_ntt::detail::broadcast, blocked_ntt::detail::diff, blocked_ntt::detail::Factor, blocked_ntt::detail::kP, blocked_ntt::detail::kR, blocked_ntt::detail::reduce, blocked_ntt::detail::slot, blocked_ntt::detail::Vec;

inline constexpr auto kInverseScales = [] {
	std::array<std::array<Word, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(Word(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Vec load(const Word* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(Word* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

struct Factors {
	Vec w, q;
};

inline Vec times(Vec x, const Factors& w) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, w.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(w.q, 32));
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline Factors entries(const Word* table, size_t k) {
	const Word* e = table + slot(k);
	return {load(e), load(e + 8)};
}


inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const Word* table, size_t k) {
	const Word* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const Word* roots, size_t p) {
	const Word* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const Word* table, size_t g)
		: x(entry(table, g)), y(entry(table, 2 * g)), z(entry(table, 2 * g + 1)) {}
};

[[gnu::always_inline]] inline void forward_h1(Vec (&f)[4], const Group& w) {
	const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], w.x), d = times(f[3], w.x);
	const Vec ac = low(add(a, c)), amc = low_difference(a, c);
	const Vec bd = times(add(b, d), w.y), bmd = times(diff(b, d), w.z);
	f[0] = add(ac, bd), f[1] = diff(ac, bd), f[2] = add(amc, bmd), f[3] = diff(amc, bmd);
}

[[gnu::always_inline]] inline void inverse_h1(Vec (&f)[4], const Group& w) {
	const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
	const Vec amb = times(diff(f[0], f[1]), w.y), cmd = times(diff(f[2], f[3]), w.z);
	f[0] = low(add(ab, cd)), f[1] = low(add(amb, cmd));
	f[2] = times(diff(ab, cd), w.x), f[3] = times(diff(amb, cmd), w.x);
}

struct alignas(64) Window {
	Word word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const Word* roots,
												size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const Word* b) {
	Vec even = _mm256_setzero_si256(), odd = even;
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) {
		const Vec bi = broadcast(b[i]);
		const Vec lo = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 8 - i));
		const Vec hi = _mm256_loadu_si256(reinterpret_cast<const Vec*>(window.word + 9 - i));
		even = _mm256_add_epi64(even, _mm256_mul_epu32(lo, bi));
		odd = _mm256_add_epi64(odd, _mm256_mul_epu32(hi, bi));
	}
	return reduce(mint_detail::Ops::reduce8(even, odd), 2 * kP);
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Word* roots;

	void operator()(Word* a, size_t count, size_t first) const {
		for (size_t j = 0; j < count; ++j, a += 32) {
			const Group w(roots, first + j);
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, w);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
};

struct InverseBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word* inverse_roots;
	const Word* in;

	void operator()(Word* a, size_t count, size_t first) const {
		const Word* from = in + 32 * first;
		for (size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const Word* roots;
	const Word* inverse_roots;
	const Word* b;
	size_t origin = 0;

	[[gnu::always_inline]] void prepare(const Word* a, size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(Word* a, size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t - origin));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(Word* a, size_t count, size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const Word* a;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[4]) const {
		const Word* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(size_t g, Window (&window)[K][4]) const {
		for (size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t - terms[0].product.origin));
				for (size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t - terms[k].product.origin))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(Word* a, size_t count, size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const Word* b = product.b + 8 * (4 * (first + j) - product.origin);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const Word* roots, const Word* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(Word* a, size_t nv, size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr size_t kTile = 256;

	void tile(Word* a, size_t nv, size_t k) const {
		if constexpr (Bottom::kForward) {
			for (size_t h = nv / 4; h >= 16; h /= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (size_t h = 16; h < nv; h *= 4)
				for (size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(Word* a) { return reinterpret_cast<Vec*>(a); }

	void forward(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(Word* a, size_t h, size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const Word *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const Word* in, size_t size, size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool within(size_t lo, size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const Word* in, size_t shift, size_t end, size_t v) {
		alignas(32) Word x[8] = {};
		for (size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const Word* in_;
	size_t shift_, end_;
	size_t first_, count_;
	size_t first_touched_, touched_;
};

template <class Input>
[[gnu::always_inline]] inline void forward_top4_body(Input in, Vec* out, size_t h, const Word* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

template <class Input>
[[gnu::always_inline]] inline void forward_top2_body(Input in, Vec* out, size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

template <class Input>
[[gnu::noinline]] inline void forward_top4(Input in, Vec* out, size_t h, const Word* roots) {
	forward_top4_body(in, out, h, roots);
}
template <class Input>
[[gnu::noinline]] inline void forward_top2(Input in, Vec* out, size_t h) {
	forward_top2_body(in, out, h);
}

template <class Bottom, class Input>
void radix2_halves(const Recursion<Bottom>& recursion, const Input& in, Word* a, size_t nv) {
	const size_t h = nv / 2;
	if constexpr (Bottom::kForward) forward_top2(in, reinterpret_cast<Vec*>(a), h);
	recursion.visit(a, h, 0);
	recursion.visit(a + 8 * h, h, 1);
}

inline void inverse_top4(Vec* f, size_t h, Half output, const Word* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output != Half::kUpper) f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		if (output != Half::kLower) f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

inline void inverse_top2(Vec* f, size_t h, Half output, Word scale) {
	const Factor s(scale);
	for (size_t j = 0; j < h; ++j) {
		const Vec lo = f[j], hi = f[j + h];
		if (output != Half::kUpper) f[j] = reduce(times(add(lo, hi), s), kP);
		if (output != Half::kLower) f[j + h] = reduce(times(diff(lo, hi), s), kP);
	}
}

template <class Input, class Bottom>
[[gnu::always_inline]] inline void transform_with(Words a, const Input& in, const Bottom& bottom,
		const Word* roots, const Word* inverse_roots, Word scale, Half output) {
	const detail::Recursion recursion(roots, inverse_roots, bottom);
	const size_t nv = a.size() / 8;
	auto* v = reinterpret_cast<Vec*>(a.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward_top4(in, v, h, roots);
		for (size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
		if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots, Factor(scale));
	} else {
		const size_t h = nv / 2;
		radix2_halves(recursion, in, a.data(), nv);
		if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const size_t table = blocked_ntt::detail::table_words(lg_max), entries = (size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const Word* roots() const { return roots_; }
	const Word* inverse_roots() const { return inverse_roots_; }

	void forward(ConstWords in, size_t shift, Words out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(Words a) const { forward(a, 0, a); }

	void forward_upper(ConstWords in, size_t shift, Words out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(ConstWords in, Words out, Half output = Half::kBoth) const {
		const Word scale = inverse_scale(out.size(), false);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(Words a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(ConstWords in, size_t shift, Words out,
						ConstWords b, Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(Words a, ConstWords b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(ConstWords a, ConstWords b, Words out,
						Half output = Half::kBoth, Word c = 1) const {
		using namespace blocked_ntt::detail;
		const Word scale = multiply_mod(inverse_scale(out.size(), true), c);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		ConstWords a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, Words out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, Words out, Half output = Half::kBoth) const {
		const Word scale = inverse_scale(out.size(), true);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(ConstWords in, size_t shift, Words out,
						ConstWords b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(Words a, ConstWords b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(Words c, ConstWords a,
					ConstWords b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

protected:
	Word inverse_scale(size_t n, bool montgomery) const {
		check_length(n);
		return detail::kInverseScales[montgomery][std::countr_zero(n)];
	}

	void check_length(size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <size_t K>
	void inverse_products(std::span<const Pair> pairs, Words out, Half output) const {
		detail::InverseProductSumBottom<K> bottom{};
		for (size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		inverse_with(bottom, out, output);
	}

	static detail::Source source(ConstWords in, size_t shift, size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(Words a, const detail::Source& in, const Bottom& bottom, Word scale, Half output) const {
		check_length(a.size());
		detail::transform_with(a, in, bottom, roots_, inverse_roots_, scale, output);
	}

	template <class Combine>
	void products(Word* out, const Word* a, const Word* b, size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	Word *roots_, *inverse_roots_;
};

namespace detail {

inline Vec load_unaligned(const Word* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(Word* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec load_padded(ConstWords f, size_t at) {
	if (at + 8 <= f.size()) return load_unaligned(f.data() + at);
	alignas(32) Word x[8] = {};
	if (at < f.size()) std::copy(f.begin() + std::ptrdiff_t(at), f.end(), x);
	return load(x);
}

inline Vec first_lanes(size_t count) {
	return _mm256_cmpgt_epi32(broadcast(Word(std::min<size_t>(count, 8))),
							_mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
}
inline void store_first(Word* p, Vec x, size_t count) {
	if (count >= 8) return store_unaligned(p, x);
	_mm256_maskstore_epi32(reinterpret_cast<int*>(p), first_lanes(count), x);
}

inline Vec montgomery(Vec a, Vec b) { return mint_detail::Ops::mul8(a, b); }

class Indices {
public:
	explicit Indices(size_t k) {
		alignas(32) Word x[8];
		for (Word i = 0; i < 8; ++i) x[i] = Word((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr Word kStep = Word((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline Word scalar_inverse(Word x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(size_t first, Words out, const A& a, Word factor = 1) {
	constexpr size_t kLanes = 32;
	const size_t n = out.size();
	if (n == 0) return;
	const size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(Word(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(Word(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(Word(n)), lanes);
	};
	const auto masked = [&](Vec value, size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	const Vec left = montgomery(prefix[0], prefix[1]), right = montgomery(prefix[2], prefix[3]);
	alignas(32) Word total[8], inverse[8];
	store(total, canonical(montgomery(left, right)));
	Word running = 1;
	for (size_t l = 0; l < 8; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (size_t l = 8; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	const Vec inv_left = montgomery(load(inverse), right), inv_right = montgomery(load(inverse), left);
	Vec inv[4] = {montgomery(inv_left, prefix[1]), montgomery(inv_left, prefix[0]),
				  montgomery(inv_right, prefix[3]), montgomery(inv_right, prefix[2])};
	const auto put = [&](size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * size_t(v) < n) put(8 * v, inv[v]);
}

}

inline Word mint_scalar(Word x) { return x; }
inline unsigned mint_scalar(mint x) { return std::min(x.v, x.v - mint::mod()); }
inline void copy_coefficients(const mint *in, Words out) {
	for (size_t i = 0; i < out.size(); ++i) out[i] = mint_scalar(in[i]);
}
template <class T> inline detail::Vec coefficients(std::span<const T> f, size_t at) {
	detail::Vec value;
	std::memcpy(&value, f.data() + at, sizeof(value));
	if constexpr (std::is_same_v<T, mint>) return detail::reduce(value, detail::kP);
	else return value;
}

template <class T> inline void derivative_impl(std::span<const T> f, Words d) {
	using namespace detail;
	const size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(coefficients(f, i + 1), index.value())));
	for (size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(Word(i + 1), mint_scalar(f[i + 1])) : 0;
}

inline void derivative(ConstWords f, Words d) { derivative_impl(f, d); }

inline void divide_by_index(ConstWords a, size_t first, Words q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	detail::divide_by_index(first, q, [a](size_t at) __attribute__((target("avx2"))) { return detail::load_padded(a, at); });
}

namespace detail {

inline constexpr size_t kInverseBase = 32;

inline Word direct_product(const Word* a, const Word* b, size_t n) {
	std::uint64_t sums[4] = {};
	size_t j = 0;
	for (; j + 4 <= n; j += 4)
		for (size_t k = 0; k < 4; ++k) sums[k] += std::uint64_t(a[j + k]) * b[n - 1 - j - k];
	std::uint64_t sum = 0;
	for (auto x : sums) sum += x % kP;
	for (; j < n; ++j) sum += std::uint64_t(a[j]) * b[n - 1 - j];
	return Word(sum % kP);
}

inline void inverse_direct(ConstWords f, Words g, Word inv0 = 0) {
	using blocked_ntt::detail::multiply_mod;
	if (!inv0) inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (size_t i = 1; i < g.size(); ++i) {
		const size_t count = std::min(i, f.size() - 1);
		const Word sum = direct_product(f.data() + 1, g.data() + i - count, count);
		g[i] = multiply_mod(sum ? kP - sum : 0, inv0);
	}
}

}

inline int inverse_log(size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<size_t>(n, 2) - 1)));
}

inline size_t inverse_scratch(size_t n) { return 2 * Arena::footprint(size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, ConstWords f, ConstWords g,
						Words to, Words gt, Words work, Word scale = detail::kP - 1) {
	const size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, scale);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, ConstWords f, Words g,
					Words scratch) {
	const size_t n = g.size();
	size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

namespace detail {

inline constexpr size_t kLogBase = 64;

inline constexpr size_t kLogBlocks = 4;

inline void log_direct(ConstWords f, Words g) {
	using blocked_ntt::detail::multiply_mod;
	Word a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / Word(i), inv[kP % i]);
	g[0] = 0;
	for (size_t i = 1; i < g.size(); ++i) {
		const Word sum = direct_product(kg + 1, a + 1, i - 1);
		kg[i] = (multiply_mod(Word(i), a[i]) + kP - sum) % kP;
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline size_t log_block(size_t n) {
	return std::max<size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline size_t log_blocks(size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

template <class T> inline void subtract_derivative(std::span<const T> f, size_t first, size_t count,
									Word* out, size_t size) {
	const size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(coefficients(f, first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (size_t c = full; c < count; ++c) {
		const Word d = c < inside ? blocked_ntt::detail::multiply_mod(Word(first + 1 + c), mint_scalar(f[first + 1 + c])) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	Word* w3;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (size_t t = 0; t < 4; ++t) {
			const size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const Word* side;

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) {
				const size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + (leaf - 8 * term.product.origin)), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline size_t log_buffers(size_t n) { return std::max<size_t>(log_blocks(n) + 2, 3); }

inline Vec mint_load(std::span<const mint> f, size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(size_t lo, size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const mint> f, size_t at) { return mint_load(f, at); }
	std::span<const mint> f_;
	size_t full_, touched_;
};
inline void mint_forward(const Transform& t, std::span<const mint> f, Words out) {
	transform_with(out, MintSource(f), ForwardBottom{t.roots()}, t.roots(), t.inverse_roots(), 0, Half::kBoth);
}

struct NoLogTransform {
	void operator()(size_t, ConstWords) const {}
};

template <class Input, class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, Input f, size_t n,
												Words scratch, ConstWords prefix, const Sink& sink, const Transformed& transformed = {}) {
	const size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const Words ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, prefix, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (size_t s = 1; s < blocks; ++s) {
		const size_t from = std::min(f.size(), (s - 1) * k);
		if constexpr (std::is_same_v<Input, std::span<const mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (size_t j = 0; j < blocks; ++j) {
		const size_t first = j * k, count = std::min(k, n - 1 - first);
		const Words work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			derivative_impl(f.first(std::min(f.size(), count + 1)), work.first(count));
		} else {
			if (j == 1) {
				t.inverse_product(window(1), q0, work, Half::kUpper);
			} else if (blocks == 3) {
				const Transform::Pair pairs[2] = {{window(2), q0}, {window(1), work}};
				t.inverse_product_sum(pairs, work, Half::kUpper);
			} else if (j == 2) {
				t.inverse_with(ToeplitzBottom{t.roots(), t.inverse_roots(), window(1).data(), window(2).data(), q0.data(),
											work.data(), window(3).data()},
							work, Half::kUpper);
			} else {
				const InverseProductBottom product{{t.roots(), t.inverse_roots(), work.data()}, window(1).data()};
				t.inverse_with(SideProductBottom{product, window(3).data()}, work, Half::kUpper);
			}
			subtract_derivative(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, ConstWords(work.first(count)));
		if (j + 1 < blocks) {
			t.forward(work.first(k), 0, work);
			transformed(j, ConstWords(work));
		}
	}
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, ConstWords f, size_t n,
		Words scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative(t, f, n, scratch, f, sink, transformed);
}


inline int log_derivative_log(size_t n) { return std::countr_zero(2 * log_block(n)); }

inline size_t log_derivative_scratch(size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}


inline size_t log_scratch(size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_derivative_scratch(n);
}

inline void log(const Transform& t, ConstWords f, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

inline void native_log(const mint *in, size_t input, std::vector<mint> &out, size_t n, bool integrate) {
	prepare_fps_output(out, n);
	out.emplace_back(0);
	const size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	const bool direct = n >= 262144;
	const auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;
	Arena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(prefix_size), scratch = arena.take(log_scratch(n));
	copy_coefficients(in, f);
	const auto sink = [&out, integrate, n](size_t first, ConstWords q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<Word *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
		}
		const auto *values = integrate ? q.data() + detail::log_block(n) : q.data();
		if (integrate) append_fps_words<true>(out, values, q.size());
		else append_fps_words<false>(out, values, q.size());
	};
	if (direct) detail::log_derivative(t, std::span<const mint>(in, c), n, scratch, f, sink);
	else detail::log_derivative(t, f, n, scratch, sink);
}
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_exp_blocked.hpp"



#line 5 "components/fps_exp_blocked.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_detail {
namespace blocked_poly {
namespace detail {
inline constexpr size_t kExpBase = 64;

inline constexpr auto kExpInverse = [] {
	std::array<Word, kExpBase> inv{0, 1};
	for (size_t i = 2; i < inv.size(); ++i) inv[i] = blocked_ntt::detail::multiply_mod(kP - kP / Word(i), inv[kP % i]);
	return inv;
}();

inline void exp_direct(ConstWords q, Words g) {
	using blocked_ntt::detail::multiply_mod;
	g[0] = 1;
	for (size_t i = 1; i < g.size(); ++i) {
		const Word sum = direct_product(q.data() + 1, g.data(), i);
		g[i] = multiply_mod(sum, kExpInverse[i]);
	}
}

inline void multiply_by_index(ConstWords a, size_t first, Words out) {
	const size_t n = out.size(), full = n / 8 * 8;
	Indices index(first);
	for (size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(out.data() + i, canonical(montgomery(load_unaligned(a.data() + i), index.value())));
	for (size_t i = full; i < n; ++i) out[i] = blocked_ntt::detail::multiply_mod(Word((first + i) % kP), a[i]);
}

inline size_t exp_length(size_t n) {
	return size_t(1) << std::max(Transform::kMinLog + 1, int(std::bit_width(std::max<size_t>(n, 2) - 1)));
}

}

inline int exp_log(size_t n) { return std::countr_zero(detail::exp_length(n)) - 1; }

namespace detail {

struct QTransforms {
	ConstWords low, block2;
};
struct NoQTransforms {};

template <class Bottom>
void product_to(const Transform& t, Words out, const Source& in, const Bottom& bottom,
				Word scale, Half output, Words to) {
	const size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	const Recursion recursion(t.roots(), t.inverse_roots(), bottom);
	const Factor s(scale);
	const auto put = [to, &s](size_t j, Vec x) __attribute__((target("avx2"))) {
		const Vec y = reduce(times(x, s), kP);
		if (8 * j < to.size()) store_first(to.data() + 8 * j, y, to.size() - 8 * j);
	};
	const bool lower = output == Half::kLower;
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward_top4(in, v, h, t.roots());
		for (size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec ab = low(add(v[j], v[j + h])), cd = low(add(v[j + 2 * h], v[j + 3 * h]));
			const Vec amb = low(diff(v[j], v[j + h])), cmd = times(diff(v[j + 2 * h], v[j + 3 * h]), z);
			put(j, lower ? add(ab, cd) : diff(ab, cd));
			put(j + h, lower ? add(amb, cmd) : diff(amb, cmd));
		}
	} else {
		const size_t h = nv / 2;
		radix2_halves(recursion, in, out.data(), nv);
		for (size_t j = 0; j < h; ++j) put(j, lower ? add(v[j], v[j + h]) : diff(v[j], v[j + h]));
	}
}

template <class Given>
[[gnu::always_inline]] inline void exp_last_step(const Transform& t, Words lo, Words g,
												 size_t m, bool first, Words gt,
												 ConstWords ht, Words w,
												 Words work, const Given& given) {
	const size_t n = g.size(), half = m / 2, rest = n - m, count = std::min(rest, half);
	const bool upper = rest > half;
	const Words g0t = gt.first(m), glt = gt.subspan(m, m), own = w.first(m), r = w.subspan(m, m);
	ConstWords qt = own, q2t = lo;
	bool given_low = false, given_block2 = false;
	if constexpr (std::is_same_v<Given, QTransforms>) {
		given_low = !given.low.empty(), given_block2 = !given.block2.empty();
		if (given_low) qt = given.low;
		if (given_block2) q2t = given.block2;
	}
	const auto q = [g](size_t i) __attribute__((target("avx2"))) { return load_padded(g, i); };
	if (first) t.forward(g.first(half), 0, g0t);
	t.forward(g.first(m), 0, glt);
	if (given_low) {
		t.inverse_product(qt, glt, r, upper ? Half::kBoth : Half::kLower);
	} else if (upper) {
		t.forward(lo, 0, own);
		t.inverse_product(own, glt, r);
	} else {
		t.cyclic_product(lo, 0, r, glt, Half::kLower);
	}

	t.forward(r.first(half), 0, work);
	t.inverse_product(ht, work, work, Half::kLower);
	detail::divide_by_index(m, r.first(count), [&](size_t i) __attribute__((target("avx2"))) {
		const Vec x = reduce(add(load(work.data() + i), q(m + i)), kP);
		return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
	});
	if (upper && !given_block2) t.forward(g.subspan(m, half), 0, lo);
	t.forward(r.first(count), 0, work);
	const Word scale = kInverseScales[1][std::countr_zero(m)];
	const Source none(nullptr, 0, 0);
	product_to(t, work, none, InverseProductBottom{{t.roots(), t.inverse_roots(), work.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m, count));
	if (!upper) return;

	t.forward(g.subspan(m, half), 0, work);
	const Transform::Pair pairs[] = {{work, qt}, {q2t, glt}};
	t.inverse_product_sum(pairs, work, Half::kUpper);
	Indices index(half);
	for (size_t i = half; i < m; i += 8, index.next()) {
		const Vec xg = montgomery(load_unaligned(g.data() + i), index.value());
		const Vec sum = add(load(r.data() + i), load(work.data() + i));
		store(r.data() + i, canonical(_mm256_sub_epi32(add(sum, broadcast(2 * kP)), xg)));
	}
	t.forward(r.subspan(half, half), 0, own);
	t.inverse_product(ht, own, own, Half::kLower);
	detail::divide_by_index(m + half, r.first(rest - half),
							[&](size_t i) __attribute__((target("avx2"))) { return add(load(own.data() + i), q(m + half + i)); });
	t.forward(r.first(rest - half), 0, own);
	product_to(t, own, none, InverseProductBottom{{t.roots(), t.inverse_roots(), own.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m + half, rest - half));
}

template <class Given = NoQTransforms>
[[gnu::always_inline]] inline void exp_newton(const Transform& t, Words lo, Words g,
											  Words scratch, const Given& given = {}) {
	const size_t n = g.size(), len = 2 * lo.size();
	const auto take = [&scratch](size_t words) __attribute__((target("avx2"))) {
		const Words s = scratch.first(words);
		scratch = scratch.subspan(Arena::footprint(words));
		return s;
	};
	const Words h = take(len / 2), gt = take(len), ht = take(len / 2), w = take(len);
	size_t m = kExpBase;
	inverse_direct(g, h.first(m / 2));
	t.forward(h.first(m / 2), 0, ht.first(m));
	for (; 2 * m < n; m *= 2) {
		const size_t half = m / 2;
		const Words g_low = gt.first(m), h_low = ht.first(m), w_low = w.first(m);
		t.forward(g.first(m), 0, g_low);

		t.inverse_product(g_low, h_low, w_low, Half::kUpper);
		const Word negated = blocked_ntt::detail::multiply_mod(kInverseScales[1][std::countr_zero(m)], kP - 1);
		product_to(t, w_low, Source(w_low.data() + half, half, half), ProductBottom{t.roots(), t.inverse_roots(), h_low.data()},
				   negated, Half::kUpper, h.subspan(half, half));

		t.forward_product(lo.first(m), 0, w_low, g_low);
		t.inverse(w_low, gt.subspan(m, m));

		t.forward(h.first(m), 0, ht.first(2 * m));
		t.forward_upper(gt.subspan(m, m), 0, w.subspan(m, m));
		t.inverse_product(w.first(2 * m), ht.first(2 * m), w.first(2 * m), Half::kLower);

		detail::divide_by_index(m, w.subspan(m, m), [&](size_t i) __attribute__((target("avx2"))) {
			const Vec x = reduce(add(load(w.data() + i), load(lo.data() + m + i)), kP);
			return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
		});
		t.forward_upper(g.first(m), 0, gt.subspan(m, m));
		product_to(t, w.first(2 * m), Source(w.data() + m, m, m), ProductBottom{t.roots(), t.inverse_roots(), gt.data()},
				   kInverseScales[1][std::countr_zero(2 * m)], Half::kUpper, g.subspan(m, m));
	}
	exp_last_step(t, lo, g, m, m == kExpBase, gt, ht, w, h, given);
}

inline size_t exp_newton_scratch(size_t n) {
	const size_t len = exp_length(n);
	return 2 * Arena::footprint(len) + 2 * Arena::footprint(len / 2);
}

}

inline size_t exp_scratch(size_t n) {
	return Arena::footprint(detail::exp_length(n) / 2) + detail::exp_newton_scratch(n);
}

inline void exp(const Transform& t, ConstWords f, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size(), m = exp_length(n) / 2, size = std::min(f.size(), n), below = std::min(size, m);
	const Words lo = scratch.first(m);
	multiply_by_index(f.first(below), 0, lo.first(below));
	std::fill(lo.begin() + std::ptrdiff_t(below), lo.end(), 0);
	if (size > m) multiply_by_index(f.subspan(m, size - m), m, g.subspan(m, size - m));
	if (n > std::max(size, m)) std::fill(g.begin() + std::ptrdiff_t(std::max(size, m)), g.end(), 0);
	exp_direct(lo, g.first(std::min(n, kExpBase)));
	if (n > kExpBase) exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
}

}

namespace blocked_poly {

namespace detail {

template <bool kSum>
void times_x(const Word* roots, const Word* a, const Word* b, Word e,
			 Word alpha, Word beta, Words out) {
	const size_t leaves = out.size() / 8;
	const Factor scale(e);
	const Vec rotate = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), odd = _mm256_setr_epi32(0, -1, 0, -1, 0, -1, 0, -1);
	const Word constants[2] = {(alpha + beta) % kP, (alpha + kP - beta) % kP};
	for (size_t p = 0; p < leaves; p += 8) {
		const bool upper = p >= leaves / 2;

		const Factors r = entries(roots, p / 16 * 8);
		const Vec pick = p % 16 ? _mm256_setr_epi32(4, 4, 5, 5, 6, 6, 7, 7) : _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3);
		const Vec rw = _mm256_permutevar8x32_epi32(r.w, pick), rq = _mm256_permutevar8x32_epi32(r.q, pick);
		const Factors w{_mm256_blend_epi32(rw, _mm256_sub_epi32(broadcast(kP), rw), 0xAA), _mm256_xor_si256(rq, odd)};
		Vec y[8];
#pragma GCC unroll 8
		for (size_t i = 0; i < 8; ++i) {
			Vec c = load(a + 8 * (p + i));
			if constexpr (kSum) c = upper ? difference(c, load(b + 8 * (p + i))) : reduce(add(c, load(b + 8 * (p + i))), kP);
			y[i] = reduce(times(c, scale), kP);
		}

		const Vec t01 = _mm256_unpackhi_epi32(y[0], y[1]), t23 = _mm256_unpackhi_epi32(y[2], y[3]);
		const Vec t45 = _mm256_unpackhi_epi32(y[4], y[5]), t67 = _mm256_unpackhi_epi32(y[6], y[7]);
		const Vec u03 = _mm256_unpackhi_epi64(t01, t23), u47 = _mm256_unpackhi_epi64(t45, t67);
		const Vec z = canonical(add(times(_mm256_permute2x128_si256(u03, u47, 0x31), w), broadcast(constants[upper])));
		const Vec zh = _mm256_permute2x128_si256(z, z, 0x11);
		const Vec lane0[8] = {z,  _mm256_srli_si256(z, 4),  _mm256_srli_si256(z, 8),  _mm256_srli_si256(z, 12),
							  zh, _mm256_srli_si256(zh, 4), _mm256_srli_si256(zh, 8), _mm256_srli_si256(zh, 12)};
#pragma GCC unroll 8
		for (size_t i = 0; i < 8; ++i)
			store(out.data() + 8 * (p + i), _mm256_blend_epi32(_mm256_permutevar8x32_epi32(y[i], rotate), lane0[i], 1));
	}
}

}

inline int power_log(size_t n) { return std::max(detail::log_derivative_log(n), exp_log(n)); }

inline size_t power_scratch(size_t n) {
	return 3 * Arena::footprint(detail::exp_length(n) / 2) +
		   std::max(detail::log_derivative_scratch(n), detail::exp_newton_scratch(n));
}

inline void power(const Transform& t, ConstWords f, Word e, Word c,
				  Words g, Words scratch) {
	using namespace detail;
	const size_t n = g.size(), m = exp_length(n) / 2, k = m / 2, stride = Arena::footprint(m);
	if (f.empty() || f[0] == 0) std::abort();
	const Words lo = scratch.first(m), rest = scratch.subspan(3 * stride);
	const auto q = [&](size_t i) __attribute__((target("avx2"))) { return i < m ? lo.data() + i : g.data() + i; };
	const Factor factor(e);

	const bool reuse = m >= 128 && log_block(n) == k && log_blocks(n) >= 3;
	const bool block2 = reuse && log_blocks(n) == 4 && n - m > k;
	const Words qt = scratch.subspan(stride, reuse ? m : 0);
	const Words q2t = scratch.subspan(2 * stride, block2 ? m : 0);
	const Word* d0t = nullptr;
	lo[0] = 0;
	if (n >= 2)
		log_derivative(
			t, f, n, rest,
			[&](size_t first, ConstWords d) __attribute__((target("avx2"))) {
				size_t i = 0;
				for (; i + 8 <= d.size(); i += 8) {
					const size_t at = first + 1 + i;
					const Vec x = reduce(times(load(d.data() + i), factor), kP);
					if (at + 8 <= m || at >= m) {
						store_unaligned(q(at), x);
					} else {
						alignas(32) Word words[8];
						store(words, x);
						for (size_t j = 0; j < 8; ++j) *q(at + j) = words[j];
					}
				}
				for (; i < d.size(); ++i) *q(first + 1 + i) = blocked_ntt::detail::multiply_mod(d[i], e);
			},
			[&](size_t j, ConstWords dt) __attribute__((target("avx2"))) {
				if (j == 0) d0t = dt.data();
				if (j == 1 && reuse) times_x<true>(t.roots(), d0t, dt.data(), e, kP - *q(m), 0, qt);
				if (j == 2 && block2) times_x<false>(t.roots(), dt.data(), nullptr, e, *q(m), kP - *q(m + k), q2t);
			});
	const Words start = g.first(std::min(n, kExpBase));
	exp_direct(lo, start);
	for (Word& x : start) x = blocked_ntt::detail::multiply_mod(x, c);
	if (n > kExpBase) exp_newton(t, lo, g, rest, QTransforms{qt, q2t});
}

namespace detail {
template <bool differentiated>
[[gnu::noinline]] mint exp_tail_coefficient(const mint* in, size_t input, const mint* g, size_t n) {
	const size_t count = std::min(n, input ? input - 1 : 0), full = count / 8 * 8;
	const Vec reversed = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	Vec total = _mm256_setzero_si256();
	Indices index(1);
	size_t j = 0;
	for (; j < full;) {
		Vec even = _mm256_setzero_si256(), odd = even;
		const size_t end = std::min(full, j + 64);
		for (; j < end; j += 8, index.next()) {
			Vec a, b;
			std::memcpy(&a, in + j + 1, sizeof(a));
			std::memcpy(&b, g + n - j - 8, sizeof(b));
			a = reduce(a, kP);
			if constexpr (!differentiated) a = canonical(montgomery(a, index.value()));
			b = _mm256_permutevar8x32_epi32(reduce(b, kP), reversed);
			even = _mm256_add_epi64(even, _mm256_mul_epu32(a, b));
			odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32)));
		}
		const Vec value = canonical(mint_detail::Ops::reduce8(even, odd));
		total = low(add(total, value));
	}
	total = low(add(total, _mm256_permute2x128_si256(total, total, 1)));
	total = low(add(total, _mm256_shuffle_epi32(total, 0x4E)));
	total = low(add(total, _mm256_shuffle_epi32(total, 0xB1)));
	mint result = mint::word(Word(_mm_cvtsi128_si32(_mm256_castsi256_si128(total))));
	for (; j < count; ++j) {
		mint value = in[j + 1] * g[n - j - 1];
		if constexpr (!differentiated) value *= mint(j + 1);
		result += value;
	}
	return result / mint(n);
}

template <bool differentiated>
[[gnu::noinline]] void import_exp_input(const mint* in, size_t first, size_t count, Word* to) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	size_t i = 0;
	const Factor undo(blocked_ntt::detail::power(kR, kP - 2));
	auto index = add(broadcast(Word(first)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	for (; i + 8 <= count; i += 8, index = add(index, broadcast(8))) {
		Vec value;
		std::memcpy(&value, in + first + i, sizeof(value));
		if constexpr (differentiated) store_unaligned(to + i, reduce(times(value, undo), kP));
		else store_unaligned(to + i, reduce(montgomery(value, index), kP));
	}
	for (; i < count; ++i) {
		if constexpr (differentiated) to[i] = in[first + i].val();
		else to[i] = blocked_ntt::detail::multiply_mod(in[first + i].val(), first + i);
	}
}
}

inline void native_exp_regular(const mint *in, size_t input, std::vector<mint> &out, size_t n, bool differentiated) {
	const size_t m = detail::exp_length(n) / 2, size = std::min(input, n), below = std::min(size, m);
	const int lg = exp_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(exp_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(exp_scratch(n));
	auto lo = scratch.first(m);
	const auto import = [&](size_t first, size_t count, Word* to) {
		if (differentiated) detail::import_exp_input<true>(in, first, count, to);
		else detail::import_exp_input<false>(in, first, count, to);
	};
	import(0, below, lo.data());
	std::fill(lo.begin() + below, lo.end(), 0u);
	if (size > m) import(m, size - m, g.data() + m);
	if (n > std::max(size, m)) std::fill(g.begin() + std::max(size, m), g.end(), 0u);
	detail::exp_direct(lo, g.first(std::min(n, detail::kExpBase)));
	for (auto &x : g.first(std::min(n, detail::kExpBase))) x = blocked_ntt::detail::multiply_mod(x, detail::kR);
	if (n > detail::kExpBase) detail::exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
	append_fps_words<true>(out, g.data(), n);
}

inline void native_exp(const mint *in, size_t input, std::vector<mint> &out, size_t n, bool differentiated = false) {
	const size_t prefix = std::bit_floor(n), tail = n - prefix;
	if (prefix >= 4096 && tail && tail <= 16) {
		const size_t offset = out.size();
		prepare_fps_output(out, offset + n);
		native_exp_regular(in, input, out, prefix, differentiated);
		for (size_t at = prefix; at < n; ++at) {
			const mint value = differentiated ? detail::exp_tail_coefficient<true>(in, input, out.data() + offset, at)
											 : detail::exp_tail_coefficient<false>(in, input, out.data() + offset, at);
			out.push_back(value);
		}
		return;
	}
	native_exp_regular(in, input, out, n, differentiated);
}

inline void native_power(const mint *in, size_t input, std::vector<mint> &out, size_t n, Word e, mint c, size_t zeros = 0) {
	const int lg = power_log(n);
	const size_t size = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(power_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(power_scratch(n));
	copy_coefficients(in, g.first(size));
	power(t, g.first(size), e, std::min(c.v, c.v - mint::mod()), g, scratch);
	if (zeros) {
		prepare_fps_output(out, out.size() + zeros + n);
		out.resize(out.size() + zeros);
	}
	append_fps_words<true>(out, g.data(), n);
}
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_inverse_blocked.hpp"



#line 5 "components/fps_inverse_blocked.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {
inline void append_scaled(Words a, std::vector<mint> &out, Word scale) {
	const detail::Factor factor(scale);
	if (a.size() > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + a.size());
	alignas(32) Word values[64];
	for (size_t i = 0; i < a.size(); i += 64) {
		const auto count = std::min<size_t>(64, a.size() - i);
		size_t j = 0;
		for (; j + 8 <= count; j += 8)
			detail::store(values + j, detail::reduce(detail::times(detail::load_unaligned(a.data() + i + j), factor), mint::mod()));
		for (; j < count; ++j) values[j] = blocked_ntt::detail::multiply_mod(a[i + j], scale);
		append_fps_words<true>(out, values, count);
	}
}

inline void native_inverse(const mint *in, size_t input, std::vector<mint> &out, size_t n) {
	const int lg = inverse_log(n);
	const size_t len = size_t(1) << lg, k = len / 2;
	const size_t c = std::min(input, n), scratch_words = std::max(len, inverse_scratch(k));
	Arena arena(Transform::words(lg) + Arena::footprint(len) + Arena::footprint(scratch_words) + Arena::footprint(k));
	const Transform t(arena, lg);
	auto a = arena.take(len), scratch = arena.take(scratch_words), g = arena.take(k);
	copy_coefficients(in, a.first(c));
	const ConstWords f = a.first(c);
	const size_t target = std::min(n, k), base = std::min(target, detail::kInverseBase);
	detail::inverse_direct(f, g.first(base));
	for (size_t i = 0; i < base; ++i) g[i] = blocked_ntt::detail::multiply_mod(g[i], 932051910);
	for (size_t count = base; count < target; count *= 2) {
		const size_t size = 2 * count;
		inverse_step(t, f, g.first(count), g.subspan(count, std::min(count, target - count)), scratch.first(size),
				scratch.subspan(Arena::footprint(size), size), 53925088);
	}
	if (n <= k) return append_fps_words<true>(out, g.data(), n);
	inverse_step(t, f, g, a.subspan(k, n - k), scratch.first(len), a, 53925088);
	prepare_fps_output(out, n);
	append_fps_words<true>(out, g.data(), k);
	append_fps_words<true>(out, a.data() + k, n - k);
}
}

#pragma GCC pop_options
#endif

#line 1 "components/fps_sqrt_blocked.hpp"


#line 4 "components/fps_sqrt_blocked.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

namespace detail {

inline constexpr size_t kSqrtBase = 64;

inline void sqrt_direct(ConstWords f, Word c, Words g, Word inverse = 0) {
	using blocked_ntt::detail::multiply_mod;
	if (!inverse) inverse = blocked_ntt::detail::power(multiply_mod(2, c), kP - 2);
	g[0] = c;
	for (size_t i = 1; i < g.size(); ++i) {
		const size_t half = (i - 1) / 2;
		Word sum = 2 * direct_product(g.data() + 1, g.data() + i - half, half);
		if (!(i & 1)) sum += multiply_mod(g[i / 2], g[i / 2]);
		g[i] = multiply_mod(((i < f.size() ? f[i] : 0) + 3 * kP - sum) % kP, inverse);
	}
}

inline Vec load_first(const Word* p, size_t count) {
	if (count >= 8) return load_unaligned(p);
	return _mm256_maskload_epi32(reinterpret_cast<const int*>(p), first_lanes(count));
}

[[gnu::always_inline]] inline Vec leaf_square(const Window& window) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2;
	asm(R"(
vpbroadcastd 32(%[w]), %[bi]
	vpmuludq 36(%[w]), %[bi], %[odd]
	vpmuludq 32(%[w]), %[bi], %[even]
	vpbroadcastd 36(%[w]), %[bi]
	vpmuludq 28(%[w]), %[bi], %[t0]
	vpbroadcastd 40(%[w]), %[bi]
	vpmuludq 28(%[w]), %[bi], %[t1]
	vpmuludq 24(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 44(%[w]), %[bi]
	vpmuludq 20(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpbroadcastd 48(%[w]), %[bi]
	vpmuludq 20(%[w]), %[bi], %[t1]
	vpmuludq 16(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 52(%[w]), %[bi]
	vpmuludq 12(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpbroadcastd 56(%[w]), %[bi]
	vpmuludq 12(%[w]), %[bi], %[t1]
	vpmuludq 8(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 60(%[w]), %[bi]
	vpmuludq 4(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpaddq %[t0], %[even], %[even]
	vpaddq %[odd], %[odd], %[odd]
	vpmuludq %[ni], %[even], %[t0]
	vpmuludq %[ni], %[odd], %[t1]
	vpmuludq %[p], %[t0], %[t0]
	vpmuludq %[p], %[t1], %[t1]
	vpaddq %[t0], %[even], %[even]
	vpaddq %[t1], %[odd], %[odd]
	vpsrlq $32, %[even], %[even]
	vpblendd $0xaa, %[odd], %[even], %[even]
	vpsubd %[p2], %[even], %[t0]
	vpminud %[t0], %[even], %[even]
)"
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2)
		: [w] "r"(window.word), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const Word(*)[17]>(window.word)));
	return even;
}

struct InverseSquareBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word* roots;
	const Word* inverse_roots;
	const Word* a;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[4]) const {
		const Word* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			Vec f[4];
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) f[t] = leaf_square(window[j & 1][t]);
			inverse_h1(f, Group(inverse_roots, first + j));
			for (int t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline bool radix4(size_t nv) { return std::countr_zero(nv) % 2 == 0; }

template <class Bottom>
void subtrees(const Recursion<Bottom>& recursion, Word* a, size_t nv) {
	if (!radix4(nv)) {
		recursion.visit(a, nv / 2, 0);
		return recursion.visit(a + 4 * nv, nv / 2, 1);
	}
	for (size_t q = 0; q < 4; ++q) recursion.visit(a + 2 * q * nv, nv / 4, q);
}

template <class Bottom>
void from_lower_source(const Transform& t, const Recursion<Bottom>& recursion, Word* a, size_t nv,
					bool written = true) {
	const Source in(a, 4 * nv, 0);
	if (!radix4(nv)) return radix2_halves(recursion, in, a, nv);
	if (!written) forward_top4(in, reinterpret_cast<Vec*>(a), nv / 4, t.roots());
	subtrees(recursion, a, nv);
}

inline void inverse_top(const Transform& t, Vec* v, size_t nv, Half output, Word scale) {
	if (radix4(nv)) return inverse_top4(v, nv / 4, output, t.inverse_roots(), Factor(scale));
	inverse_top2(v, nv / 2, output, scale);
}

template <Half kOutput, class Visit>
[[gnu::always_inline]] inline void inverse_columns(const Transform& t, Vec* v, size_t nv, Word scale,
												Visit visit) {
	constexpr bool kLower = kOutput != Half::kUpper, kUpper = kOutput != Half::kLower;
	const Factor s(scale);
	if (radix4(nv)) {
		const size_t h = nv / 4;
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec p0 = v[j], p1 = v[j + h], p2 = v[j + 2 * h], p3 = v[j + 3 * h];
			const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
			const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
			Vec lo[2] = {}, hi[2] = {};
			if constexpr (kLower) lo[0] = times(add(ab, cd), s), lo[1] = times(add(amb, cmd), s);
			if constexpr (kUpper) hi[0] = times(diff(ab, cd), s), hi[1] = times(diff(amb, cmd), s);
			visit(std::true_type{}, j, h, lo, hi);
		}
		return;
	}
	const size_t h = nv / 2;
	for (size_t j = 0; j < h; ++j) {
		Vec lo[1] = {}, hi[1] = {};
		if constexpr (kLower) lo[0] = times(add(v[j], v[j + h]), s);
		if constexpr (kUpper) hi[0] = times(diff(v[j], v[j + h]), s);
		visit(std::false_type{}, j, h, lo, hi);
	}
}

[[gnu::always_inline]] inline void forward_lower4(Vec* v, size_t j, size_t h, Vec a, Vec b, const Factor& z) {
	const Vec zb = times(b, z);
	v[j] = add(a, b), v[j + h] = diff(a, b), v[j + 2 * h] = add(a, zb), v[j + 3 * h] = diff(a, zb);
}

[[gnu::always_inline]] inline void forward_upper4(Vec* v, size_t j, size_t h, Vec c, Vec d, const Factor& z) {
	const Vec mc = _mm256_sub_epi32(broadcast(kP), c), zmd = times(_mm256_sub_epi32(broadcast(2 * kP), d), z);
	v[j] = add(c, d), v[j + h] = diff(c, d), v[j + 2 * h] = add(mc, zmd), v[j + 3 * h] = diff(mc, zmd);
}

inline Factor top_root(const Transform& t) { return Factor(t.roots()[1], t.roots()[9]); }

inline Vec* vectors(Words a) { return reinterpret_cast<Vec*>(a.data()); }

inline Word product_scale(size_t n) { return kInverseScales[1][std::countr_zero(n)]; }

inline Vec half_residual(ConstWords f, size_t m, size_t at, Vec s) {
	const Factor half((kP + 1) / 2);
	const Vec fs = add(load_padded(f, at), load_padded(f, m + at));
	return reduce(times(_mm256_sub_epi32(add(s, broadcast(2 * kP)), fs), half), kP);
}

inline void half_residual(ConstWords f, size_t m, size_t first, const Word* s,
						Word* out, size_t count) {
	for (size_t i = 0; i < count; i += 8) store_first(out + i, half_residual(f, m, first + i, load(s + i)), count - i);
}

inline void sqrt_step(const Transform& t, ConstWords f, size_t m, Word* g,
					Word* h, Words b, Words ht) {
	const size_t half = m / 2, nv = m / 8;
	const Words gt = b.first(m), w = b.subspan(m, m), work = b.first(2 * m);
	const Factor z = top_root(t);
	const ProductBottom with_h{t.roots(), t.inverse_roots(), ht.data()};
	const Word scale = product_scale(m), scale2 = product_scale(2 * m);
	t.forward(ConstWords(g, m), 0, gt);
	Vec* v = vectors(w);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseProductBottom{with_h, gt.data()}), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* e) {
		if constexpr (radix4) forward_upper4(v, j, q, reduce(e[0], kP), e[1], z);
		else v[j] = e[0], v[j + q] = _mm256_sub_epi32(broadcast(2 * kP), e[0]);
	});
	subtrees(Recursion(t.roots(), t.inverse_roots(), with_h), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* x) {
		store_unaligned(h + half + 8 * j, reduce(x[0], kP));
		if constexpr (radix4) store_unaligned(h + half + 8 * (j + q), reduce(x[1], kP));
	});
	t.forward(ConstWords(h, m), 0, ht);

	v = vectors(gt);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}), gt.data(),
			nv);
	inverse_columns<Half::kBoth>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* lo, const Vec* hi) {
		if constexpr (radix4) {
			v[j] = half_residual(f, m, 8 * j, lo[0]), v[j + q] = half_residual(f, m, 8 * (j + q), lo[1]);
			v[j + 2 * q] = half_residual(f, m, half + 8 * j, hi[0]);
			v[j + 3 * q] = half_residual(f, m, half + 8 * (j + q), hi[1]);
		} else {
			forward_lower4(v, j, q, half_residual(f, m, 8 * j, lo[0]), half_residual(f, m, half + 8 * j, hi[0]), z);
		}
	});
	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_h), work.data(), 2 * nv);
	inverse_columns<Half::kLower>(t, vectors(work), 2 * nv, scale2,
								[&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
									store_unaligned(g + m + 8 * j, reduce(d[0], kP));
									if constexpr (radix4) store_unaligned(g + m + 8 * (j + q), reduce(d[1], kP));
								});
}

}

inline int sqrt_log(size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<size_t>(n, 2) - 1)) - 1);
}

inline size_t sqrt_scratch(size_t n) {
	if (n <= detail::kSqrtBase) return 0;
	return 3 * Arena::footprint(size_t(1) << sqrt_log(n));
}

inline void sqrt_steps(const Transform& t, ConstWords f, Word c, Words a,
					Words b, Words ht) {
	using namespace detail;
	const size_t len = a.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0] || len < kSqrtBase) std::abort();
	const Word inv0 = blocked_ntt::detail::power(c, kP - 2);
	sqrt_direct(f, c, a.first(kSqrtBase), blocked_ntt::detail::multiply_mod(inv0, (kP + 1) / 2));
	Word* const h = a.data() + len / 2;
	alignas(32) Word h0[kSqrtBase / 2];
	inverse_direct(a.first(kSqrtBase / 2), h0, inv0);
	for (Word& x : h0) x = x ? kP - x : 0;
	t.forward(h0, 0, ht.first(kSqrtBase));
	if (len > kSqrtBase) std::copy_n(h0, kSqrtBase / 2, h);
	for (size_t m = kSqrtBase; m < len; m *= 2) sqrt_step(t, f, m, a.data(), h, b.first(2 * m), ht.first(2 * m));
}

inline void sqrt_last_step(const Transform& t, ConstWords f, ConstWords gt,
						ConstWords ht, Words work, Words hi) {
	using namespace detail;
	const size_t m = gt.size(), half = m / 2, rest = hi.size(), nv = m / 8;
	if (rest == 0 || rest > m) std::abort();
	const Word scale = product_scale(m);
	const Factor z = top_root(t);
	const ProductBottom with_g{t.roots(), t.inverse_roots(), gt.data()}, with_h{t.roots(), t.inverse_roots(), ht.data()};
	const Recursion product_h(t.roots(), t.inverse_roots(), with_h);
	Vec* v = vectors(work);

	const auto source = [&](auto radix4, size_t j, size_t q, auto x) {
		if constexpr (radix4) forward_lower4(v, j, q, x(0, j), x(1, j + q), z);
		else v[j] = x(0, j);
	};

	const auto each = [](auto radix4, size_t j, size_t q, auto put) {
		put(0, j);
		if constexpr (radix4) put(1, j + q);
	};
	const auto put = [&](Word* out, size_t count, size_t p, Vec x) __attribute__((target("avx2"))) {
		if (8 * p < count) store_first(out + 8 * p, x, count - 8 * p);
	};

	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}),
			work.data(), nv);
	const bool split = rest > half;
	inverse_top(t, v, nv, split ? Half::kBoth : Half::kLower, scale);
	half_residual(f, m, 0, work.data(), work.data(), std::min(rest, half));
	const size_t count = split ? rest - half : 0;
	Word* const upper = hi.data() + (rest - count);
	half_residual(f, m, half, work.data() + half, upper, count);
	from_lower_source(t, product_h, work.data(), nv, false);
	if (!split) {
		inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
			each(radix4, j, q, [&](int k, size_t p) { put(hi.data(), rest, p, reduce(d[k], kP)); });
		});
		return;
	}

	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
		source(radix4, j, q, [&](int k, size_t p) {
			const Vec d0 = reduce(d[k], kP);
			store_unaligned(hi.data() + 8 * p, d0);
			return d0;
		});
	});

	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_g), work.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* x) {
		source(radix4, j, q, [&](int k, size_t p) {
			const size_t at = 8 * p;
			return canonical(add(x[k], load_first(upper + std::min(at, count), at < count ? count - at : 0)));
		});
	});
	from_lower_source(t, product_h, work.data(), nv);
	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
		each(radix4, j, q, [&](int k, size_t p) { put(upper, count, p, reduce(d[k], kP)); });
	});
}

inline void sqrt(const Transform& t, ConstWords f, Word c, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0]) std::abort();
	if (n <= kSqrtBase) return sqrt_direct(f, c, g);
	const size_t m = size_t(1) << sqrt_log(n), words = Arena::footprint(m);
	const Words b = scratch.first(m), ht = scratch.subspan(words, m), work = scratch.subspan(2 * words, m);
	sqrt_steps(t, f, c, g.first(m), b, ht);
	t.forward(g.first(m), 0, b);
	sqrt_last_step(t, f, b, ht, work, g.subspan(m));
}

}
namespace fps_detail::blocked_poly {
inline void native_sqrt(const mint *in, size_t input, std::vector<mint> &out, size_t n, mint constant) {
	const int lg = sqrt_log(n);
	const size_t c = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(c) + Arena::footprint(n) + Arena::footprint(sqrt_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(c), g = arena.take(n), scratch = arena.take(sqrt_scratch(n));
	copy_coefficients(in, f);
	const Word ct = blocked_ntt::detail::multiply_mod(std::min(constant.v, constant.v - mint::mod()),
		blocked_ntt::detail::power(65536, mint::mod() - 2));
	sqrt(t, f, ct, g, scratch);
	append_scaled(g, out, 65536);
}
}

#pragma GCC pop_options
#endif

#line 1 "components/mint_convolution.hpp"



#line 1 "components/fps_convolution.hpp"


#line 1 "components/fps_convolution_unbalanced.hpp"



#line 10 "components/fps_convolution_unbalanced.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::unbalanced {
using namespace blocked_poly;
inline bool fits(size_t n, size_t m) {
	const auto shorter = std::min(n, m), longer = std::max(n, m);
	return shorter > 32 && longer >= 32768 && shorter <= longer / 32;
}
inline std::vector<mint> multiply(std::span<const mint> a, std::span<const mint> b) {
	const size_t len = 8 * std::bit_ceil(std::min(a.size(), b.size()));
	const auto& x = a.size() >= b.size() ? a : b;
	const auto& y = a.size() >= b.size() ? b : a;
	const size_t n = x.size(), m = y.size(), stride = len - m + 1;
	const int lg = std::countr_zero(len);
	Arena arena(Transform::words(lg) + 3 * Arena::footprint(len));
	const Transform t(arena, lg);
	auto bt = arena.take(len), in = arena.take(len), out = arena.take(len);
	for (size_t i = 0; i < m; ++i) in[i] = y[i].val();
	t.forward(in.first(m), 0, bt);
	std::vector<mint> result;
	fps_detail::prepare_fps_output(result, n + m - 1);
	for (size_t p = 0; p < n; p += stride) {
		const size_t count = std::min(stride, n - p);
		for (size_t i = 0; i < count; ++i) {
			const unsigned v = x[p + i].v;
			in[i] = std::min(v, v - mint::mod());
		}
		t.cyclic_product(in.first(count), 0, out, bt);
		const size_t overlap = p == 0 ? 0 : m - 1;
		for (size_t i = 0; i < overlap; ++i) {
			const unsigned v = result[p + i].v + out[i];
			result[p + i] = mint::word(std::min(v, v - mint::mod()));
		}
		fps_detail::append_fps_words<true>(result, out.data() + overlap, count + m - 1 - overlap);
	}
	return result;
}
}
#pragma GCC pop_options
#endif

#line 1 "components/fps_convolution_constant.hpp"



#line 11 "components/fps_convolution_constant.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail::constant {
inline unsigned canonical(mint x) { return std::min(x.v, x.v - mint::mod()); }
inline bool same_eight(const mint* p, unsigned value) {
	__m256i v;
	std::memcpy(&v, p, sizeof(v));
	v = _mm256_min_epu32(v, _mm256_sub_epi32(v, _mm256_set1_epi32(mint::mod())));
	return _mm256_movemask_epi8(_mm256_cmpeq_epi32(v, _mm256_set1_epi32(value))) == -1;
}
inline bool remaining(std::span<const mint> a, unsigned value) {
	size_t i = 8;
	for (; i + 8 <= a.size(); i += 8) if (!same_eight(a.data() + i, value)) return false;
	for (; i < a.size(); ++i) if (canonical(a[i]) != value) return false;
	return true;
}
[[gnu::noinline]] inline bool fits(const std::vector<mint>& a, const std::vector<mint>& b) {
	if (std::min(a.size(), b.size()) < 4096) return false;
	const unsigned av = canonical(a[0]), bv = canonical(b[0]);
	if (!same_eight(a.data(), av) || !same_eight(b.data(), bv)) return false;
	return remaining(a, av) && (&a == &b || remaining(b, bv));
}
[[gnu::noinline]] inline std::vector<mint> multiply(std::span<const mint> a, std::span<const mint> b) {
	const size_t n = std::max(a.size(), b.size()), m = std::min(a.size(), b.size()), size = n + m - 1;
	const unsigned c = canonical(mint::word(canonical(a[0])) * mint::word(canonical(b[0])));
	std::vector<mint> result(size);
	if (c == 0) return result;
	alignas(32) unsigned first[8];
	for (unsigned i = 0; i < 8; ++i) first[i] = std::uint64_t(c) * (i + 1) % mint::mod();
	__m256i value = _mm256_load_si256(reinterpret_cast<const __m256i*>(first));
	const auto step = _mm256_set1_epi32(std::uint64_t(c) * 8 % mint::mod()), p = _mm256_set1_epi32(mint::mod());
	const auto reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	size_t i = 0;
	for (; i + 8 <= m; i += 8) {
		std::memcpy(static_cast<void*>(result.data() + i), &value, sizeof(value));
		const auto reflected = _mm256_permutevar8x32_epi32(value, reverse);
		std::memcpy(static_cast<void*>(result.data() + size - i - 8), &reflected, sizeof(reflected));
		value = _mm256_add_epi32(value, step);
		value = _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
	}
	unsigned current = _mm256_extract_epi32(value, 0);
	for (; i < m; ++i) {
		result[i] = result[size - i - 1] = mint::word(current);
		current += c;
		current = std::min(current, current - mint::mod());
	}
	if (n > m + 1) std::fill(result.begin() + m, result.begin() + n - 1, mint::word(std::uint64_t(c) * m % mint::mod()));
	return result;
}
}
#pragma GCC pop_options
#endif

#line 14 "components/fps_convolution.hpp"
#if CP_GCC_LINUX64
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace fps_detail::multiply_detail {
namespace ntt {
namespace kernels = blocked_ntt::kernels;
namespace detail = blocked_ntt::detail;
class Convolution {
	size_t n_, m_, len_;
	blocked_poly::Arena arena_;
	blocked_poly::Transform transform_;
	Word *a_, *b_;
	static int log_length(size_t n, size_t m) {
		if (n == 0 || m == 0 || n > (1u << 23) || m > (1u << 23) || n + m - 1 > (1u << 23)) std::abort();
		return std::max(6, int(std::bit_width(n + m - 2)));
	}
public:
	Convolution(size_t n, size_t m) : Convolution(n, m, nullptr) {}
	Convolution(size_t n, size_t m, Word* borrowed)
		: n_(n), m_(m), len_(size_t(1) << log_length(n, m)),
		  arena_(blocked_poly::Transform::words(log_length(n, m)) + (borrowed ? 1 : 2) * blocked_poly::Arena::footprint(len_)),
		  transform_(arena_, log_length(n, m)), a_(borrowed ? borrowed : arena_.take(len_).data()), b_(arena_.take(len_).data()) {
		if (n == 0 || m == 0 || log_length(n, m) > 23) std::abort();
	}
	static bool fits(size_t n, size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= size_t(1) << lg;
	}
	Word* a() { return a_; }
	Word* b() { return b_; }
	const Word* multiply(bool square = false) {
		transform_.forward(ConstWords(b_, m_), 0, Words(b_, len_));
		if (square) transform_.inverse_product(ConstWords(b_, len_), ConstWords(b_, len_), Words(a_, len_),
			blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		else transform_.cyclic_product(ConstWords(a_, n_), 0,
			Words(a_, len_), ConstWords(b_, len_), blocked_poly::Half::kBoth, detail::power(detail::kR, kernels::kP - 2));
		return a_;
	}
};
}
using Product = ntt::Convolution;
using ntt::detail::Vec, ntt::detail::Factor, ntt::detail::add, ntt::detail::diff, ntt::detail::kP, ntt::detail::reduce;
inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }
inline Vec diff_canonical(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y); }
class Subtrees {
	const Word *roots_, *inverse_;
public:
	Subtrees(const Word* r, const Word* ir) : roots_(r), inverse_(ir) {}
	void visit(Vec* a, Vec* b, size_t nv, size_t k) const {
		using namespace blocked_poly::detail;
		Recursion<ForwardBottom>(roots_, inverse_, ForwardBottom{roots_}).visit(reinterpret_cast<Word*>(b), nv, k);
		ProductBottom bottom{roots_, inverse_, reinterpret_cast<Word*>(b), k * nv};
		Recursion<ProductBottom>(roots_, inverse_, bottom).visit(reinterpret_cast<Word*>(a), nv, k);
	}
};
inline void forward_radix8(Vec* f, size_t q, const Word* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (size_t j = 0; j < q; ++j) {
		const Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];

		const Vec g0 = add(f0, f2), g1 = add(f1, f3);
		const Vec h0 = diff_canonical(f0, f2), ih1 = times(diff_canonical(f1, f3), i);
		f[j] = add(g0, g1), f[j + q] = diff(g0, g1);
		f[j + 2 * q] = add(h0, ih1), f[j + 3 * q] = diff(h0, ih1);

		const Vec if2 = times(f2, i), if3 = times(f3, i);
		const Vec u0 = reduce(add(f0, if2), 2 * kP), v0 = reduce(diff(f0, if2), 2 * kP);
		const Vec yu1 = times(add(f1, if3), y), zv1 = times(diff(f1, if3), z);
		f[j + 4 * q] = add(u0, yu1), f[j + 5 * q] = diff(u0, yu1);
		f[j + 6 * q] = add(v0, zv1), f[j + 7 * q] = diff(v0, zv1);
	}
}
inline void copy_input(const std::vector<mint> &in, Word *out, size_t len) {
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<mint>& in, mint* out) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	size_t i = 0;
	const auto p = _mm256_set1_epi32(mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = mint::word(std::min(in[i].v, in[i].v - mint::mod()));
}

template <class Plan>
inline std::vector<mint> run_borrowed_as(const std::vector<mint>& a, const std::vector<mint>& b) {
	const size_t len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr size_t alignment = 32;
	const size_t words = len + 16 + alignment / sizeof(mint) - 1, size = a.size() + b.size() - 1;
	std::vector<mint> result;
	prepare_fps_output(result, words);
	result.assign(words, mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(mint);
	mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<Word*>(to));
	if (&a != &b) copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply(&a == &b);
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(mint));
	result.resize(size);
	return result;
}

inline std::vector<mint> run_borrowed(const std::vector<mint>& a, const std::vector<mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<mint> run(const std::vector<mint> &a, const std::vector<mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<mint> result;
	const size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}


inline std::vector<mint> multiply(const std::vector<mint>& a, const std::vector<mint>& b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}
}
#pragma GCC pop_options
#endif

#line 10 "components/mint_convolution.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_convolution_detail {
inline std::vector<mint> multiply(const std::vector<mint> &a, const std::vector<mint> &b) {
	using NTT = fps_detail::NTT;
	using Word = NTT::U;
	if (a.empty() || b.empty()) return {};
	auto words = [](auto &v) { return reinterpret_cast<Word *>(v.data()); };
	auto input = [](const auto &v) { return reinterpret_cast<const Word *>(v.data()); };
	if (std::min(a.size(), b.size()) <= 32) {
		std::vector<mint> result(a.size() + b.size() - 1);
		const auto &small = a.size() < b.size() ? a : b;
		const auto &large = a.size() < b.size() ? b : a;
		for (int j = 0; j < static_cast<int>(small.size()); ++j)
			NTT::add_scaled(words(result) + j, input(large), large.size(), small[j].val());
		return result;
	}
	int n = 1, size = a.size() + b.size() - 1;
	while (n < size) n <<= 1;
	assert(n <= (1 << 23));
#if CP_GCC_LINUX64
	if (NTT::vectorized()) return fps_detail::multiply_detail::multiply(a, b);
#endif
	std::vector<mint> x = a, y;
	x.resize(n);
	if (&a != &b) y = b, y.resize(n);
	auto &plan = fps_detail::ntt_plan(std::max(1, n / 2));
	plan.fft<false>(words(x), n);
	if (&a == &b) NTT::product(words(x), input(x), input(x), n);
	else {
		plan.fft<false>(words(y), n);
		NTT::product(words(x), input(x), input(y), n);
	}
	plan.fft<true>(words(x), n);
	plan.scale(words(x), n, mint(n).inv().val());
	x.resize(size);
	return x;
}
}

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 1 "components/fps_division_blocked.hpp"



#line 6 "components/fps_division_blocked.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

inline size_t quotient_size(size_t n, size_t m) { return n >= m ? n - m + 1 : 0; }
inline size_t remainder_size(size_t n, size_t m) { return std::min(n, m - 1); }

namespace division_detail {
using namespace detail;

inline constexpr size_t kDivisionTerms = 8;

inline constexpr size_t kLongDivision = size_t(1) << 12;

inline constexpr size_t kDirectRemainder = 64;

inline constexpr std::array<double, 26> kTransformNs = {0,    0,    0,    0,    0,    0,    0.45, 0.48, 0.51,
														0.55, 0.6,  0.65, 0.7,  0.76, 0.81, 0.88, 0.95, 1.02,
														1.09, 1.17, 1.23, 1.3,  1.37, 1.44, 1.51, 1.58};
inline constexpr double kLeafProductNs = 0.92, kDirectTermNs = 0.04, kFoldNs = 0.25;

inline size_t transform_length(size_t n) { return std::max<size_t>(64, std::bit_ceil(n)); }

inline double transform_ns(size_t len) {
	const auto lg = std::countr_zero(len);
	return size_t(lg) < kTransformNs.size() ? kTransformNs[lg] * double(len) : std::numeric_limits<double>::infinity();
}
inline double cyclic_ns(size_t len) { return 2 * transform_ns(len) + kLeafProductNs * double(len); }

inline double inverse_ns(size_t n) {
	double ns = 0;
	for (size_t k = kInverseBase; k < n; k *= 2) ns += transform_ns(2 * k) + 2 * cyclic_ns(2 * k);
	return ns;
}

struct QuotientPlan {
	size_t s = 0, blocks = 0, pad = 0, windows = 0, tail = 0;
	bool tails = false;
	double ns = std::numeric_limits<double>::infinity();

	QuotientPlan() = default;

	QuotientPlan(size_t k, size_t d, size_t block)
		: s(block), blocks((k + block - 1) / block), pad(blocks * block - k), tail(transform_length(2 * d)),
		  tails(d < block) {
		const size_t len = 2 * s;
		ns = inverse_ns(precision(k)) + transform_ns(len) + double(blocks) * cyclic_ns(len);
		if (blocks == 1) return;
		if (tails) {
			ns += transform_ns(tail) + double(blocks - 1) * cyclic_ns(tail);
			return;
		}
		const size_t reach = d / s + 1;
		windows = std::min(reach, blocks - 1);
		if (windows > kDivisionTerms) {
			ns = std::numeric_limits<double>::infinity();
			return;
		}
		double terms = 0;
		for (size_t j = 1; j < blocks; ++j) terms += double(std::min(j, reach));
		ns += double(windows + 2 * (blocks - 1)) * transform_ns(len) + terms * kLeafProductNs * double(len);
	}

	size_t precision(size_t k) const { return blocks == 1 ? k : s; }

	size_t scratch(bool use_tail) const {
		size_t later = Arena::footprint(blocks * s + s);
		if (use_tail) later += 2 * Arena::footprint(tail);
		if (windows) later += (2 * windows + 1) * Arena::footprint(2 * s);
		return std::max(inverse_scratch(precision(blocks * s - pad)), later);
	}
};

enum class Remainder { kDirect, kWrap, kSplit, kTail };

struct RemainderPlan {
	Remainder method;
	size_t length = 0;
	double ns = std::numeric_limits<double>::infinity();

	RemainderPlan(Remainder how, size_t n, size_t k, size_t d, const QuotientPlan& quotient) : method(how) {
		const size_t wrap = transform_length(d);
		switch (how) {
			case Remainder::kDirect:
				if (k <= kDirectRemainder) ns = double(k) * double(d) * kDirectTermNs;
				break;
			case Remainder::kWrap:
				length = wrap;
				ns = cyclic_ns(length) + transform_ns(length) + double(n + k) * kFoldNs;
				break;
			case Remainder::kSplit:
				length = wrap / 2;
				if (length >= 64 && d > length && k + (d - length) <= length + 1)
					ns = transform_ns(length) + 2 * cyclic_ns(length) + double(n + d) * kFoldNs;
				break;
			case Remainder::kTail:
				if (!quotient.tails) break;
				ns = (quotient.blocks == 1 ? transform_ns(quotient.tail) : 0) + cyclic_ns(quotient.tail) + double(d) * kFoldNs;
				break;
		}
	}

	size_t buffers() const { return method == Remainder::kWrap ? 2 : method == Remainder::kSplit ? 3 : 0; }
};

struct DivisionPlan {
	size_t n, m, k, d;
	bool long_division;
	QuotientPlan quotient;
	RemainderPlan remainder;
	int lg = Transform::kMinLog;

	DivisionPlan(size_t n_, size_t m_)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(k * d <= kLongDivision),
		  remainder(Remainder::kWrap, n, k, d, quotient) {
		if (long_division) return;
		double best = std::numeric_limits<double>::infinity();
		for (size_t s = 32;; s *= 2) {
			const QuotientPlan p(k, d, s);
			for (Remainder how : {Remainder::kDirect, Remainder::kWrap, Remainder::kSplit, Remainder::kTail}) {
				const RemainderPlan r(how, n, k, d, p);
				if (p.ns + r.ns < best) best = p.ns + r.ns, quotient = p, remainder = r;
			}
			if (s >= k || 4 * s > (size_t(1) << Transform::kMaxLog)) break;
		}
		set_log();
	}

	DivisionPlan(size_t n_, size_t m_, size_t s, Remainder how)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(false), quotient(k, d, s),
		  remainder(how, n, k, d, quotient) {
		set_log();
	}

	bool tails() const { return quotient.tails && (quotient.blocks > 1 || remainder.method == Remainder::kTail); }

	void set_log() { lg = std::countr_zero(std::max({2 * quotient.s, tails() ? quotient.tail : 0, remainder.length})); }

	size_t divisor() const {
		const QuotientPlan& p = quotient;
		return tails() ? m : std::min(m, std::max(p.precision(k), (p.windows + 1) * p.s));
	}

	size_t quotient_words() const {
		const QuotientPlan& p = quotient;
		return Arena::footprint(divisor()) + Arena::footprint(2 * p.s) + p.scratch(tails());
	}

	size_t scratch() const { return std::max(quotient_words(), remainder.buffers() * Arena::footprint(remainder.length)); }

	size_t words() const {
		return long_division ? Arena::footprint(n) : Transform::words(lg) + Arena::footprint(scratch());
	}
};

class Bump {
public:
	explicit Bump(Words memory) : memory_(memory) {}

	Words take(size_t n) {
		if (used_ > memory_.size() || Arena::footprint(n) > memory_.size() - used_) throw std::bad_alloc();
		const Words s = memory_.subspan(used_, n);
		used_ += Arena::footprint(n);
		return s;
	}

	Words rest() const { return memory_.subspan(used_); }

private:
	Words memory_;
	size_t used_ = 0;
};

inline Vec montgomery_reduce(Vec even, Vec odd) {
	return mint_detail::Ops::reduce8(even, odd);
}

inline Vec load_any(const Word* p) { return load_unaligned(p); }
inline Vec load_any(const mint* p) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	Vec value;
	std::memcpy(&value, p, sizeof(value));
	return reduce(value, kP);
}
inline void store_any(Word* p, Vec x) { store_unaligned(p, x); }
inline Vec reversed(Vec x) { return _mm256_permutevar8x32_epi32(x, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }

inline void append_words(std::vector<mint>& out, const Word* values, size_t n, Word factor = 1) {
	const Factor scale(factor);
	size_t i = 0;
	for (; i + 8 <= n; i += 8) {
		Vec word = load_any(values + i);
		if (factor != 1) word = times(word, scale);
		mint block[8];
		static_assert(sizeof(block) == sizeof(word) && std::is_trivially_copyable_v<mint>);
		std::memcpy(static_cast<void*>(block), &word, sizeof(block));
		out.insert(out.end(), block, block + 8);
	}
	for (; i < n; ++i) out.emplace_back(mint::word(factor == 1 ? values[i] : blocked_ntt::detail::multiply_mod(values[i], factor)));
}

inline void append_division_words(std::vector<mint>& out, const Word* values, size_t n, Word factor = 1) {
	static_assert(sizeof(mint) == sizeof(Word) && std::is_trivially_copyable_v<mint>);
	const Factor scale(factor);
	std::array<mint, 64> batch;
	size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if (factor == 1) std::memcpy(static_cast<void*>(batch.data()), values + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (size_t j = 0; j < 64; j += 8) {
				const Vec word = times(load_any(values + i + j), scale);
				std::memcpy(static_cast<void*>(batch.data() + j), &word, sizeof(word));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	if (i < n) append_words(out, values + i, n - i, factor);
}

template<class T>
inline void subtract(const T* a, const Word* b, Word* out, size_t count) {
	size_t i = 0;
	for (; i + 8 <= count; i += 8) store_any(out + i, difference(load_any(a + i), load_any(b + i)));
	for (; i < count; ++i) out[i] = (mint_scalar(a[i]) + kP - b[i]) % kP;
}

template<class T>
inline void reverse_difference(std::span<T> a, std::ptrdiff_t top, Word* out, size_t count,
							   const Word* sub = nullptr) {
	const std::ptrdiff_t size = std::ptrdiff_t(a.size());
	const auto value = [&](size_t i) {
		const std::ptrdiff_t p = top - std::ptrdiff_t(i);
		const Word x = p >= 0 && p < size ? mint_scalar(a[size_t(p)]) : 0;
		return sub ? (x + kP - sub[i]) % kP : x;
	};
	size_t i = top >= size ? std::min(count, size_t(top - size + 1)) : 0;
	if (sub) {
		for (size_t j = 0; j < i; ++j) out[j] = value(j);
	} else {
		std::fill_n(out, i, 0);
	}
	for (; i + 8 <= count && top - std::ptrdiff_t(i) >= 7; i += 8) {
		const Vec x = reversed(load_any(a.data() + (top - std::ptrdiff_t(i) - 7)));
		store_any(out + i, sub ? difference(x, load_any(sub + i)) : x);
	}
	for (; i < count; ++i) out[i] = value(i);
}

template<class T>
inline void fold(std::span<T> a, Words out) {
	const size_t len = out.size();
	std::fill(out.begin(), out.end(), 0);
	for (size_t from = 0; from < a.size(); from += len) {
		const size_t count = std::min(len, a.size() - from), full = count / 8 * 8;
		const T* x = a.data() + from;
		for (size_t j = 0; j < full; j += 8) {
			store(out.data() + j, reduce(add(load(out.data() + j), load_any(x + j)), kP));
		}
		for (size_t j = full; j < count; ++j) out[j] = (out[j] + mint_scalar(x[j])) % kP;
	}
}

template<class T>
inline void long_division(std::span<const T> f, ConstWords g, Words q,
						  Words r, Words rest) {
	using blocked_ntt::detail::multiply_mod;
	const size_t d = g.size() - 1;
	if constexpr (std::is_same_v<T, Word>) std::copy(f.begin(), f.end(), rest.begin());
	else for (size_t i = 0; i < f.size(); ++i) rest[i] = mint_scalar(f[i]);
	const Word lead = blocked_ntt::detail::power(g[d], kP - 2);
	for (size_t i = q.size(); i-- > 0;) {
		const Word c = multiply_mod(rest[i + d], lead);
		q[i] = c;
		for (size_t t = 0; t < d; ++t) rest[i + t] = (rest[i + t] + kP - multiply_mod(c, g[t])) % kP;
	}
	std::copy_n(rest.begin(), r.size(), r.begin());
}

template<class T>
inline void remainder_direct(std::span<const T> f, ConstWords g,
							 ConstWords q, Words r) {
	const size_t d = r.size(), k = q.size();
	Vec factor[kDirectRemainder];
	for (size_t i = 0; i < k; ++i) factor[i] = _mm256_set1_epi64x(std::int64_t((std::uint64_t(q[i]) << 32) % kP));
	const auto scalar = [&](size_t j) {
		std::uint64_t sum = 0;
		for (size_t i = 0; i <= std::min(j, k - 1); ++i) sum = (sum + std::uint64_t(q[i]) * g[j - i]) % kP;
		r[j] = Word((mint_scalar(f[j]) + kP - sum) % kP);
	};
	size_t j = 0;
	for (; j < std::min(d, k); ++j) scalar(j);
	for (; j + 8 <= d; j += 8) {
		Vec total = load_any(f.data() + j);
		for (size_t i = 0; i < k; i += 4) {
			Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
			for (size_t t = i; t < std::min(k, i + 4); ++t) {
				const Vec x = load_any(g.data() + (j - t));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, factor[t]));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), factor[t]));
			}
			total = reduce(_mm256_sub_epi32(add(total, broadcast(2 * kP)), montgomery_reduce(even, odd)), 2 * kP);
		}
		store_any(r.data() + j, reduce(total, kP));
	}
	for (; j < d; ++j) scalar(j);
}

template<class T>
inline void remainder_wrap(const Transform& t, std::span<const T> f, ConstWords g,
						   ConstWords q, Words r, Words a,
						   Words b) {
	const size_t len = a.size();
	if (q.size() <= len) {
		t.forward(q, 0, a);
	} else {
		fold(q, a);
		t.forward(a);
	}
	ConstWords gl = g;
	if (g.size() > len) {
		std::copy_n(g.begin(), len, b.begin());
		b[0] = (b[0] + g[len]) % kP;
		gl = b;
	}
	t.cyclic_product(gl, 0, b, a);
	fold(f, a);
	subtract(a.data(), b.data(), r.data(), r.size());
}

template<class T>
inline void remainder_split(const Transform& t, std::span<const T> f, ConstWords g,
							ConstWords q, Words r, Words a,
							Words b, Words c) {
	const size_t len = a.size(), k = q.size(), e = r.size() - len;
	t.forward(q, 0, a);
	t.cyclic_product(g.subspan(len + 1 - k, k + e - 1), 0, c, a);
	subtract(f.data() + len, c.data() + (k - 1), r.data() + len, e);
	fold(g, b);
	t.cyclic_product(b, a);
	fold(f, c);
	subtract(c.data(), b.data(), r.data(), len);
	subtract(r.data(), r.data() + len, r.data(), e);
}

template <size_t K>
void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, Words out) {
	InverseProductSumBottom<K> bottom{};
	for (size_t i = 0; i < K; ++i)
		bottom.terms[i] = {{t.roots(), t.inverse_roots(), pairs[i].b.data()}, pairs[i].a.data()};
	t.inverse_with(bottom, out, Half::kUpper);
}

inline void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, Words out) {
	switch (pairs.size()) {
		case 1:
		case 2:
		case 3: return t.inverse_product_sum(pairs, out, Half::kUpper);
		case 4: return residual_sum<4>(t, pairs, out);
		case 5: return residual_sum<5>(t, pairs, out);
		case 6: return residual_sum<6>(t, pairs, out);
		case 7: return residual_sum<7>(t, pairs, out);
		case 8: return residual_sum<8>(t, pairs, out);
		default: std::abort();
	}
}

template<class T>
inline void subtract_reversed(const T* a, const Word* b, Word* out, size_t count) {
	size_t j = 0;
	for (; j + 8 <= count; j += 8) store_any(out + j, difference(load_any(a + j), reversed(load_any(b + count - 8 - j))));
	for (; j < count; ++j) out[j] = (mint_scalar(a[j]) + kP - b[count - 1 - j]) % kP;
}

template<class T>
inline void divide_blocks(const Transform& t, const DivisionPlan& plan, std::span<const T> f,
						  ConstWords g, Words q, Words r,
						  Words scratch) {
	const QuotientPlan& p = plan.quotient;
	const size_t k = q.size(), s = p.s, len = 2 * s, d = plan.d;
	const std::ptrdiff_t top = std::ptrdiff_t(f.size() - 1 + p.pad);
	Bump bump(scratch);
	const Words G = bump.take(plan.divisor()), ht = bump.take(len);
	std::reverse_copy(g.end() - std::ptrdiff_t(G.size()), g.end(), G.begin());
	const size_t precision = p.precision(k);
	inverse(t, G.first(std::min(G.size(), precision)), ht.first(precision), bump.rest());
	t.forward(ht.first(precision), 0, ht);

	const Words qs = bump.take(p.blocks * s + s);
	Words gt, mp, work;
	std::array<Words, kDivisionTerms> windows{}, ring{};
	if (plan.tails()) {
		gt = bump.take(p.tail), mp = bump.take(p.tail);
		t.forward(G, 0, gt);
	}
	if (p.windows) {
		work = bump.take(len);
		for (size_t i = 0; i < p.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(G.subspan(i * s, std::min(len, G.size() - i * s)), 0, windows[i]);
		}
	}

	const auto tail_product = [&](size_t j) {
		t.cyclic_product(qs.subspan(j * s - d, d), p.tail - 2 * d, mp, gt, Half::kUpper);
		return mp.data() + (p.tail - d);
	};
	for (size_t j = 0; j < p.blocks; ++j) {
		Word* const block = qs.data() + j * s;
		const std::ptrdiff_t from = top - std::ptrdiff_t(j * s);
		if (j == 0) {
			reverse_difference(f, from, block, s);
		} else if (p.tails) {
			reverse_difference(f, from, block, d, tail_product(j));
			reverse_difference(f, from - std::ptrdiff_t(d), block + d, s - d);
		} else {
			const size_t terms = std::min(j, d / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % p.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			reverse_difference(f, from, block, s, work.data() + s);
		}
		t.cyclic_product(qs.subspan(j * s, s), 0, qs.subspan(j * s, len), ht, Half::kLower);
		if (p.windows && j + 1 < p.blocks) t.forward(qs.subspan(j * s, s), 0, ring[j % p.windows]);

		reverse_difference(qs.first((j + 1) * s), std::ptrdiff_t((j + 1) * s - 1), q.data() + (p.blocks - 1 - j) * s,
						   j == 0 ? s - p.pad : s);
	}

	if (plan.remainder.method == Remainder::kTail) subtract_reversed(f.data(), tail_product(p.blocks), r.data(), d);
}

template<class T>
inline void divide(Arena& arena, const DivisionPlan& plan, std::span<const T> f, ConstWords g,
				   Words q, Words r) {
	if (plan.long_division) return long_division(f, g, q, r, arena.take(f.size()));
	const Transform t(arena, plan.lg);
	const Words scratch = arena.take(plan.scratch());
	divide_blocks(t, plan, f, g, q, r, scratch);
	Bump bump(scratch);
	const size_t len = plan.remainder.length;
	switch (plan.remainder.method) {
		case Remainder::kDirect: return remainder_direct(f, g, q, r);
		case Remainder::kWrap: return remainder_wrap(t, f, g, q, r, bump.take(len), bump.take(len));
		case Remainder::kSplit: return remainder_split(t, f, g, q, r, bump.take(len), bump.take(len), bump.take(len));
		case Remainder::kTail: return;
	}
}

}

inline size_t divide_words(size_t n, size_t m) { return division_detail::DivisionPlan(n, m).words(); }

inline void divide(Arena& arena, ConstWords f, ConstWords g,
				   Words q, Words r) {
	const division_detail::DivisionPlan plan(f.size(), g.size());
	if (plan.k == 0) {
		std::copy(f.begin(), f.end(), r.begin());
		return;
	}
	division_detail::divide(arena, plan, f, g, q, r);
}

}

namespace fps_detail::blocked_poly {
inline void native_divide(const mint* a, size_t n, const mint* b, size_t m,
						  std::vector<mint>& quotient, std::vector<mint>& remainder) {
	const division_detail::DivisionPlan plan(n, m);
	const size_t k = quotient_size(n, m), d = remainder_size(n, m);
	Arena arena(plan.words() + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));
	const std::span<const mint> f(a, n);
	auto g = arena.take(m), q = arena.take(k), r = arena.take(d);
	copy_coefficients(b, g);
	division_detail::divide(arena, plan, f, g, q, r);
	prepare_fps_output(quotient, k), prepare_fps_output(remainder, d);
	division_detail::append_division_words(quotient, q.data(), q.size(), blocked_ntt::detail::kR);
	division_detail::append_division_words(remainder, r.data(), r.size());
}

inline void native_div_series(const mint* a, size_t input, const mint* b, size_t divisor,
							  std::vector<mint>& out, size_t n) {
	using namespace division_detail;
	const size_t count = std::min(divisor, n), degree = count - 1;
	QuotientPlan plan;
	for (size_t s = 32;; s *= 2) {
		QuotientPlan candidate(n, degree, s);
		if (candidate.ns < plan.ns) plan = candidate;
		if (s >= n || 4 * s > (size_t(1) << Transform::kMaxLog)) break;
	}
	const size_t s = plan.s, len = 2 * s;
	const bool tails = plan.tails && plan.blocks > 1;
	const size_t precision = plan.precision(n), numerator = std::min(input, n);
	const int lg = std::countr_zero(std::max(len, tails ? plan.tail : 0));
	const size_t workspace = plan.scratch(tails);
	Arena arena(Transform::words(lg) + Arena::footprint(numerator) + Arena::footprint(count) +
		Arena::footprint(len) + Arena::footprint(workspace));
	const Transform t(arena, lg);
	auto f = arena.take(numerator), g = arena.take(count), ht = arena.take(len);
	copy_coefficients(a, f);
	copy_coefficients(b, g);
	auto scratch = arena.take(workspace);
	inverse(t, g.first(std::min(count, precision)), ht.first(precision), scratch);
	t.forward(ht.first(precision), 0, ht);
	Bump bump(scratch);
	auto qs = bump.take(plan.blocks * s + s);
	Words gt, mp, work;
	std::array<Words, kDivisionTerms> windows{}, ring{};
	if (tails) {
		gt = bump.take(plan.tail), mp = bump.take(plan.tail);
		t.forward(g, 0, gt);
	}
	if (plan.windows) {
		work = bump.take(len);
		for (size_t i = 0; i < plan.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(g.subspan(i * s, std::min(len, count - i * s)), 0, windows[i]);
		}
	}
	prepare_fps_output(out, n);
	for (size_t j = 0; j < plan.blocks; ++j) {
		auto block = qs.subspan(j * s, len);
		const size_t from = j * s, available = from < numerator ? std::min(s, numerator - from) : 0;
		if (available) std::copy_n(f.data() + from, available, block.data());
		std::fill(block.begin() + available, block.begin() + s, 0u);
		if (j && tails) {
			t.cyclic_product(qs.subspan(j * s - degree, degree), plan.tail - 2 * degree, mp, gt, Half::kUpper);
			subtract(block.data(), mp.data() + plan.tail - degree, block.data(), degree);
		} else if (j) {
			const size_t terms = std::min(j, degree / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % plan.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			subtract(block.data(), work.data() + s, block.data(), s);
		}
		t.cyclic_product(block.first(s), 0, block, ht, Half::kLower);
		if (plan.windows && j + 1 < plan.blocks) t.forward(block.first(s), 0, ring[j % plan.windows]);
		append_division_words(out, block.data(), std::min(s, n - from), blocked_ntt::detail::kR);
	}
}
}

#pragma GCC pop_options
#endif

#line 24 "fps/fps.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_API
#define CP_FPS_API 1
template <class T = mint>
struct [[nodiscard, gnu::warn_unused]] FormalPowerSeries : std::vector<T>, private fps_detail::Operations<T> {
	static_assert(T::mod() == 998244353, "FormalPowerSeries supports only mod 998244353");
	using W = FormalPowerSeries<>;
	using base = std::vector<T>;
	using F = FormalPowerSeries;
	using Sparse = fps_detail::SparseSeries;
	using base::base;

	FormalPowerSeries() = default;
	FormalPowerSeries(base v) : base(std::move(v)) {}
	F &operator=(base v) {
		base::operator=(std::move(v));
		return *this;
	}
	F &operator=(std::initializer_list<T> v) {
		base::operator=(v);
		return *this;
	}

	static F multiply(const F &a, const F &b) {
		if constexpr (std::is_same_v<T, mint>) return F(mint_convolution_detail::multiply(a, b));
		else {
			W x = import_work(a);
			return export_work(&a == &b ? W::multiply(x, x) : W::multiply(x, import_work(b)));
		}
	}

	F operator-() const {
		F r = *this;
		for (T &x : r) x = -x;
		return r;
	}
	F &operator+=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] += g[i];
		return *this;
	}
	F &operator-=(const F &g) {
		if (this->size() < g.size()) this->resize(g.size());
		for (std::size_t i = 0; i < g.size(); ++i) (*this)[i] -= g[i];
		return *this;
	}
	F &operator+=(T x) {
		if (this->empty()) this->resize(1);
		(*this)[0] += x;
		return *this;
	}
	F &operator-=(T x) { return *this += -x; }
	F &operator*=(const F &g) { return *this = multiply(*this, g); }
	F &operator*=(T x) {
		if constexpr (std::is_same_v<T, mint>) {
			scale_words(*this, x);
			return *this;
		}
		for (T &a : *this) a *= x;
		return *this;
	}
	F &operator/=(const F &g) { return *this = div_series(g); }
	F &operator%=(const F &g) { return *this = divmod(g).second; }
	F &operator/=(T x) { return *this *= x.inv(); }

	F &operator<<=(int d) {
		assert(d >= 0);
		auto edge = std::shift_right(this->begin(), this->end(), d);
		std::fill(this->begin(), edge, T(0));
		return *this;
	}
	F &operator>>=(int d) {
		assert(d >= 0);
		auto edge = std::shift_left(this->begin(), this->end(), d);
		std::fill(edge, this->end(), T(0));
		return *this;
	}

	F operator+(const F &g) const {
		F r(*this);
		r += g;
		return r;
	}
	F operator-(const F &g) const {
		F r(*this);
		r -= g;
		return r;
	}
	F operator*(const F &g) const { return multiply(*this, g); }
	F operator/(const F &g) const { return div_series(g); }
	F operator%(const F &g) const { return divmod(g).second; }
	F operator+(T x) const {
		F r(*this);
		r += x;
		return r;
	}
	F operator-(T x) const {
		F r(*this);
		r -= x;
		return r;
	}
	F operator*(T x) const {
		F r(*this);
		r *= x;
		return r;
	}
	F operator/(T x) const {
		F r(*this);
		r /= x;
		return r;
	}
	friend F operator+(T x, F f) { return f += x; }
	friend F operator-(T x, const F &f) { return -f + x; }
	friend F operator*(T x, F f) { return f *= x; }
	F operator<<(int d) const {
		F r(*this);
		r <<= d;
		return r;
	}
	F operator>>(int d) const {
		F r(*this);
		r >>= d;
		return r;
	}

	F pre(int n) const {
		assert(n >= 0);
		return F(this->begin(), this->begin() + std::min<int>(n, this->size()));
	}
	void shrink() { O::trim(*this); }
	void trim() { shrink(); }

	F div_series(const F &g, int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).div_series(import_work(g), deg));
		if (deg < 0) deg = this->size();
		assert(deg >= 0);
		if (!deg) return {};
		assert(!g.empty() && g[0] != T(0));
		if (this->empty()) return F(deg);
#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 129 && deg <= (1 << 23) && g.size() >= 64 && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_div_series(this->data(), this->size(), g.data(), g.size(), r, deg);
				return r;
			}
		}
#endif
		F r = multiply(pre(deg), g.inv(deg)).pre(deg);
		r.resize(deg);
		return r;
	}

	F inv(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).inv(deg));
		else {
			if (deg < 0) deg = static_cast<int>(this->size());
			assert(deg >= 0);
			if (deg == 0) return {};
			assert(!this->empty() && (*this)[0] != T(0));
			auto terms = Sparse::collect(this->data(), this->size(), deg, 32);
			int stride = 0;
			for (auto term : terms) stride = std::gcd(stride, term.first);
			if (terms.size() <= 32 && (deg <= 256 || terms.size() <= 12ULL * stride)) {
				const T inverse = (*this)[0].inv();
				for (auto &[j, x] : terms) x *= inverse;
				return F(Sparse::get<Sparse::Kind::inverse>(std::move(terms), deg, inverse, T(0), nullptr));
			}

#if CP_GCC_LINUX64
			if (deg >= 512 && deg <= (1 << 23) && NTT::vectorized()) {
				F result;
				const int block = 1 << (std::bit_width(static_cast<unsigned>(deg)) - 1), tail = deg - block;
				if (tail > 0 && tail <= 96) {
					fps_detail::prepare_fps_output(result, deg);
					fps_detail::blocked_poly::native_inverse(this->data(), this->size(), result, block);
					result.resize(deg);
					for (int i = block; i < deg; ++i)
						result[i] = -result[0] * dot(this->data() + 1, result.data() + i - 1, std::min<int>(i, this->size() - 1));
					return result;
				}
				fps_detail::blocked_poly::native_inverse(this->data(), this->size(), result, deg);
				return result;
			}
#endif

			F g{(*this)[0].inv()};
			g.resize(std::min(deg, 32));
			for (int i = 1; i < static_cast<int>(g.size()); ++i)
				g[i] =
					-g[0] * dot(this->data() + 1, g.data() + i - 1, std::min<int>(i, this->size() - 1));
#if CP_X86
			fps_detail::FpsWords storage;
			Word *a = nullptr, *b = nullptr;
			if (deg > 96 && NTT::vectorized()) {
				int capacity = 64;
				while (capacity < deg && capacity < (1 << 23)) capacity <<= 1;
				storage = fps_detail::allocate_fps_words(2 * capacity);
				a = storage.get();
				b = a + capacity;
				g.reserve(capacity);
				transform(std::max(32, capacity / 16));
			}
#endif
			while (static_cast<int>(g.size()) < deg) {
				const int m = static_cast<int>(g.size()), n = 2 * m;
				if (m >= 32 && (deg - m <= 32 || (m >= 512 && deg - m <= 96))) {
					g.resize(deg);
					for (int i = m; i < deg; ++i) {
						const int count = std::min<int>(i, this->size() - 1);
						g[i] = -g[0] * dot(this->data() + 1, g.data() + i - 1, count);
					}
					break;
				}
#if CP_X86
				if (m >= 64 && n <= (1 << 23) && NTT::vectorized()) {
					int lo = std::min<int>(n, this->size());
					std::copy_n(words(*this), lo, a);
					std::fill(a + lo, a + n, Word(0));
					std::copy_n(words(g), m, b);
					transform(std::max(1, n / 16)).inverse_newton8(a, b, n);
					g.resize(n);
					std::copy_n(a + m, m, words(g) + m);

				} else
#endif
				{
					base f = this->pre(n), r = g;
					f.resize(n);
					r.resize(n);
					ntt(f);
					ntt_half(r);
					pointwise(f, f, r, n);
					ntt_inverse(f);
					std::fill(f.begin(), f.begin() + m, T(0));
					ntt(f);
					pointwise(f, f, r, n);
					ntt_inverse(f);
					const T inv = T::raw(T::mod() - (T::mod() - 1) / n);
					const T scale = -inv * inv;
					g.resize(n);
					transform(1).scale(words(f) + m, m, scale.val());
					std::copy_n(f.begin() + m, m, g.begin() + m);
				}
			}
			g.resize(deg);
			return g;
		}
	}

	F sqrt(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).sqrt(deg));
		else {
			if (deg < 0) deg = static_cast<int>(this->size());
			assert(deg >= 0);
			int last = static_cast<int>(std::min<std::size_t>(deg, this->size())), first = 0;
			while (first < last && (*this)[first] == T(0)) ++first;
			if (first == last) return F(deg);
			if (first & 1) return {};
			const T constant = mod_sqrt((*this)[first]);
			if (constant == T(0)) return {};
			while ((*this)[last - 1] == T(0)) --last;
			if (first + 1 == last) {
				F result(deg);
				result[first / 2] = constant;
				return result;
			}
			const int shift = first / 2, target = deg - shift;
			if (target >= 256 && unsigned(target) < T::mod()) {
				auto terms = Sparse::collect(this->data() + first, last - first, target);
				if (terms.size() <= 12) {
					const T inverse = (*this)[first].inv();
					for (auto &[j, x] : terms) x *= inverse;
					F result(Sparse::get<Sparse::Kind::power>(std::move(terms), target, constant, T(2).inv(), inv_table));
					result.insert(result.begin(), shift, T(0));
					return result;
				}
			}
#if CP_GCC_LINUX64
			if (target >= 256 && target <= (1 << 23) && NTT::vectorized()) {
				F result;
				fps_detail::prepare_fps_output(result, deg);
				result.resize(shift);
				const int block = std::bit_floor(unsigned(target)), tail = target - block;
				const int work = block >= 512 && tail <= 8 ? block : target;
				fps_detail::blocked_poly::native_sqrt(this->data() + first, last - first, result, work, constant);
				if (work < target) {
					result.resize(deg);
					const T factor = (constant * T(2)).inv();
					for (int i = work; i < target; ++i)
						result[shift + i] = ((first + i < last ? (*this)[first + i] : T(0)) - dot(result.data() + shift + 1, result.data() + shift + i - 1, i - 1)) * factor;
				}
				return result;
			}
#endif
			F f(this->begin() + first, this->begin() + last);
			f.resize(target);
			const T half = T(2).inv(), scale = half / constant;
			F g(std::min(target, 32));
			g[0] = constant;
			for (int i = 1; i < static_cast<int>(g.size()); ++i) {
				T sum = 0;
				for (int j = 1; j < i; ++j) sum += g[j] * g[i - j];
				g[i] = (f[i] - sum) * scale;
			}
			F h;
			base cache;
			if (static_cast<int>(g.size()) < target) {
				h = g.pre(16).inv(16);
				cache = h;
				cache.resize(32);
				ntt(cache);
			}
			while (static_cast<int>(g.size()) < target) {
				const int m = static_cast<int>(g.size()), n = 2 * m;
				if (target - m <= 32) {
					g.resize(target);
					for (int i = m; i < target; ++i)
						g[i] = (f[i] - dot(g.data() + 1, g.data() + i - 1, i - 1)) * scale;
					break;
				}
				assert(n <= (1 << 23));
				const T in = T::raw(T::mod() - (T::mod() - 1) / n);
				base a = g, error(n);
				a.resize(n);
				ntt(a);
				inverse_extend(h, a, cache);
				cache = h;
				cache.resize(n);
				ntt(cache);
				pointwise(error, a, a, n);
				ntt_inverse(error);
				std::fill(error.begin(), error.begin() + m, T(0));
				for (int i = m; i < n; ++i) error[i] = (i < target ? f[i] : T(0)) - error[i] * in;
				ntt(error);
				pointwise(error, error, cache, n);
				F::scale_words(error, half);
				ntt_inverse(error);
				g.resize(std::min(n, target));
				for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
			}
			g.resize(target);
			g.insert(g.begin(), shift, T(0));
			return g;
		}
	}

	[[nodiscard]] T eval(T x) const {
		if constexpr (std::is_same_v<T, mint>) return fps_detail::horner(this->data(), this->size(), x);
		T ans = 0;
		for (auto it = this->rbegin(); it != this->rend(); ++it) ans = ans * x + *it;
		return ans;
	}

	void onemul(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this *= T(1) + c;
			return;
		}
		for (int i = (static_cast<int>(this->size())) - 1; i >= d; --i) (*this)[i] += (*this)[i - d] * c;
	}
	void onediv(int d, T c) {
		assert(d >= 0);
		if (d == 0) {
			*this /= T(1) + c;
			return;
		}
		for (int i = d; i < static_cast<int>(this->size()); ++i) (*this)[i] -= (*this)[i - d] * c;
	}

	F diff() const {
		const int n = static_cast<int>(this->size());
		F r(n);
		if constexpr (std::is_same_v<T, mint>) {
			if (n > 1) NTT::weighted(words(r), words(*this) + 1, n - 1, 1);
		} else
			for (int i = 1; i < n; ++i) r[i - 1] = (*this)[i] * i;
		return r;
	}
	F integral() const {
		const int n = static_cast<int>(this->size());
		assert(unsigned(n) < unsigned(T::mod()));
		F r(n);
		const auto &inv = inverses(n);
		if constexpr (std::is_same_v<T, mint>) {
			if (n > 1) NTT::product(words(r) + 1, words(*this), words(inv) + 1, n - 1);
		} else
			for (int i = 1; i < n; ++i) r[i] = (*this)[i - 1] * inv[i];
		return r;
	}

	F log(int deg = -1) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).log(deg));
		else {
			if (deg < 0) deg = static_cast<int>(this->size());
			assert(deg >= 0);
			if (deg == 0) return {};
			assert(!this->empty() && (*this)[0] == T(1));
			if (deg >= 256 && unsigned(deg) < T::mod()) {
				auto terms = Sparse::collect(this->data(), this->size(), deg);
				if (terms.size() <= 12) return F(Sparse::get<Sparse::Kind::logarithm>(std::move(terms), deg, T(0), T(0), inv_table));
			}
			return log_derivative(deg, true);
		}
	}

	F exp(int deg = -1) const {
		if constexpr (std::is_same_v<T, mint>) return exp_impl<false>(deg);
		else return export_work(import_work(*this).exp(deg));
	}

	F pow(long long k) const {
		if constexpr (!std::is_same_v<T, mint>) return export_work(import_work(*this).pow(k));
		else {
			assert(k >= 0);
			const int n = static_cast<int>(this->size());
			if (n == 0) return {};
			if (k == 0) {
				F r(n);
				r[0] = 1;
				return r;
			}
			if (k == 1) return *this;
			int first = 0;
			while (first < n && (*this)[first] == T(0)) ++first;
			if (first == n || first > (n - 1) / k) return F(n);
			const int shift = first * k;
			const int len = n - shift;
			const T lead = (*this)[first];
			if (unsigned(len) < T::mod()) {
				auto terms = Sparse::collect(this->data() + first, this->size() - first, len);
				if (terms.size() <= 12) {
					const T inverse = lead.inv();
					for (auto &[j, x] : terms) x *= inverse;
					F result(Sparse::get<Sparse::Kind::power>(std::move(terms), len, lead.pow(k), T(k), inv_table));
					result.insert(result.begin(), shift, T(0));
					return result;
				}
			}
			if (k <= 4) {
				F square = multiply(*this, *this).pre(n);
				return k == 2 ? square : multiply(square, k == 3 ? *this : square).pre(n);
			}

#if CP_GCC_LINUX64
			if (len >= 256 && len <= (1 << 24) && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_power(this->data() + first, this->size() - first, r, len, T(k).val(), lead.pow(k), shift);
				return r;
			}
#endif
			F f(this->begin() + first, this->end());
			f.resize(len);
			f = f.log_derivative(len);
			f *= T(k);
			f = f.template exp_impl<true>(len);
			f *= lead.pow(k);
			F r(n);
			for (int i = 0; i < len; ++i) r[shift + i] = f[i];
			return r;
		}
	}

	[[nodiscard]] std::pair<F, F> divmod(const F &divisor) const {
		if constexpr (!std::is_same_v<T, mint>) {
			auto [q, r] = import_work(*this).divmod(import_work(divisor));
			return {export_work(q), export_work(r)};
		}
		int n = static_cast<int>(this->size()), m = static_cast<int>(divisor.size());
		while (n && (*this)[n - 1] == T(0)) --n;
		while (m && divisor[m - 1] == T(0)) --m;
		assert(m);
		if (n < m) return {{}, pre(n)};
		const int qn = n - m + 1;

#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (m > 2 && (std::size_t)m * qn > 4096 && n <= (1 << 23) && NTT::vectorized()) {
				F q, r;
				fps_detail::blocked_poly::native_divide(this->data(), n, divisor.data(), m, q, r);
				q.trim(), r.trim();
				return {std::move(q), std::move(r)};
			}
		}
#endif
		F a = pre(n);
		if (m <= 2 || (std::size_t)m * qn <= 4096) {
			F q(qn);
			const T inv = divisor[m - 1].inv();
			for (int i = qn - 1; i >= 0; --i) {
				q[i] = a[i + m - 1] * inv;
				for (int j = 0; j < m; ++j) a[i + j] -= q[i] * divisor[j];
			}
			a.resize(m - 1);
			a.trim();
			q.trim();
			return {std::move(q), std::move(a)};
		}
		F ra = a, rb(divisor.begin(), divisor.begin() + m);
		std::reverse(ra.begin(), ra.end());
		std::reverse(rb.begin(), rb.end());
		F q = ra.pre(qn).div_series(rb, qn);
		std::reverse(q.begin(), q.end());
		std::reverse(rb.begin(), rb.end());
		F product = multiply(rb.pre(m - 1), q.pre(m - 1));
		a.resize(m - 1);
		for (int i = 0; i < std::min<int>(a.size(), product.size()); ++i) a[i] -= product[i];
		a.trim();
		q.trim();
		return {std::move(q), std::move(a)};
	}

	private:
	using O = fps_detail::Operations<T>;
	using O::import_work, O::export_work, O::transform, O::words, O::transform_inplace, O::ntt, O::ntt_half, O::ntt_inverse, O::scale_words, O::geometric_scale, O::batch_inverse, O::pointwise, O::inverse_extend, O::dot, O::intt, O::mod_sqrt, O::inverse_ntt_roots, O::ntt_roots, O::inverses;

	using NTT = fps_detail::NTT;
	using Word = typename O::Word;

	F log_derivative(int deg, bool integrate = false) const {
#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 256 && deg <= (1 << 24) && this->size() >= 64 && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_log(this->data(), this->size(), r, deg, integrate);
				return r;
			}
		}
#endif

		F derivative(std::min<int>(deg, this->size()));
		if (derivative.size() > 1) NTT::weighted(words(derivative) + 1, words(*this) + 1, derivative.size() - 1, 1);
		F r = derivative.div_series(*this, deg);
		if (integrate) {
			const auto &iv = inverses(deg - 1);
			NTT::product(words(r), words(r), words(iv), deg);
		}
		return r;
	}

	template <bool differentiated>
	F exp_impl(int deg) const {
		if (deg < 0) deg = static_cast<int>(this->size());
		assert(deg >= 0);
		if (deg == 0) return {};
		assert(this->empty() || (*this)[0] == T(0));
		if constexpr (std::is_same_v<T, mint>) {
			auto terms = Sparse::collect(this->data(), this->size(), deg, 64);
			int stride = 0;
			for (auto term : terms) stride = std::gcd(stride, term.first);
			if (terms.size() <= 64 && (deg <= 256 || terms.size() <= 24ULL * stride)) {
				if constexpr (differentiated) for (auto &[j, x] : terms) x /= T(j);
				return F(Sparse::get<Sparse::Kind::exponential>(std::move(terms), deg, T(1), T(0), inv_table));
			}
		}

#if CP_GCC_LINUX64
		if constexpr (std::is_same_v<T, mint>) {
			if (deg >= 256 && deg <= (1 << 24) && NTT::vectorized()) {
				F r;
				fps_detail::blocked_poly::native_exp(this->data(), this->size(), r, deg, differentiated);
				return r;
			}
		}
#endif
		const auto &iv = inverses(deg);
		const int initial = std::min(deg, 32);
		F g(initial);
		g[0] = 1;
		for (int i = 1; i < initial; ++i) {
			T sum = 0;
			for (int j = 1; j <= i && j < static_cast<int>(this->size()); ++j)
				sum += (differentiated ? (*this)[j] : T(j) * (*this)[j]) * g[i - j];
			g[i] = sum * iv[i];
		}
		if (initial == deg) return g;
		F h = g.pre(16).inv(16);
		base cache = h;
		cache.resize(32);
		ntt(cache);
		while (static_cast<int>(g.size()) < deg) {
			const int m = static_cast<int>(g.size()), n = 2 * m;
			if (deg - m <= 32) {
				base derivative(std::min<int>(deg, this->size()));
				for (int i = 1; i < static_cast<int>(derivative.size()); ++i) derivative[i] = differentiated ? (*this)[i] : T(i) * (*this)[i];
				derivative.resize(std::max<int>(1, derivative.size()));
				g.resize(deg);
				for (int i = m; i < deg; ++i) {
					const int count = std::min<int>(i, derivative.size() - 1);
					g[i] = dot(derivative.data() + 1, g.data() + i - 1, count) * iv[i];
				}
				break;
			}
			assert(n <= (1 << 23));
			const T im = T::raw(T::mod() - (T::mod() - 1) / m);
			const T in = T::raw(T::mod() - (T::mod() - 1) / n);
			base a = g, b(m), error(n);
			a.resize(n);
			ntt(a);
			inverse_extend(h, a, cache);
			cache = h;
			cache.resize(n);
			ntt(cache);
			std::fill(b.begin(), b.end(), T(0));
			for (int i = 1; i < m && i < static_cast<int>(this->size()); ++i) b[i - 1] = differentiated ? (*this)[i] : T(i) * (*this)[i];
			ntt(b);
			pointwise(b, b, a, m);
			ntt_inverse(b);
			error[m - 1] = b[m - 1] * im;
			for (int i = 0; i < m - 1; ++i) error[m + i] = b[i] * im - T(i + 1) * g[i + 1];
			ntt(error);
			pointwise(error, error, cache, n);
			ntt_inverse(error);
			for (int i = (std::min(n, deg)) - 1; i >= m; --i)
				error[i] = (i < static_cast<int>(this->size()) ? (*this)[i] * (differentiated ? iv[i] : T(1)) : T(0)) + error[i - 1] * in * iv[i];
			std::fill(error.begin(), error.begin() + m, T(0));
			std::fill(error.begin() + std::min(n, deg), error.end(), T(0));
			ntt(error);
			pointwise(error, error, a, n);
			ntt_inverse(error);
			g.resize(std::min(n, deg));
			for (int i = m; i < static_cast<int>(g.size()); ++i) g[i] = error[i] * in;
		}
		return g;
	}

	static const mint *inv_table(int n) { return fps_detail::Operations<mint>::inverses(n).data(); }

};

using fps = FormalPowerSeries<>;

#endif

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 1 "components/mint_factorials.hpp"



#line 5 "components/mint_factorials.hpp"

namespace mint_detail {

#if CP_X86

__attribute__((target("avx2"))) inline void factorial_transpose(Ops::I8 (&x)[8]) {
	Ops::I8 a[8], b[8];
	for (int i = 0; i < 8; i += 2) {
		a[i] = _mm256_unpacklo_epi32(x[i], x[i + 1]);
		a[i + 1] = _mm256_unpackhi_epi32(x[i], x[i + 1]);
	}
	for (int i = 0; i < 8; i += 4) {
		b[i] = _mm256_unpacklo_epi64(a[i], a[i + 2]);
		b[i + 1] = _mm256_unpackhi_epi64(a[i], a[i + 2]);
		b[i + 2] = _mm256_unpacklo_epi64(a[i + 1], a[i + 3]);
		b[i + 3] = _mm256_unpackhi_epi64(a[i + 1], a[i + 3]);
	}
	for (int i = 0; i < 4; ++i) {
		x[i] = _mm256_permute2x128_si256(b[i], b[i + 4], 0x20);
		x[i + 4] = _mm256_permute2x128_si256(b[i], b[i + 4], 0x31);
	}
}

template<bool reverse>
__attribute__((target("avx2"))) inline void factorial_chain(Ops::U* out, int chunk, Ops::I8 value, Ops::I8 factor) {
	using I = Ops::I8;
	I step = _mm256_set1_epi32(Ops::one);
	I order = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
	for (int j = 0; j < chunk; j += 8) {
		I block[8];
		for (int k = 0; k < 8; ++k) {
			block[k] = value;
			value = Ops::mul8(value, factor);
			if constexpr (reverse) factor = Ops::sub8(factor, step);
			else factor = Ops::add8(factor, step);
		}
		factorial_transpose(block);
		for (int k = 0; k < 8; ++k) {
			if constexpr (reverse) Ops::store8(out + (k + 1) * chunk - j - 8, _mm256_permutevar8x32_epi32(block[k], order));
			else Ops::store8(out + k * chunk + j, block[k]);
		}
	}
}

__attribute__((target("avx2"))) inline void factorials8(Ops::U* fact, Ops::U* invfact, int n) {
	using I = Ops::I8;
	using U = Ops::U;
	const int chunk = n / 256 * 8, covered = chunk * 32;
	alignas(32) U factor[32], products[32], starts[33], inverse[33];
	I p[4], x[4], step = _mm256_set1_epi32(Ops::one);
	for (int i = 0; i < 32; ++i) factor[i] = Ops::in(i * chunk + 1);
	for (int k = 0; k < 4; ++k) p[k] = _mm256_set1_epi32(Ops::one), x[k] = Ops::load8(factor + 8 * k);
	for (int i = 0; i < chunk; ++i) {
		for (int k = 0; k < 4; ++k) p[k] = Ops::mul8(p[k], x[k]), x[k] = Ops::add8(x[k], step);
	}
	for (int k = 0; k < 4; ++k) Ops::store8(products + 8 * k, p[k]);
	starts[0] = Ops::one;
	for (int i = 0; i < 32; ++i) starts[i + 1] = Ops::mul(starts[i], products[i]);
	U last = starts[32];
	for (int i = covered; i < n; ++i) fact[i] = last, last = Ops::mul(last, Ops::in(i + 1));
	last = mint::word(last).inv().v;
	for (int i = n - 1; i >= covered; --i) last = Ops::mul(last, Ops::in(i + 1)), invfact[i] = last;
	inverse[32] = last;
	for (int i = 31; i >= 0; --i) inverse[i] = Ops::mul(inverse[i + 1], products[i]);
	for (int k = 0; k < 4; ++k) {
		factorial_chain<false>(fact + 8 * k * chunk, chunk, Ops::load8(starts + 8 * k), Ops::load8(factor + 8 * k));
		alignas(32) U values[8], factors[8];
		for (int j = 0; j < 8; ++j) {
			const int end = (8 * k + j + 1) * chunk;
			values[j] = Ops::mul(inverse[8 * k + j + 1], Ops::in(end));
			factors[j] = Ops::in(end - 1);
		}
		factorial_chain<true>(invfact + 8 * k * chunk, chunk, Ops::load8(values), Ops::load8(factors));
	}
}
#endif

inline void factorials(Ops::U* fact, Ops::U* invfact, int n) {
	if (!n) return;
#if CP_X86
	if (n >= 256 && Ops::vectorized()) {
		factorials8(fact, invfact, n);
		return;
	}
#endif
	Ops::U value = Ops::one;
	for (int i = 0; i < n; ++i) fact[i] = value, value = Ops::mul(value, Ops::in(i + 1));
	value = mint::word(fact[n - 1]).inv().v;
	for (int i = n - 1; i >= 0; --i) invfact[i] = value, value = Ops::mul(value, Ops::in(i));
}

template<class T> requires std::is_same_v<T, mint>
inline void factorials(T* fact, T* invfact, int n) {
	assert(0 <= n && unsigned(n) <= mint::mod());
	static_assert(sizeof(mint) == sizeof(Ops::U) && std::is_trivially_copyable_v<mint>);
	if (unsigned(n) == mint::mod()) {
		factorials(fact, invfact, n - 1);
		fact[n - 1] = -fact[n - 2];
		invfact[n - 1] = -invfact[n - 2];
		return;
	}
	factorials(reinterpret_cast<Ops::U*>(fact), reinterpret_cast<Ops::U*>(invfact), n);
}

}


#line 1 "components/fps_taylor_blocked.hpp"



#line 6 "components/fps_taylor_blocked.hpp"

#if CP_GCC_LINUX64
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace taylor_factorials {
inline constexpr Word kStep = 1024;
template <int Block>
inline constexpr auto factorial_block = [] {
	std::array<Word, 16> result{};
	std::uint64_t value = 1;
	if constexpr (Block > 0) value = factorial_block<Block - 1>.back();
	for (int i = 0; i < 16; ++i) {
		for (Word j = 1; j <= kStep; ++j) value = value * ((Block * 16 + i) * kStep + j) % 998244353;
		result[i] = Word(value);
	}
	return result;
}();
inline constexpr auto kTable = []<size_t... I>(std::index_sequence<I...>) {
	return std::array<Word, 513>{1, factorial_block<I / 16>[I % 16]...};
}(std::make_index_sequence<512>{});
}
namespace taylor_detail {
namespace ntt = multiply_detail::ntt;
using ntt::detail::add, ntt::detail::broadcast, ntt::detail::diff, ntt::detail::kP, ntt::detail::kR, ntt::detail::multiply_mod, ntt::detail::power, ntt::detail::reduce, ntt::detail::slot, multiply_detail::Subtrees, ntt::detail::Vec, blocked_poly::detail::load, blocked_poly::detail::low, blocked_poly::detail::store;
inline Word inverse(Word x) { return power(x, kP - 2); }
inline Word mont(Word x) { return multiply_mod(x, kR); }
inline Vec odd_lanes(Vec x) { return _mm256_srli_epi64(x, 32); }
[[gnu::always_inline]] inline Vec montgomery(Vec a, Vec a_odd, Vec b, Vec b_odd) {
	Vec even = _mm256_mul_epu32(a, b), odd = _mm256_mul_epu32(a_odd, b_odd);
	return mint_detail::Ops::reduce8(even, odd);
}
[[gnu::always_inline]] inline void transpose(Vec (&r)[8]) {
	Vec t[8], u[8];
#pragma GCC unroll 4
	for (int i = 0; i < 8; i += 2) {
		t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]);
		t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
	}
#pragma GCC unroll 2
	for (int i = 0; i < 8; i += 4) {
		u[i] = _mm256_unpacklo_epi64(t[i], t[i + 2]);
		u[i + 1] = _mm256_unpackhi_epi64(t[i], t[i + 2]);
		u[i + 2] = _mm256_unpacklo_epi64(t[i + 1], t[i + 3]);
		u[i + 3] = _mm256_unpackhi_epi64(t[i + 1], t[i + 3]);
	}
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i) {
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20);
		r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
	}
}
constexpr int kVectors = 4, kLanes = 8 * kVectors;
template <class F>
void fill(Vec (&x)[kVectors], F f) {
	alignas(32) Word values[kLanes];
	for (int s = 0; s < kLanes; ++s) values[s] = f(s);
	for (int v = 0; v < kVectors; ++v) x[v] = load(values + 8 * v);
}
using Block = Vec[kVectors][8];
struct Chain {
	Vec x[kVectors], base[kVectors], base_odd[kVectors], u, step;
	template <class X, class Base>
	Chain(X initial, Base base_of, Word step_value) : u(_mm256_setzero_si256()), step(broadcast(step_value)) {
		fill(x, initial);
		fill(base, base_of);
		for (int v = 0; v < kVectors; ++v) base_odd[v] = odd_lanes(base[v]);
	}
	[[gnu::always_inline]] void advance(Block& block, int t) {
#pragma GCC unroll 4
		for (int v = 0; v < kVectors; ++v) {
			block[v][t] = x[v];
			x[v] = montgomery(x[v], odd_lanes(x[v]), add(base[v], u), add(base_odd[v], u));
		}
		u = reduce(add(u, step), kP);
	}
};
class TaylorShift {
public:
	explicit TaylorShift(size_t n) : n_(n) {
		lg_ = std::max(10, int(std::bit_width(2 * n - 1)));
		lg_ += lg_ % 2;
		chunk_ = ((n + kLanes - 1) / kLanes + 15) / 16 * 16;
		if (chunk_ / 16 % 2 == 0) chunk_ += 16;
		const size_t len = length(), tables = ntt::detail::table_words(lg_);
		const size_t words = 2 * len + 3 * kPadding + 2 * tables;
		storage_ = allocate_fps_words(words);
		if (!storage_.get_deleter().region) std::fill_n(storage_.get(), words, 0u);
		b_ = storage_.get() + kPadding;
		a_ = b_ + len + kPadding;
		roots_ = a_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
	}
	Word* coefficients() { return a_; }
	const Word* shift(Word c) {
		using namespace ntt::detail;
		const size_t len = length(), q = len / 64;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		lane_factorials();
		weights(c);
		auto* av = reinterpret_cast<Vec*>(a_);
		auto* bv = reinterpret_cast<Vec*>(b_);
		multiply_detail::forward_radix8(av, q, roots_);
		multiply_detail::forward_radix8(bv, q, roots_);
		for (size_t t = 0; t < 8; ++t) {
			Word& x = b_[8 * q * t];
			x = (x % kP + (t < 4 ? 1 : kP - 1)) % kP;
		}
		const Subtrees subtrees(roots_, inverse_roots_);
		for (size_t t = 0; t < 4; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse_identity(av, q, inverse_roots_);
		for (size_t t = 4; t < 8; ++t) subtrees.visit(av + t * q, bv + t * q, q, t);
		ntt::kernels::inverse(av + 4 * q, q, inverse_roots_ + slot(1), inverse_roots_ + slot(2));
		output();
		return b_;
	}
private:
	static constexpr size_t kPadding = 1040;
	size_t length() const { return size_t(1) << lg_; }
	size_t start(int s) const { return s * chunk_; }
	void lane_factorials() {
		using taylor_factorials::kStep;
		const auto below = [this](int s) { return Word(start(s + 1) / kStep * kStep); };
		Vec x[kVectors], m[kVectors], count[kVectors];
		fill(x, [&](int s) { return mont(taylor_factorials::kTable[below(s) / kStep]); });
		fill(m, [&](int s) { return mont(below(s) + 1); });
		fill(count, [&](int s) { return Word(start(s + 1) - below(s)); });
		const Vec unit = broadcast(kR);
		for (Word j = 0; j < kStep - 1; ++j) {
			const Vec step = broadcast(j);
#pragma GCC unroll 4
			for (int v = 0; v < kVectors; ++v) {
				const Vec factor = _mm256_blendv_epi8(unit, m[v], _mm256_cmpgt_epi32(count[v], step));
				x[v] = montgomery(x[v], odd_lanes(x[v]), factor, odd_lanes(factor));
				m[v] = reduce(add(m[v], unit), kP);
			}
		}
		alignas(32) Word lane_x[kLanes];
		for (int v = 0; v < kVectors; ++v) store(lane_x + 8 * v, x[v]);
		const Word unit_inverse = inverse(kR);
		factorial_[0] = 1;
		for (int s = 0; s < kLanes; ++s) factorial_[s + 1] = multiply_mod(lane_x[s] % kP, unit_inverse);
		Word f = 1;
		for (int s = 0; s <= kLanes; ++s) inverse_start_[s] = f, f = multiply_mod(f, factorial_[s]);
		f = inverse(f);
		for (int s = kLanes; s >= 0; --s)
			inverse_start_[s] = multiply_mod(inverse_start_[s], f), f = multiply_mod(f, factorial_[s]);
	}
	void weights(Word c) {
		const Word c_chunk = power(c, Word(chunk_)), c_step = mont(inverse(c));
		std::array<Word, kLanes> c_power;
		for (int s = 0; s < kLanes; ++s) c_power[s] = s ? multiply_mod(c_power[s - 1], c_chunk) : c_chunk;
		Chain input([this](int s) { return mont(factorial_[s]); },
					[this](int s) { return mont(Word(start(s) + 1)); }, kR);
		Chain kernel([&](int s) { return multiply_mod(c_power[s], inverse_start_[s + 1]); },
					[&](int s) { return multiply_mod(Word(start(s + 1)), c_step); }, kP - c_step);
		const size_t stride = chunk_;
		Block forward[2], backward[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) input.advance(forward[0], t), kernel.advance(backward[0], t);
		for (size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			Word* const a = a_ + j;
			Word* const e = b_ + length() / 2 - stride + j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				input.advance(forward[cur ^ 1], t);
				kernel.advance(backward[cur ^ 1], t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(forward[cur][v]), transpose(backward[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const size_t at = (8 * v + l) * stride;
					const Vec f = load(a + at), x = forward[cur][v][l];
					store(a + at, reduce(montgomery(f, odd_lanes(f), x, odd_lanes(x)), kP));
					store(e - at, reduce(backward[cur][v][l], kP));
				}
			}
		}
	}
	void output() {
		const Word scale = multiply_mod(mont(inverse(Word(length() / 8))), kR);
		Chain chain([&](int s) { return multiply_mod(multiply_mod(inverse_start_[s + 1], Word(start(s + 1))), scale); },
					[this](int s) { return mont(Word(start(s + 1) - 1)); }, kP - kR);
		const size_t stride = chunk_, half = length() / 2;
		Block block[2];
#pragma GCC unroll 8
		for (int t = 0; t < 8; ++t) chain.advance(block[0], 7 - t);
		for (size_t j = 0, cur = 0; j < stride; j += 8, cur ^= 1) {
			const Word* const u = a_ + stride - 8 - j;
			Word* const out = b_ + stride - 8 - j;
#pragma GCC unroll 8
			for (int t = 0; t < 8; ++t) {
				chain.advance(block[cur ^ 1], 7 - t);
				const int v = kVectors * t / 8, first = kVectors * t % 8;
				if (first == 0) transpose(block[cur][v]);
#pragma GCC unroll 4
				for (int l = first; l < first + kVectors; ++l) {
					const size_t at = (8 * v + l) * stride;
					const Vec g = low(diff(load(u + at), load(u + at + half))), z = block[cur][v][l];
					store(out + at, reduce(montgomery(g, odd_lanes(g), z, odd_lanes(z)), kP));
				}
			}
		}
	}
	size_t n_, chunk_;
	int lg_;
	Word *a_, *b_, *roots_, *inverse_roots_;
	FpsWords storage_;
	std::array<Word, kLanes + 1> factorial_, inverse_start_;
};
inline void shift(std::vector<mint> &f, mint c) {
	TaylorShift t(f.size());
	size_t i = 0;
	for (; i + 8 <= f.size(); i += 8) {
		const auto x = blocked_poly::detail::load_unaligned(reinterpret_cast<const unsigned *>(f.data() + i));
		store(t.coefficients() + i, reduce(x, kP));
	}
	for (; i < f.size(); ++i) t.coefficients()[i] = std::min(f[i].v, f[i].v - kP);
	const auto *out = t.shift(c.val());
	i = 0;
	for (; i + 8 <= f.size(); i += 8)
		blocked_poly::detail::store_unaligned(reinterpret_cast<unsigned *>(f.data() + i), load(out + i));
	for (; i < f.size(); ++i) f[i] = mint::word(out[i]);
}
}
}

#pragma GCC pop_options
#endif

#line 7 "fps/taylor_shift.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_SHIFT_API
#define CP_FPS_SHIFT_API 1
template<class T>
FormalPowerSeries<T> taylor_shift(FormalPowerSeries<T> f, std::type_identity_t<T> c) {
	using F = FormalPowerSeries<T>;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, mint>)
		return O::export_work(taylor_shift<mint>(O::import_work(f), mint::raw(c.val())));
	else {
		const int n = static_cast<int>(f.size());
		if (n <= 1 || c == T(0)) return f;
		assert(unsigned(n) < unsigned(T::mod()));
#if CP_GCC_LINUX64
		if (n >= 256 && n <= (1 << 19) && fps_detail::NTT::vectorized()) {
			fps_detail::taylor_detail::shift(f, c);
			return f;
		}
#endif
		F a(n), b(n), weights;
		mint_detail::factorials(O::words(a), O::words(b), n);
		fps_detail::NTT::product(O::words(a), O::words(a), O::words(f), n);
		std::reverse(a.begin(), a.end());
		weights = b;
		O::geometric_scale(b, T(1), c);
		F prod = F::multiply(a, b);
		std::reverse(prod.begin(), prod.begin() + n);
		fps_detail::NTT::product(O::words(f), O::words(prod), O::words(weights), n);
		return f;
	}
}

#endif

#if CP_GCC_X86
#pragma GCC pop_options
#endif


#line 1 "test/fixtures/fps_bulk_io.hpp"



#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#line 15 "components/fast_io.hpp"
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
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


#line 1 "components/fps_io.hpp"



#line 1 "components/fast_io_bulk.hpp"



#line 5 "components/fast_io_bulk.hpp"

#line 1 "components/fast_io_output.hpp"



#line 7 "components/fast_io_output.hpp"

namespace fast_io_detail {
struct BulkOutput {
	static char *small(char *out, std::uint32_t value) {
		unsigned n = 1 + (value >= 10) + (value >= 100) + (value >= 1000);
		if constexpr (std::endian::native == std::endian::little) {
			std::uint32_t word;
			std::memcpy(&word, FastOutput::quads[value].data(), 4);
			word >>= (4 - n) * 8;
			std::memcpy(out, &word, 4);
		} else std::memcpy(out, FastOutput::quads[value].data() + 4 - n, n);
		return out + n;
	}
	static char *number(char *out, std::uint32_t value) {
		if (value >= 100000000) {
			out = small(out, value / 100000000);
			value %= 100000000;
			std::memcpy(out, FastOutput::quads[value / 10000].data(), 4);
			std::memcpy(out + 4, FastOutput::quads[value % 10000].data(), 4);
			return out + 8;
		}
		if (value >= 10000) {
			out = small(out, value / 10000);
			std::memcpy(out, FastOutput::quads[value % 10000].data(), 4);
			return out + 4;
		}
		return small(out, value);
	}
	template<std::size_t Fields, class Read>
#if defined(__GNUC__) || defined(__clang__)
	[[gnu::noinline]]
#endif
	static void records(FastOutput &out, std::size_t count, Read read) {
		constexpr auto bytes = 11 * Fields;
		constexpr auto block = std::max<std::size_t>(1, (1 << 16) / bytes);
		for (std::size_t base = 0; base < count;) {
			const auto n = std::min(count - base, block);
			char *begin = out.reserve(n * bytes), *p = begin;
			for (std::size_t i = 0; i < n; ++i) {
				auto values = read(base + i);
				for (std::size_t j = 0; j < Fields; ++j) {
					p = number(p, std::uint32_t(values[j]));
					*p++ = j + 1 == Fields ? '\n' : ' ';
				}
			}
			out.length += p - begin;
			base += n;
		}
	}
	static void write(FastOutput &out, std::string_view text) {
		if (text.size() <= (1 << 16)) return out.write(text);
		if (out.length && !out.flush()) throw std::runtime_error("output failed");
		std::size_t done = 0;
		while (done < text.size()) {
			const auto written = std::fwrite(text.data() + done, 1, text.size() - done, out.file);
			if (!written) throw std::runtime_error("output failed");
			done += written;
		}
	}
};

}

template<std::size_t Fields, class Read>
void write_unsigned_records(FastOutput &output, std::size_t count, Read read) {
	static_assert(Fields > 0 && Fields <= std::numeric_limits<std::size_t>::max() / 11);
	fast_io_detail::BulkOutput::records<Fields>(output, count, std::move(read));
}


#line 10 "components/fast_io_bulk.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fast_io_detail {
struct BulkInput {
	alignas(32) static inline constexpr auto rows = [] {
		std::array<std::array<unsigned char, 16>, 11> r{};
		for (int n = 0; n <= 10; ++n)
			for (int i = 0; i < 16; ++i) r[n][i] = i < 16 - n ? 128 : i - (16 - n);
		return r;
	}();
	static constexpr std::size_t kChunk = 1 << 17, kCapacity = kChunk / 4 + 66;

	static unsigned separators(__m256i bytes) {
		return unsigned(_mm256_movemask_epi8(_mm256_cmpeq_epi8(_mm256_max_epu8(bytes, _mm256_set1_epi8(' ')), _mm256_set1_epi8(' '))));
	}

	template <bool Signed = false>
	[[gnu::always_inline]] static __m256i two(const char *&p, __m256i &invalid, unsigned *signs = nullptr) {
		const auto bytes = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p));
		const unsigned sep = separators(bytes), a = std::countr_zero(sep), b = std::countr_zero(sep & (sep - 1));
		unsigned n0 = 0, n1 = 0;
		if constexpr (Signed) {
			if (a < 1 || a > 11 || b <= a + 1 || b - a - 1 > 11) [[unlikely]] {
				invalid = _mm256_set1_epi32(-1);
				p += std::min(b + 1, 32u);
				*signs = 0;
				return _mm256_setzero_si256();
			}
			n0 = p[0] == '-', n1 = p[a + 1] == '-';
			*signs = n0 | (n1 << 1);
		}
		if (a < n0 + 1 || a - n0 > 10 || b <= a + n1 + 1 || b - a - n1 - 1 > 10) [[unlikely]] {
			invalid = _mm256_set1_epi32(-1);
			p += std::min(b + 1, 32u);
			return _mm256_setzero_si256();
		}
		auto window = bytes;
		if constexpr (Signed) window = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1 + n1)), _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + n0)));
		else window = _mm256_inserti128_si256(bytes, _mm_loadu_si128(reinterpret_cast<const __m128i *>(p + a + 1)), 1);
		const auto control = _mm256_set_m128i(_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[b - a - n1 - 1].data())),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[a - n0].data())));
		const auto digits = _mm256_shuffle_epi8(_mm256_sub_epi8(window, _mm256_set1_epi8('0')), control);
		invalid = _mm256_or_si256(invalid, _mm256_subs_epu8(digits, _mm256_set1_epi8(9)));
		p += b + 1;
		return _mm256_madd_epi16(_mm256_maddubs_epi16(digits, _mm256_set1_epi16(0x010a)), _mm256_set1_epi32(0x00010064));
	}

	template <bool Signed = false>
	static std::uint32_t one(const char *&p, bool &bad) {
		bool negative = false;
		if constexpr (Signed) negative = *p == '-', p += negative;
		const auto bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(p));
		const auto spaces = _mm_cmpeq_epi8(_mm_max_epu8(bytes, _mm_set1_epi8(' ')), _mm_set1_epi8(' '));
		const unsigned n = std::countr_zero(unsigned(_mm_movemask_epi8(spaces)) | 0x10000u);
		if (n < 1 || n > 10) [[unlikely]] {
			bad = true;
			p += std::min(n + 1, 16u);
			return 0;
		}
		const auto digits = _mm_shuffle_epi8(_mm_sub_epi8(bytes, _mm_set1_epi8('0')),
			_mm_loadu_si128(reinterpret_cast<const __m128i *>(rows[n].data())));
		const auto invalid = _mm_subs_epu8(digits, _mm_set1_epi8(9));
		bad |= !_mm_testz_si128(invalid, invalid);
		const auto groups = _mm_madd_epi16(_mm_maddubs_epi16(digits, _mm_set1_epi16(0x010a)), _mm_set1_epi32(0x00010064));
		const auto halves = _mm_madd_epi16(_mm_packus_epi32(groups, groups), _mm_set1_epi32(0x00012710));
		const std::uint64_t value = std::uint64_t(unsigned(_mm_cvtsi128_si32(halves))) * 100000000 + unsigned(_mm_extract_epi32(halves, 1));
		if constexpr (Signed) bad |= value > 2147483647ULL + negative;
		else bad |= value > 0xffffffffULL;
		p += n + 1;
		return negative ? 0u - std::uint32_t(value) : std::uint32_t(value);
	}

	static const char *after_separator(const char *p) {
		const unsigned mask = separators(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)));
		return mask ? p + std::countr_zero(mask) + 1 : nullptr;
	}

	template <bool Signed = false>
	static std::size_t lockstep(const char *(&streams)[4], const char *const (&end)[4], unsigned *const (&out)[4], __m256i &invalid) {
		const char *p[4] = {streams[0], streams[1], streams[2], streams[3]};
		std::size_t done = 0;
		for (;;) {
			std::ptrdiff_t left = end[0] - p[0];
			for (int k = 1; k < 4; ++k) left = std::min(left, end[k] - p[k]);
			const auto steps = std::min(left / 33, std::ptrdiff_t((kCapacity - done) / 2));
			if (steps <= 0) break;
			for (auto j = steps; j; --j, done += 2) {
				unsigned signs0 = 0, signs1 = 0, signs2 = 0, signs3 = 0;
				const auto g0 = two<Signed>(p[0], invalid, &signs0), g1 = two<Signed>(p[1], invalid, &signs1), g2 = two<Signed>(p[2], invalid, &signs2), g3 = two<Signed>(p[3], invalid, &signs3);
				const auto k = _mm256_set1_epi32(0x00012710);
				const auto h01 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g0, g1), k));
				const auto h23 = _mm256_castsi256_ps(_mm256_madd_epi16(_mm256_packus_epi32(g2, g3), k));
				const auto high = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0x88));
				const auto low = _mm256_castps_si256(_mm256_shuffle_ps(h01, h23, 0xdd));
				auto max_high = _mm256_set1_epi32(42), max_low = _mm256_set1_epi32(94967295);
				auto signs = _mm256_setzero_si256();
				if constexpr (Signed) {
					signs = _mm256_setr_epi32(signs0 & 1, signs1 & 1, signs2 & 1, signs3 & 1, signs0 >> 1, signs1 >> 1, signs2 >> 1, signs3 >> 1);
					max_high = _mm256_set1_epi32(21), max_low = _mm256_add_epi32(_mm256_set1_epi32(47483647), signs);
				}
				const auto overflow = _mm256_or_si256(_mm256_cmpgt_epi32(high, max_high),
					_mm256_and_si256(_mm256_cmpeq_epi32(high, max_high), _mm256_cmpgt_epi32(low, max_low)));
				invalid = _mm256_or_si256(invalid, overflow);
				auto value = _mm256_add_epi32(_mm256_mullo_epi32(high, _mm256_set1_epi32(100000000)), low);
				if constexpr (Signed) {
					const auto mask = _mm256_sub_epi32(_mm256_setzero_si256(), signs);
					value = _mm256_sub_epi32(_mm256_xor_si256(value, mask), mask);
				}
				const auto q = _mm256_permutevar8x32_epi32(value, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
				const auto q01 = _mm256_castsi256_si128(q), q23 = _mm256_extracti128_si256(q, 1);
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[0] + done), q01);
				_mm_storeh_pd(reinterpret_cast<double *>(out[1] + done), _mm_castsi128_pd(q01));
				_mm_storel_epi64(reinterpret_cast<__m128i *>(out[2] + done), q23);
				_mm_storeh_pd(reinterpret_cast<double *>(out[3] + done), _mm_castsi128_pd(q23));
			}
		}
		for (int k = 0; k < 4; ++k) streams[k] = p[k];
		return done;
	}

	template <bool Signed = false, class Emit>
	static std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *const (&out)[4], Emit &&emit) {
		const auto bytes = std::min({kChunk, 2 * (count - 32), std::size_t(limit - p) - 64}) & ~std::size_t(3);
		const std::size_t stream = bytes / 4;
		const char *p1 = after_separator(p + stream), *p2 = after_separator(p + 2 * stream);
		const char *p3 = after_separator(p + 3 * stream), *last = after_separator(p + bytes);
		if (!p1 || !p2 || !p3 || !last) return 0;
		const char *s[4] = {p, p1, p2, p3}, *const end[4] = {p1, p2, p3, last};
		auto invalid = _mm256_setzero_si256();
		const auto done = lockstep<Signed>(s, end, out, invalid);
		bool bad = !_mm256_testz_si256(invalid, invalid);
		std::size_t n[4];
		for (int k = 0; k < 4; ++k) {
			n[k] = done;
			while (s[k] < end[k] && n[k] < kCapacity) out[k][n[k]++] = one<Signed>(s[k], bad);
			bad |= s[k] != end[k];
		}
		const auto parsed = n[0] + n[1] + n[2] + n[3];
		if (bad || parsed > count) return 0;
		for (int k = 0; k < 4; ++k) emit(out[k], n[k]);
		p = last;
		return parsed;
	}

	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		unsigned *const out[4] = {dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity};
		bool first = true;
		unsigned *next = dst;
		return chunk_blocks(p, limit, count, out, [&](const unsigned *values, std::size_t n) {
			if (!first) std::memcpy(next, values, n * sizeof(unsigned));
			first = false;
			next += n;
		});
	}

	template <bool Signed = false, class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		bool chunks = true;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (chunks && count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				unsigned *const out[4] = {scratch.get(), scratch.get() + kCapacity, scratch.get() + 2 * kCapacity, scratch.get() + 3 * kCapacity};
				const auto parsed = chunk_blocks<Signed>(p, in.end, count - done, out, emit);
				if (!parsed) {
					chunks = false;
					break;
				}
				done += parsed;
			}
			unsigned tail[64];
			std::size_t used = 0;
			while (!Signed && done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				tail[used++] = h0 * 100000000 + l0;
				tail[used++] = h1 * 100000000 + l1;
				if (used == 64) {
					emit(tail, used);
					used = 0;
				}
				done += 2;
				p = next;
			}
			if (used) emit(tail, used);
			in.current = p;
			if (done < count) {
				std::conditional_t<Signed, std::int32_t, unsigned> value = 0;
				in.read(value);
				emit(reinterpret_cast<unsigned *>(&value), 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}

};
}
#pragma GCC pop_options
#endif


namespace fast_io_detail {
template <class Emit, class T, std::size_t... I>
void emit_record(Emit &emit, const T *values, std::index_sequence<I...>) {
	emit(values[I]...);
}
}

template <class Emit>
void read_unsigned_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks(input, count, emit);
		return;
	}
#endif
	std::uint32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

inline void read_unsigned(FastInput &input, std::span<std::uint32_t> out) {
	read_unsigned_blocks(input, out.size(), [&](const std::uint32_t *values, std::size_t count) {
		std::memcpy(out.data(), values, count * sizeof(std::uint32_t));
		out = out.subspan(count);
	});
}

template <std::size_t Fields, class Emit>
void read_unsigned_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::uint32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_unsigned_blocks(input, count * Fields, [&](const std::uint32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}

template <class Emit>
void read_signed_blocks(FastInput &input, std::size_t count, Emit emit) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) {
		fast_io_detail::BulkInput::read_blocks<true>(input, count, [&](const unsigned *values, std::size_t n) {
			emit(reinterpret_cast<const std::int32_t *>(values), n);
		});
		return;
	}
#endif
	std::int32_t values[64];
	while (count) {
		const auto n = std::min<std::size_t>(count, 64);
		for (std::size_t i = 0; i < n; ++i) input.read(values[i]);
		emit(values, n);
		count -= n;
	}
}

template <std::size_t Fields, class Emit>
void read_signed_records(FastInput &input, std::size_t count, Emit emit) {
	static_assert(Fields > 0);
	if (count > std::numeric_limits<std::size_t>::max() / Fields) throw std::length_error("input size");
	std::array<std::int32_t, Fields> pending;
	std::size_t used = 0;
	constexpr auto indices = std::make_index_sequence<Fields>();
	read_signed_blocks(input, count * Fields, [&](const std::int32_t *values, std::size_t n) {
		std::size_t i = 0;
		if (used) {
			while (used < Fields && i < n) pending[used++] = values[i++];
			if (used == Fields) {
				fast_io_detail::emit_record(emit, pending.data(), indices);
				used = 0;
			}
		}
		for (; i < n && n - i >= Fields; i += Fields) fast_io_detail::emit_record(emit, values + i, indices);
		while (i < n) pending[used++] = values[i++];
	});
}


#line 1 "components/fps_write.hpp"



#line 10 "components/fps_write.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_io_fields {

namespace detail {

struct alignas(32) Lanes {
	std::uint32_t lane[8];
};

constexpr Lanes splat(std::uint32_t x) { return {{x, x, x, x, x, x, x, x}}; }
constexpr Lanes all16(std::uint32_t x) { return splat(x * 0x10001); }

constexpr int Z = 0x80;

constexpr Lanes shuffle(const int (&b)[16]) {
	Lanes t{};
	for (int i = 0; i < 32; ++i) t.lane[i / 4] |= std::uint32_t(b[i % 16] & 0xFF) << 8 * (i % 4);
	return t;
}

struct Constants {
	Lanes div10 = splat(429496730);
	Lanes div10e5 = splat(1407374884);
	Lanes pack = splat(1 - 10000 * 65536u);
	Lanes ten = splat(10);
	Lanes tail = splat(' ' << 8 | '0');
	Lanes by100 = all16(5243), hundred = all16(100);
	Lanes by10 = all16(6554);
	Lanes tens_out = all16(2559);
	Lanes ones = splat(~0u), zero = splat(0x30303030), space = splat(0x20202020);

	Lanes chunk0[2] = {shuffle({0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, Z, Z, Z, Z, Z, Z})};
	Lanes chunk1[3] = {shuffle({14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, 4, 5, Z, Z, Z, Z, Z, Z, Z, Z, 8, 9, Z, Z}),
					shuffle({Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7, Z, Z, 8, 9})};
	Lanes chunk2[3] = {shuffle({10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, 12, 13, Z, Z, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3, 4, 5, 6, 7})};
	Lanes chunk3[3] = {shuffle({0, 1, Z, Z, Z, Z, Z, Z, Z, Z, 4, 5, Z, Z, Z, Z}),
					shuffle({Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z, Z, Z, Z, Z}),
					shuffle({Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, Z, 0, 1, 2, 3})};
	Lanes chunk4[2] = {shuffle({4, 5, 6, 7, Z, Z, 8, 9, 10, 11, 12, 13, 14, 15, Z, Z}),
					shuffle({Z, Z, Z, Z, 8, 9, Z, Z, Z, Z, Z, Z, Z, Z, 12, 13})};
};

inline constexpr Constants kConstants{};

[[gnu::always_inline]] inline __m256i load(const Lanes& l) {
	return _mm256_load_si256(reinterpret_cast<const __m256i*>(&l));
}

[[gnu::always_inline]] inline __m256i shuffle(__m256i x, const Lanes& control) {
	return _mm256_shuffle_epi8(x, load(control));
}

struct Divided {
	__m256i halves, tail;
};

[[gnu::always_inline]] inline Divided divide(__m256i v, __m256i odd, const Constants& k) {
	const __m256i w = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10)), 32),
										_mm256_mul_epu32(odd, load(k.div10)), 0xAA);
	const __m256i h = _mm256_blend_epi32(_mm256_srli_epi64(_mm256_mul_epu32(v, load(k.div10e5)), 47),
										_mm256_srli_epi64(_mm256_mul_epu32(odd, load(k.div10e5)), 15), 0xAA);
	const __m256i halves = _mm256_add_epi32(_mm256_slli_epi32(w, 16), _mm256_mullo_epi32(h, load(k.pack)));
	const __m256i tail = _mm256_sub_epi32(_mm256_add_epi32(v, load(k.tail)), _mm256_mullo_epi32(w, load(k.ten)));
	return {halves, tail};
}

[[gnu::always_inline]] inline __m256i two_digits(__m256i z, const Constants& k) {
	const __m256i tens = _mm256_mulhi_epu16(z, load(k.by10));
	return _mm256_sub_epi16(_mm256_slli_epi16(z, 8), _mm256_mullo_epi16(tens, load(k.tens_out)));
}

[[gnu::always_inline]] inline __m256i text(__m256i digits, const Constants& k) {
	const __m256i upto = _mm256_xor_si256(digits, _mm256_add_epi64(digits, load(k.ones)));
	return _mm256_blendv_epi8(_mm256_or_si256(digits, load(k.zero)), load(k.space), upto);
}

[[gnu::always_inline]] inline void digits(const Divided& d, __m256i& lo, __m256i& hi, const Constants& k) {
	const __m256i hundreds = _mm256_srli_epi16(_mm256_mulhi_epu16(d.halves, load(k.by100)), 3);
	const __m256i rest = _mm256_sub_epi16(d.halves, _mm256_mullo_epi16(hundreds, load(k.hundred)));
	lo = text(two_digits(_mm256_unpacklo_epi16(hundreds, rest), k), k);
	hi = text(two_digits(_mm256_unpackhi_epi16(hundreds, rest), k), k);
}

[[gnu::always_inline]] inline void store(char* p, const Divided& lo, const Divided& hi, const Constants& k) {
	__m256i d01, d23, d45, d67;
	const auto either = [&](__m256i a, __m256i b) __attribute__((target("avx2"))) { return _mm256_or_si256(a, b); };
	digits(lo, d01, d23, k);
	digits(hi, d45, d67, k);
	const __m256i c0 = either(shuffle(d01, k.chunk0[0]), shuffle(lo.tail, k.chunk0[1]));
	const __m256i c1 = either(either(shuffle(d01, k.chunk1[0]), shuffle(lo.tail, k.chunk1[1])), shuffle(d23, k.chunk1[2]));
	const __m256i c2 = either(either(shuffle(d23, k.chunk2[0]), shuffle(lo.tail, k.chunk2[1])), shuffle(d45, k.chunk2[2]));
	const __m256i c3 = either(either(shuffle(hi.tail, k.chunk3[0]), shuffle(d45, k.chunk3[1])), shuffle(d67, k.chunk3[2]));
	const __m256i c4 = either(shuffle(d67, k.chunk4[0]), shuffle(hi.tail, k.chunk4[1]));

	const auto whole = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm256_storeu_si256(reinterpret_cast<__m256i*>(p + 64 + 16 * c), chunk); };
	const auto low = [p](int c, __m256i chunk) __attribute__((target("avx2"))) { _mm_store_si128(reinterpret_cast<__m128i*>(p + 16 * c), _mm256_castsi256_si128(chunk)); };
	whole(4, c4), whole(3, c3), whole(2, c2), whole(1, c1), whole(0, c0);
	low(0, c0), low(1, c1), low(2, c2), low(3, c3), low(4, c4);
}

[[gnu::always_inline]] inline void divide16(const std::uint32_t* x, Divided& lo, Divided& hi, const Constants& k) {
	const auto at = [x](int i) __attribute__((target("avx2"))) { return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(x + i)); };
	const auto both = [x](int i) __attribute__((target("avx2"))) { return _mm256_broadcastsi128_si256(_mm_loadu_si128(reinterpret_cast<const __m128i*>(x + i))); };
	const __m256i l = _mm256_blend_epi32(at(0), both(8), 0xF0), h = _mm256_blend_epi32(at(8), both(4), 0x0F);
	lo = divide(l, _mm256_srli_epi64(l, 32), k);
	hi = divide(h, _mm256_srli_epi64(h, 32), k);
}

inline void format(char* p, const std::uint32_t* x, std::size_t count, const Constants& k) {
	Divided dl, dh;
	divide16(x, dl, dh, k);
	for (std::size_t i = 16; i < count; i += 16, p += 160) {
		Divided nl, nh;
		divide16(x + i, nl, nh, k);
		store(p, dl, dh, k);
		dl = nl, dh = nh;
	}
	store(p, dl, dh, k);
}

}

inline constexpr std::size_t kBlock = 25600;
inline constexpr std::size_t kTextBytes = 10 * kBlock;

}

namespace fps_detail {
[[gnu::always_inline]] inline __m256i io_values(__m256i x) {
	const auto p = _mm256_set1_epi32(mint::mod()), ni = _mm256_set1_epi32(998244351);
	auto even = _mm256_and_si256(x, _mm256_set1_epi64x(0xffffffffULL));
	auto odd = _mm256_srli_epi64(x, 32);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	const auto value = _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xaa);
	return _mm256_min_epu32(value, _mm256_sub_epi32(value, p));
}

inline void write_coefficients_native(FastOutput &output, const mint *in, std::size_t n) {
	alignas(32) unsigned values[25600];
	alignas(32) char text[256000];
	const auto &k = fps_io_fields::detail::kConstants;
	for (std::size_t i = 0; i < n; i += 25600) {
		const auto count = std::min<std::size_t>(25600, n - i), full = count / 16 * 16;
		char *end = text;
		static constexpr char zeros[] = "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ";
		const auto empty = [&]() __attribute__((target("avx2"))) {
			std::size_t at = 0;
			const auto p = _mm256_set1_epi32(mint::mod());
			for (; at + 8 <= count; at += 8) {
				__m256i x;
				std::memcpy(&x, in + i + at, sizeof(x));
				x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
				if (!_mm256_testz_si256(x, x)) return false;
			}
			for (; at < count; ++at) if (in[i + at] != mint(0)) return false;
			return true;
		};
		if (empty()) {
			for (std::size_t at = 0; at < count; at += 16) std::memcpy(text + 2 * at, zeros, 32);
			end += 2 * count;
		} else {
			std::size_t j = 0;
			for (; j + 8 <= count; j += 8) {
				alignas(32) unsigned words[8];
				std::memcpy(words, in + i + j, sizeof(words));
				_mm256_storeu_si256(reinterpret_cast<__m256i *>(values + j), io_values(_mm256_load_si256(reinterpret_cast<const __m256i *>(words))));
			}
			for (; j < count; ++j) values[j] = in[i + j].val();
			auto zero = [&](std::size_t at) __attribute__((target("avx2"))) {
				auto x = _mm256_or_si256(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + at)), _mm256_loadu_si256(reinterpret_cast<const __m256i *>(values + at + 8)));
				return _mm256_testz_si256(x, x);
			};
			std::size_t at = 0;
			while (at < full) {
				if (zero(at)) { std::memcpy(end, zeros, 32); end += 32; at += 16; }
				else {
					const auto first = at;
					do at += 16; while (at < full && !zero(at));
					fps_io_fields::detail::format(end, values + first, at - first, k);
					end += 10 * (at - first);
				}
			}
			if (full < count) {
				alignas(32) unsigned rest[16] = {};
				alignas(16) char last[160];
				std::copy_n(values + full, count - full, rest);
				bool empty = true;
				for (auto x : rest) empty &= x == 0;
				if (empty) { std::memcpy(end, zeros, 2 * (count - full)); end += 2 * (count - full); }
				else {
					fps_io_fields::detail::format(last, rest, 16, k);
					std::memcpy(end, last, 10 * (count - full));
					end += 10 * (count - full);
				}
			}
		}
		if (i + count == n) end[-1] = '\n';
		fast_io_detail::BulkOutput::write(output, std::string_view(text, end - text));
	}
	if (!n) output.write('\n');
}
}
#pragma GCC pop_options
#endif

namespace fps_detail {
inline void write_coefficients(FastOutput &output, const mint *in, std::size_t n) {
#if CP_GCC_LINUX64
	if (__builtin_cpu_supports("avx2")) return write_coefficients_native(output, in, n);
#endif
	for (std::size_t i = 0; i < n; ++i) { output.write(in[i].val()); output.write(' '); }
	output.write('\n');
}
}

#line 8 "components/fps_io.hpp"

#if CP_GCC_LINUX64
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
inline void read_coefficients(FastInput &input, std::vector<mint> &out, std::size_t n) {
	if (!n) return;
	if (n > out.max_size() - out.size()) throw std::length_error("fps input");
	prepare_fps_output(out, out.size() + n);
	fast_io_detail::BulkInput::read_blocks(input, n, [&](const unsigned *values, std::size_t count) {
		append_fps_words<false>(out, values, count);
	});
}
}
#pragma GCC pop_options
#endif

#line 7 "test/fixtures/fps_bulk_io.hpp"

inline fps read_fps(FastInput &input, int n) {
	fps result;
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (fps_detail::NTT::vectorized()) {
		fps_detail::read_coefficients(input, result, n);
		return result;
	}
#endif
	result.resize(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	return result;
}

inline void write_fps(FastOutput &output, const fps &f) {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (fps_detail::NTT::vectorized()) {
		fps_detail::write_coefficients(output, f.data(), f.size());
		return;
	}
#endif
	for (auto x : f) { output.write(x.val()); output.write(' '); }
	output.write('\n');
}


#line 4 "test/fps_taylor_shift.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0; unsigned c = 0; input.read(n); input.read(c);
	write_fps(output, taylor_shift(read_fps(input, n), mint::raw(c)));
}
